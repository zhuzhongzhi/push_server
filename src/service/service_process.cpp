
#include <stdlib.h>
#include "zbasedef.h"
#include "zcrc32.h"
#include "znlmessage.h"
#include "zmqttconnect.h"
#include "zdispatch.h"
#include "zmd5.h"
#include "zmqttdef.h"
#include "zmqttbase.h"
#include "zmqttping.h"
#include "zmqttpublish.h"
#include "zmqttsubscribe.h"
#include "zmqttunsubscribe.h"
#include "zconfiger.h"
#include "zlogger.h"
#include "zutil.h"
#include "zservicemessage.h"
#include "session.h"
#include "session_manger.h"
#include "service_thread.h"
#include "device_manager.h"
#include "service_process.h"

PROCESS_FUNC  ServiceProcess::m_ProcessFuctions[MqttTypeSize] = {&ServiceProcess::process_reserved1,
&ServiceProcess::process_connect, &ServiceProcess::process_connack, &ServiceProcess::process_publish,
&ServiceProcess::process_puback, &ServiceProcess::process_pubrec, &ServiceProcess::process_pubrel,
&ServiceProcess::process_pubcomp, &ServiceProcess::process_subscribe, &ServiceProcess::process_suback,
&ServiceProcess::process_unsubscribe, &ServiceProcess::process_unsuback, &ServiceProcess::process_pingreq,
&ServiceProcess::process_pingrsp, &ServiceProcess::process_disconnect, &ServiceProcess::process_reserved2};

ServiceProcess::ServiceProcess()
{
}

ServiceProcess::~ServiceProcess()
{
}

int ServiceProcess::process_recv(MQTTMessage*& mqtt_msg, Session* session)
{
    int result = m_ProcessFuctions[mqtt_msg->m_FixedHdr.m_MessageType](mqtt_msg, session);
    LOGINFO("process %d message result is %d.",
            mqtt_msg->m_FixedHdr.m_MessageType,
            result);

    ZDELETE(mqtt_msg);   
    return result;
}

int ServiceProcess::process_reserved1(MQTTMessage*& mqtt_msg, Session* session)
{
    LOGWARN("ServiceProcess process User[%d:%lld]'s reserved1!",
            session->get_peer_fd(),
            session->get_user_id());
    return 0;
}

int ServiceProcess::process_connect(MQTTMessage*& mqtt_msg, Session* session)
{    
    MQTTConnect* connect_msg = (MQTTConnect*)mqtt_msg;
    UserIDType user_id = atoll(connect_msg->m_ClientIdentifier.c_str());
    LOGWARN("process_connect UserName[%s] Password[%s] DeviceType[%s] DeviceToken[%s] "
            "ClientID[%s] UserID[%lld] Connect!",
            connect_msg->m_UserName.c_str(),
            connect_msg->m_Password.c_str(),
            connect_msg->m_WillTopic.c_str(),
            connect_msg->m_WillMessage.c_str(),
            connect_msg->m_ClientIdentifier.c_str(),
            user_id);
    // set user info and device info to session
    session->set_user_id(user_id);
    session->set_user_name(connect_msg->m_UserName);
    session->set_password(connect_msg->m_Password);
    session->set_device_type(connect_msg->m_WillTopic);
    session->set_device_token(connect_msg->m_WillMessage);
    int type = get_device_type(connect_msg->m_WillTopic);
    session->set_device_int_type(type);

    // store session to session manager
    SessionMgrIns::instance()->map_userid_and_session(session);

    // find all session by user id
    std::list<Session*> sessions_find;
    std::list<Session*>::iterator iter_find;
    SessionMgrIns::instance()->get_session(user_id, sessions_find);
    for (iter_find = sessions_find.begin(); iter_find != sessions_find.end(); ++iter_find)
    {
        Session* find_session = *iter_find;
        if ((NULL != find_session) 
        && (find_session != session) 
        && (!find_session->is_kick_off()))
        {   
            ServiceThread* session_thread = find_session->get_thread();
            if ((NULL != session_thread)  
            && (find_session->get_device_int_type() == type))
            {
                LOGWARN("process_connect need close session[%d:%lld] UserName[%s] Password[%s] DeviceType[%s] DeviceToken[%s].",
                        find_session->get_peer_fd(), 
                        find_session->get_user_id(),
                        find_session->get_user_name().c_str(),
                        find_session->get_password().c_str(),
                        find_session->get_device_type().c_str(),
                        find_session->get_device_token().c_str());

                // same device login twice
                if (find_session->get_device_token() == connect_msg->m_WillMessage)
                {
                    LOGWARN("process_connect user[%d:%lld] already connected!",
                        session->get_peer_fd(), session->get_user_id());

                    find_session->set_same_device();

                    auth_failed(session, Identifier_Rejected);
                    return 0;
                }
                else
                {
                    // same type device login, will kick off the device that login before
                    LOGWARN("process_connect user[%lld] session[%d] will kick off session[%d]!",
                        session->get_user_id(),
                        session->get_peer_fd(),
                        find_session->get_peer_fd());

                    // set falg
                    find_session->set_is_kick_off();

                    if (session_thread == session->get_thread())
                    {
                        // in same thread , close straight
                        find_session->close();
                    }
                    else
                    {
                        // not in same thread, will send a message to that thread close it
                        KickOff* kick_off = NULL;
                        ZNEW(kick_off, KickOff);
                        kick_off->m_UserFD = find_session->get_peer_fd();
                        if (0 == session_thread->post_message(kick_off))
                        {
                            LOGINFO("process_connect post message to thread[%p] success.", session_thread);
                        }
                    }
                }
            }
        }
    }

    if (type < 3)
    {
    	LOGWARN("Aple Device , Need Add Device Token.[%d:%lld:%s].",
                type, user_id, connect_msg->m_WillMessage.c_str());
        DeviceMgrIns::instance()->add_device(user_id, connect_msg->m_WillMessage, type);
    }
    
    //Auth Connect Request
    int auth_user = GET_CFG_ITEM(Service, AuthUser);
    if (auth_user > 1)
    {
        AuthMsgReq* auth_req = NULL;
        ZNEW(auth_req, AuthMsgReq);
        auth_req->m_ThreadIndex = session->get_thread()->get_index();
        auth_req->m_DeviceFD = session->get_peer_fd();
        auth_req->m_UserID = session->get_user_id();
        auth_req->m_UserName = connect_msg->m_UserName;
        auth_req->m_Password = connect_msg->m_Password;
            
        if (0 != Dispatch::instance()->post_message_by_user(auth_req, T_DB, user_id))
        {
            LOGWARN("Service Process Connect Message send auth request message failed.");
            ZDELETE(auth_req);

            auth_failed(session, Server_Unavailable);
        }
        else
        {
            LOGINFO("Service Process wait for Auth response.");
            session->set_status(SESSION_STATUS_WAIT_AUTH_RSP);
            int future = GET_CFG_ITEM(SESSION, AuthTimeOut);
            session->set_timer(future);
        }
    }
    else if (1 == auth_user)
    {
        LOGINFO("Service Process system need auth user token.");
        STLString::size_type pos = connect_msg->m_Password.find(',');
        if (pos != STLString::npos)
        {            
            STLString ts = connect_msg->m_Password.substr(pos + 1);
            time_t valid_time = atoi(ts.c_str());
            time_t current = session->get_thread()->get_current_time();
            int term = GET_CFG_ITEM(AUTH, TermOfValidty);
            if ((valid_time + term) > current)
            {
                STLString key = GET_CFG_ITEM(AUTH, PublishKey);
                STLString value = connect_msg->m_UserName + key + ts;
                // token = MD5(UserName + Key + ts)
                char md5_buffer[256] = {0};
                if (0 == MD5_hmac((char*)(value.c_str()), value.length(), md5_buffer))
                {
                    if (0 == strncmp(md5_buffer, 
                                      connect_msg->m_Password.c_str(), 
                                      pos))
                    {
                        // ok 
                        LOGSERVICE("Service Process system auth user token success.");
                        
                        auth_ok(session);
                    }
                    else
                    {
                        LOGSERVICE("Service Process system auth user token , "
                        "MD5 check failed, [%s, %s].", 
                        md5_buffer, connect_msg->m_Password.c_str());

                        auth_failed(session, Bad_UserName_OR_Password);
                    }
                }
                else
                {
                    LOGSERVICE("Service Process system auth user token , MD5 failed.");
                    auth_failed(session, Server_Unavailable);
                }
            }
            else
            {
                // expired
                LOGSERVICE("Service Process system auth user token expired.[%d:%d]", 
                                    valid_time,  current);
                auth_failed(session, Not_Authorized);
            }
        }
        else
        {
            // bad password
            LOGSERVICE("Service Process system auth user token , "
                        "bad password[%s].", 
                        connect_msg->m_Password.c_str());
            auth_failed(session, Identifier_Rejected);
        }
    }
    else if (0 == auth_user)
    {
        //not need auth
        LOGINFO("Service Process system not need auth user, user login success.");
        auth_ok(session);
    }
    return 0;
}

void ServiceProcess::auth_ok(Session* session)
{
    // query user's offline message
    session->query_offline_message();

    //connect ok
    MQTTConnAck* connack_msg = NULL;
    ZNEW(connack_msg, MQTTConnAck);
    connack_msg->m_RemainingLength = 2;
    connack_msg->m_ConnectReturnCode = Connection_Accepted;
    session->put_down_message(connack_msg);
    connack_msg = NULL;

    session->try_sending();
    session->set_status(SESSION_STATUS_WORK);
}

void ServiceProcess::auth_failed(Session* session, const unsigned char reason)
{
    MQTTConnAck* connack_msg = NULL;
    ZNEW(connack_msg, MQTTConnAck);
    connack_msg->m_RemainingLength = 2;
    connack_msg->m_ConnectReturnCode = reason;
    session->put_down_message(connack_msg);
    connack_msg = NULL;

    session->try_sending();
    session->set_status(SESSION_STATUS_INIT);
    session->handle_close();
}

int ServiceProcess::get_device_type(const STLString& device)
{
    int type = 99;
    if (device == STLString("iphone"))
    {
        type = 0;
    }
    else if (device == STLString("ipad"))
    {
        type = 1;
    }
    else if (device == STLString("imac"))
    {
        type = 2;
    }
    else if (device == STLString("android_phone"))
    {
        type = 3;
    }
    else if (device == STLString("android_pad"))
    {
        type = 4;
    }
    else if (device == STLString("windows_phone"))
    {
        type = 5;
    }
    else if (device == STLString("windows_pc"))
    {
        type = 6;
    }
    else
    {
        // other  etc. android or windows
        type = 99;
    }

    return type;
}

int ServiceProcess::process_connack(MQTTMessage*& mqtt_msg, Session* session)
{
    LOGINFO("process_connack server just discard connect ack message!");
    return 0;
}

int ServiceProcess::process_publish(MQTTMessage*& mqtt_msg, Session* session)
{        
    MQTTPublish* publish_msg = (MQTTPublish*)mqtt_msg;

    //send response to client
    if (1 == publish_msg->m_FixedHdr.m_QosLevel)
    {
        // send puback
        MQTTPubAck* puback_msg = NULL;
        ZNEW(puback_msg, MQTTPubAck);
        puback_msg->m_RemainingLength = 2;
        puback_msg->m_FixedHdr.m_QosLevel= publish_msg->m_FixedHdr.m_QosLevel;
        puback_msg->m_MessageID = publish_msg->m_RecvMessageID;
        session->put_down_message(puback_msg);
        puback_msg = NULL;

        LOGWARN("User[%d:%lld] Service Porcess send puback.", 
                session->get_peer_fd(), session->get_user_id());
        session->try_sending();
    }
    else if (2 == publish_msg->m_FixedHdr.m_QosLevel)
    {
        // send pubrec
        MQTTPubRec* purec_msg = NULL;
        ZNEW(purec_msg, MQTTPubRec);
        purec_msg->m_RemainingLength = 2;
        purec_msg->m_FixedHdr.m_QosLevel= publish_msg->m_FixedHdr.m_QosLevel;
        purec_msg->m_MessageID = publish_msg->m_RecvMessageID;
        session->put_down_message(purec_msg);
        purec_msg = NULL;

        LOGWARN("User[%d:%lld] Service Porcess send pubrec.", 
                session->get_peer_fd(), session->get_user_id());
        session->end_sending();
    }
    else
    {
        LOGWARN("User[%d:%lld] publish message qos_level is 0, no need send puback.", 
                session->get_peer_fd(), session->get_user_id());
    }

    // set send user name, used when ios apns
    publish_msg->m_SenderName = session->get_user_name();
    publish_msg->replace_time_stamp(); // use server time, not client time
    
    // do one chat record
    ChatRecord* record = NULL;
    ZNEW(record, ChatRecord);
    record->m_RecordMsg = publish_msg->clone();
    if (0 != Dispatch::instance()->post_message(record, T_DB))
    {
        LOGWARN("Service Process Publish Message send chat record message failed.");
        ZDELETE(record);
    }
    else
    {
        LOGINFO("Service Process Publish Message send chat record message Success.");
        record = NULL;
    }
            
    // send to one person
    if (RT_PO == publish_msg->m_ReceiverType)
    {
        if (publish_msg->m_ReceiverID == session->get_user_id())
        {
            // send to himself will be discrad
            LOGWARN("Service Discard publish message, because the message is send by himself,"
                    " message id is %u, topic name is %s, receiver is %lld, extend is %lld." ,
                     publish_msg->m_RecvMessageID,
                     publish_msg->m_TopicName.c_str(),
                     publish_msg->m_ReceiverID,
                     0);
        }
        else
        {
            LOGWARN("Service Porcess puhlish message, message id is %u, "
                    "topic name is %s, receiver is %lld, extend is %lld." ,
                     publish_msg->m_RecvMessageID,
                     publish_msg->m_TopicName.c_str(),
                     publish_msg->m_ReceiverID,
                     0);

            //process one receiver
            ReceiverNode receiver_node;
            receiver_node.m_PublishMsg = publish_msg;
            mqtt_msg = NULL; //mqtt_msg store in ReceiverNode
            receiver_node.m_ReceiverUserID = publish_msg->m_ReceiverID;
            receiver_node.m_Next = NULL;
            process_send_to_receiver(&receiver_node, 
                                     session->get_thread(), 
                                     session->get_user_id());
            ZDELETE(receiver_node.m_PublishMsg);                         
        }

        return 0;
    }
    else if (RT_GO == publish_msg->m_ReceiverType) // send to group
    {
        // query group info
        GroupQuery* group_req = NULL;
        ZNEW(group_req, GroupQuery);
        group_req->m_GroupID = publish_msg->m_ReceiverID;
        group_req->m_PublishMsg = publish_msg;
        mqtt_msg = NULL; //mqtt_msg store in Group_req
        group_req->m_ThreadIndex = session->get_thread()->get_index();
        group_req->m_DeviceFD = session->get_peer_fd();
        group_req->m_UserID = session->get_user_id();

        // send req to query thread
        if (0 != Dispatch::instance()->post_message_by_user(group_req,
                                                            T_DB,
                                                            group_req->m_GroupID))
        {
            LOGWARN("Service Process Publish Message send group request failed.");
            ZDELETE(group_req);
        }
        else
        {
            LOGINFO("Service Process Publish Message send group request Success.");
            group_req = NULL;
        }

        // wait for response
        return 0;
    }
    else if (RT_PS == publish_msg->m_ReceiverType)
    {
        // receiver is person
        std::string recevier_list = publish_msg->m_MultReceivers;
        std::list<std::string> out_list;
        std::list<std::string>::iterator iter;
        split_string(recevier_list.c_str(), ',', out_list);
        UserIDType receiver_id = 0;
        for (iter = out_list.begin(); iter != out_list.end(); ++iter)
        {
            std::string& arecviver = *iter;
            receiver_id == atoll(arecviver.c_str());
            if (receiver_id == session->get_user_id())
            {
                LOGWARN("Service Discard puhlish message, because the message is send by himself,"
                        " message id is %u, topic name is %s, receiver is %lld, extend is %lld." ,
                         publish_msg->m_RecvMessageID,
                         publish_msg->m_TopicName.c_str(),
                         receiver_id,
                         0);
            }
            else
            {
                LOGWARN("Service Porcess puhlish message, message id is %u, "
                        "topic name is %s, receiver is %lld, extend is %lld." ,
                         publish_msg->m_RecvMessageID,
                         publish_msg->m_TopicName.c_str(),
                         receiver_id,
                         0);

                //process one receiver
                ReceiverNode receiver_node;
                receiver_node.m_PublishMsg = publish_msg->clone();
                // set the real receiver
                receiver_node.m_PublishMsg->m_ReceiverID = receiver_id;
                receiver_node.m_ReceiverUserID = receiver_id;
                receiver_node.m_Next = NULL;
                process_send_to_receiver(&receiver_node, 
                                         session->get_thread(), 
                                         session->get_user_id());  
                ZDELETE(receiver_node.m_PublishMsg);
            }
        }

        return 0;
    }
}

int ServiceProcess::process_send_to_receiver(ReceiverNode* current_receiver, ServiceThread* thread, UserIDType sender_id)
{    
    int mult_login = GET_CFG_ITEM(Service, MultLogin);
    UserIDType receiver_id = current_receiver->m_ReceiverUserID; 
    Session* receiver = NULL;
    bool is_offline = false;
    LOGINFO("Receiver is %lld, query user's session from session manager.", receiver_id);
    if (0 == mult_login)
    {   
        // not support mult login, so only one device
        SessionMgrIns::instance()->get_session(receiver_id, receiver);
        if ((NULL != receiver) && (receiver->valid()))
        {
            process_send_to_one_receiver(current_receiver->m_PublishMsg, 
                                         receiver, 
                                         thread, 
                                         sender_id); 
        }
        else
        {
            is_offline = true;
        }
    }
    else
    {
        std::list<Session*> sessions;
        SessionMgrIns::instance()->get_session(receiver_id, sessions);
        LOGINFO("User[%lld] find %d sessions.", receiver_id, sessions.size());
        if (!sessions.empty())
        {
            std::list<Session*>::iterator iter;            
            for (iter = sessions.begin(); iter != sessions.end(); ++iter)
            {
                receiver = *iter;
                if ((NULL != receiver) && (receiver->valid()))
                {
                    MQTTPublish *msg = current_receiver->m_PublishMsg->clone();
                    process_send_to_one_receiver(msg, 
                                                 receiver, 
                                                 thread, 
                                                 sender_id);
                    ZDELETE(msg);
                }
            }

            
        }
        else
        {
            is_offline = true;
        }
        
    }

    if (is_offline)
    {
        LOGINFO("Service Process publish receiver[%lld] is offline.", receiver_id);

        OfflineMsg* offline = NULL;
        ZNEW(offline, OfflineMsg);
        offline->m_PublishMsg = current_receiver->m_PublishMsg;
        current_receiver->m_PublishMsg = NULL;
        if (0 != Dispatch::instance()->post_message(offline, T_DB))
        {
            LOGWARN("Service Process Publish Message send offline message failed.");
            ZDELETE(offline);
        }
        else
        {
            LOGINFO("Service Process Publish Message send offline message Success.");
        }           
    }

    return 0;
}

int ServiceProcess::process_send_to_one_receiver(MQTTPublish*& publishMsg, Session* receiver, ServiceThread* thread, UserIDType sender_id)
{            
    LOGINFO("get session[%p] by user id[%lld].", 
            receiver, receiver->get_user_id());  
    ServiceThread* session_thread = receiver->get_thread();
    if (session_thread == thread)
    {
        LOGINFO("Sender[%lld] and receiver[%d:%lld] in same thread.",
                sender_id,
                receiver->get_peer_fd(), 
                receiver->get_user_id());
        receiver->put_down_message(publishMsg);
        publishMsg = NULL;

        receiver->end_sending();
    }
    else if (NULL != session_thread)
    {
        publishMsg->m_SessionFD = receiver->get_peer_fd();
        if (0 == session_thread->post_message(publishMsg))
        {
            publishMsg = NULL;
            LOGINFO("process_publish post message to thread[%p] success.", 
                    receiver->get_thread());
        }
        else
        {
            ZDELETE(publishMsg);
            LOGWARN("Message[%p] post to thread[%p] failed!", 
                    publishMsg, 
                    receiver->get_thread());
        }
    }

    return 0;
}

int ServiceProcess::process_puback(MQTTMessage*& mqtt_msg, Session* session)
{    
    // send by a subscriber , server can delete the id's message
    MQTTPubAck* puback_msg = (MQTTPubAck*)mqtt_msg;
    LOGWARN("process_puback recv user[%d:%lld]'s publish ack message[%u]", 
            session->get_peer_fd(), 
            session->get_user_id(),
            puback_msg->m_MessageID);
    
    session->process_ack(puback_msg->m_MessageID);
    
    return 0;
}

int ServiceProcess::process_pubrec(MQTTMessage*& mqtt_msg, Session* session)
{
    // send a subscriber , in response to a PUBLISH message from the server.
    // need send A PUBREL message as  the response

    MQTTPubRec* pubrec_msg = (MQTTPubRec*)mqtt_msg;
    LOGWARN("Service Porcess process user[%d:%lld]'s pubrec, message id is %u." ,
            session->get_peer_fd(), session->get_user_id(),
            pubrec_msg->m_MessageID);

    MQTTPubRel* pubrel_msg = NULL;
    ZNEW(pubrel_msg, MQTTPubRel);
    pubrel_msg->m_RemainingLength = 2;
    pubrel_msg->m_MessageID = pubrec_msg->m_MessageID;
    session->put_down_message(pubrel_msg);
    pubrel_msg = NULL;
    
    session->try_sending();

    return 0;
}

int ServiceProcess::process_pubrel(MQTTMessage*& mqtt_msg, Session* session)
{    
    MQTTPubRel* pubrel_msg = (MQTTPubRel*)mqtt_msg;
    LOGWARN("Service Porcess process user[%d:%lld]'s pubrel, message id is %u." ,
            session->get_peer_fd(), 
            session->get_user_id(),
            pubrel_msg->m_MessageID);

    session->process_ack(pubrel_msg->m_MessageID);
    
    MQTTPubComp* pubcomp_msg = NULL;
    ZNEW(pubcomp_msg, MQTTPubComp);
    pubcomp_msg->m_RemainingLength = 2;
    pubcomp_msg->m_MessageID = pubrel_msg->m_MessageID;
    session->put_down_message(pubcomp_msg);
    pubcomp_msg = NULL;

    session->try_sending();
    return 0;
}

int ServiceProcess::process_pubcomp(MQTTMessage*& mqtt_msg, Session* session)
{    
    MQTTPubComp* pubcomp_msg = (MQTTPubComp*)mqtt_msg;
    LOGWARN("Service Porcess process user[%d:%lld]'s pubcomp, message id is %u." ,
            session->get_peer_fd(), 
            session->get_user_id(),
            pubcomp_msg->m_MessageID);

    return 0;
}

int ServiceProcess::process_subscribe(MQTTMessage*& mqtt_msg, Session* session)
{
    MQTTSubscribe* subscribe_msg = (MQTTSubscribe*)mqtt_msg;
    LOGINFO("process_subscribe UserID[%d:%lld] MessageID[%d] TopicQos Num[%d]!",
            session->get_peer_fd(),
            session->get_user_id(),
            subscribe_msg->m_MessageID,
            subscribe_msg->m_SubTopics.size());

    MQTTSubAck* suback_msg = NULL;
    ZNEW(suback_msg, MQTTSubAck);
    suback_msg->m_MessageID = subscribe_msg->m_MessageID;
    std::list<TSubTopic>::iterator iter = subscribe_msg->m_SubTopics.begin();
    for (; iter != subscribe_msg->m_SubTopics.end(); ++iter)
    {
        TSubTopic& pair = *iter;
        unsigned char qos = pair.m_Qos;
        suback_msg->m_GrantedQos.push_back(qos);
    }
    suback_msg->m_RemainingLength = sizeof(suback_msg->m_MessageID) + suback_msg->m_GrantedQos.size();
    session->put_down_message(suback_msg);
    suback_msg = NULL;
    
    session->try_sending();

    return 0;
}

int ServiceProcess::process_suback(MQTTMessage*& mqtt_msg, Session* session)
{
    MQTTSubAck* suback_msg = (MQTTSubAck*)mqtt_msg;
    LOGINFO("ServiceProcess process UserID[%d:%lld]'s suback MessageID[%d]!",
            session->get_peer_fd(),
            session->get_user_id(),
            suback_msg->m_MessageID);

    return 0;
}

int ServiceProcess::process_unsubscribe(MQTTMessage*& mqtt_msg, Session* session)
{
    MQTTUNSubscribe* unsubscribe_msg = (MQTTUNSubscribe*)mqtt_msg;
    LOGINFO("ServiceProcess process UserID[%d:%lld]'s unsubscribe MessageID[%d] Topic Num[%d]!",
            session->get_peer_fd(),
            session->get_user_id(),
            unsubscribe_msg->m_MessageID,
            unsubscribe_msg->m_Topics.size());

    return 0;
}

int ServiceProcess::process_unsuback(MQTTMessage*& mqtt_msg, Session* session)
{
    MQTTUNSubAck* unsuback_msg = (MQTTUNSubAck*)mqtt_msg;
    LOGINFO("ServiceProcess process UserID[%d:%lld]'s unsuback MessageID[%d]!",
            session->get_peer_fd(),
            session->get_user_id(),
            unsuback_msg->m_MessageID);
    
    return 0;
}

int ServiceProcess::process_pingreq(MQTTMessage*& mqtt_msg, Session* session)
{     
    LOGINFO("ServiceProcess process UserID[%d:%lld]'s ping req!",
            session->get_peer_fd(),
            session->get_user_id());

    MQTTPingRsp* pingrsp_msg = NULL;
    ZNEW(pingrsp_msg, MQTTPingRsp);
    session->put_down_message(pingrsp_msg);
    pingrsp_msg = NULL;

    session->try_sending();

    session->query_offline_message();
    return 0;
}

int ServiceProcess::process_pingrsp(MQTTMessage*& mqtt_msg, Session* session)
{
    LOGINFO("ServiceProcess process UserID[%d:%lld]'s ping rsp!",
            session->get_peer_fd(),
            session->get_user_id());
    
    return 0;
}

int ServiceProcess::process_disconnect(MQTTMessage*& mqtt_msg, Session* session)
{
    MQTTDisConnect* disconnect_msg = (MQTTDisConnect*)mqtt_msg;
    LOGWARN("ServiceProcess process UserID[%d:%lld]'s disconnect MessageID[%d]!",
            session->get_peer_fd(),
            session->get_user_id());

    session->close();
    return 0;
}

int ServiceProcess::process_reserved2(MQTTMessage*& mqtt_msg, Session* session)
{
    LOGWARN("ServiceProcess process User[%d:%lld]'s reserved2!",
            session->get_peer_fd(),
            session->get_user_id());
    return 0;
}

