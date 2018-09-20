#include <unistd.h>
#include <stdlib.h>
#include "ace/Event_Handler.h"
#include "ace/Reactor.h"
#include "zbasedef.h"
#include "zdispatch.h"
#include "zlogger.h"
#include "zmqttconnect.h"
#include "zmqttpublish.h"
#include "zmsgtype.h"
#include "znlmessage.h"
#include "zservicemessage.h"
#include "session.h"
#include "session_manger.h"
#include "service_process.h"
#include "service_thread.h"

ServiceThread::ServiceThread ()
{
}

ServiceThread::~ServiceThread ()
{
}

int ServiceThread::init (int id)
{
    support(TYPE_CONNECT, std::bind(&ServiceThread::on_connect, this, std::placeholders::_1));
    support(TYPE_MQTT, std::bind(&ServiceThread::on_mqtt, this, std::placeholders::_1));
    support(TYPE_SERVICE, std::bind(&ServiceThread::on_service, this, std::placeholders::_1));
    support(TYPE_CONTROL, std::bind(&ServiceThread::on_control, this, std::placeholders::_1));
    
    set_index(id);

    return TQThread::init();
}

int ServiceThread::on_connect(NLMessage*& msg)
{
    ConnectMsg* connect = (ConnectMsg*) msg;
    LOGINFO ("thread get a tcp connection.peer fd is %d.", connect->m_PeerFd);
    Session* session = SessionMgrIns::instance()->create_session(connect->m_PeerFd,
                                                                this);
    if (NULL == session)
    {
        LOGWARN("create session failed! peer fd is %d.", connect->m_PeerFd);
        close(connect->m_PeerFd);
        return -1;
    }

    session->set_status (SESSION_STATUS_INIT);
    if (0 != session->register_input ())
    {
        LOGWARN("register fd[%d] to FDDriver failed!", connect->m_PeerFd);
        session->close();
        return -1;
    }

    return 0;
}

int ServiceThread::on_mqtt(NLMessage*& msg)
{
    MQTTPublish* publish_msg = (MQTTPublish*) msg;

    Session* receiver = SessionMgrIns::instance()->get_session(publish_msg->m_SessionFD);
    if ((NULL == receiver) || !(receiver->valid())
     || (receiver->is_heart_beat_timeout()))
    {
        if ((NULL != receiver) && (receiver->valid())
         && (receiver->is_heart_beat_timeout()))
        {
            LOGWARN("on_mqtt get session failed is hearbeat timeout! user[%d:%lld].", 
                    publish_msg->m_SessionFD, 
                    publish_msg->m_ReceiverID);
            receiver->handle_close();
        }
        
        LOGWARN("on_mqtt user[%d:%lld] is offline..", 
                publish_msg->m_SessionFD, 
                publish_msg->m_ReceiverID);

        OfflineMsg* offline = NULL;
        ZNEW(offline, OfflineMsg);
        offline->m_PublishMsg = publish_msg;
        msg = NULL;
        if (0 != Dispatch::instance()->post_message_by_user(offline, T_DB, 
                                                            publish_msg->m_ReceiverID))
        {
            LOGWARN("Service Process Publish Message send offline message failed.");
            ZDELETE(offline);
        }
        else
        {
            LOGINFO("Service Process Publish Message send offline message Success.");
        }

        return -1;
    }
    else
    {
        receiver->put_down_message (msg);
        msg = NULL; 
        receiver->try_sending();
        return 0;
    }
}

int ServiceThread::on_service(NLMessage*& msg)
{
    int result = 0;
    ServiceMsg *service = (ServiceMsg*)msg;
    switch (service->m_ServiceType)
    {
        case ST_AUTH_RES:
        {
            result = on_auth_rsp(msg);
            break;
        }
        case ST_ONLINE_RES:
        {
            result = on_online_rsp(msg);
            break;
        }
        case ST_KICK_OFF:
        {
            result = on_kick_off_user(msg);
            break;
        }
        case ST_GROUP_QUERY:
        {
            result = on_group_rsp(msg);
            break;
        }
    }
    return result;
}

int ServiceThread::on_control(NLMessage*& msg)
{
    return 0;
}

int ServiceThread::on_online_rsp(NLMessage*& msg)
{
    OnlineRsp* online_rsp = (OnlineRsp*) msg;

    LOGWARN("thread on_online_rsp, peer fd is %d.", online_rsp->m_DeviceFD);
    Session* session = SessionMgrIns::instance()->get_session(online_rsp->m_DeviceFD);
    if (NULL == session)
    {
        LOGWARN("on_online_rsp get session failed! peer fd is %d.", online_rsp->m_DeviceFD);
        return -1;
    }

    if (!session->valid ())
    {
        LOGWARN("on_online_rsp session invalid! peer fd is %d.", online_rsp->m_DeviceFD);

        std::list<NLMessage*>::iterator iter = online_rsp->m_Messages.begin();
        for (; iter != online_rsp->m_Messages.end(); ++iter)
        {
            NLMessage* nl_msg = *iter;
            MQTTMessage* mqtt_msg = (MQTTMessage*)nl_msg;
            if (PUBLISH == mqtt_msg->m_FixedHdr.m_MessageType)
            {
                OfflineMsg* offline = NULL;
                ZNEW(offline, OfflineMsg);
                offline->m_PublishMsg = (NLMessage*)mqtt_msg;
                if (0 != Dispatch::instance()->post_message(offline, T_DB))
                {
                    LOGWARN("handle_close m_DownMessage Publish Message send offline message failed.");
                    ZDELETE(offline);
                }
                else
                {
                    LOGINFO("handle_close m_DownMessage Publish Message send offline message Success.");
                }
            }
            else
            {
                ZDELETE(nl_msg);
            }
        }
        online_rsp->m_Messages.clear();

        if (! event_unregister(online_rsp->m_DeviceFD, EPOLLIN|EPOLLOUT))
        {
            LOGWARN("unregister FD[%d] event[EPOLLIN|EPOLLOUT] to FDDriver failed!",
                    online_rsp->m_DeviceFD);
        }

        return -1;
    }

    session->put_down_message(online_rsp->m_Messages);
    online_rsp->m_Messages.clear ();
    session->try_sending();

    return 0;
}

int ServiceThread::on_auth_rsp(NLMessage*& msg)
{
    AuthMsgRsp* auth_rsp = (AuthMsgRsp*) msg;

    LOGINFO ("thread on_auth_rsp, peer fd is %d.", auth_rsp->m_DeviceFD);
    Session* session = SessionMgrIns::instance()->get_session(auth_rsp->m_DeviceFD);
    if (NULL == session)
    {
        LOGWARN("Service Thread on_online_rsp get session failed! peer fd is %d.", 
                auth_rsp->m_DeviceFD);
        return -1;
    }

    if (!session->valid())
    {
        LOGWARN("Service Thread on_auth_rsp session invalid! peer fd is %d.", 
                auth_rsp->m_DeviceFD);
        if (! event_unregister(auth_rsp->m_DeviceFD, EPOLLIN|EPOLLOUT))
        {
            LOGWARN("Service Thread unregister FD[%d] event[EPOLLIN|EPOLLOUT] to FDDriver failed!", 
                    auth_rsp->m_DeviceFD);
        }
        return -1;
    }

    session->cancle_timer ();

    MQTTConnAck* connack_msg = NULL;
    ZNEW(connack_msg, MQTTConnAck);
    connack_msg->m_RemainingLength = 2;
    connack_msg->m_ConnectReturnCode = auth_rsp->m_AuthResult;
    session->put_down_message (connack_msg);
    connack_msg = NULL;

    session->try_sending();

    if (Connection_Accepted == auth_rsp->m_AuthResult)
    {
        session->query_offline_message();
        session->set_status(SESSION_STATUS_WORK);
    }
    else
    {
        LOGWARN("Service Thread User[%d:%lld] Auth failed!", 
                session->get_peer_fd(), 
                session->get_user_id());
        session->handle_close();
        return 0;
    }

    return 0;
}

int ServiceThread::on_group_rsp (NLMessage*& msg)
{
    GroupQuery* group_rsp = (GroupQuery*) msg;

    LOGINFO ("Service Thread on group response, peer fd is %d.",
             group_rsp->m_DeviceFD);
    Session* session = SessionMgrIns::instance()->get_session(group_rsp->m_DeviceFD);
    if (NULL == session)
    {
        LOGWARN("Service Thread on_online_rsp get session failed! peer fd is %d.", 
                group_rsp->m_DeviceFD);
        return -1;
    }

    MQTTPublish* publish_msg = (MQTTPublish*)(group_rsp->m_PublishMsg);
    group_rsp->m_PublishMsg = NULL;
    std::list<UserIDType>::iterator iter;
    for (iter = group_rsp->m_UserIDs.begin(); iter != group_rsp->m_UserIDs.end(); ++iter)
    {
        UserIDType receiver_id = *iter;
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
            ServiceProcess::process_send_to_receiver(&receiver_node, 
                                     session->get_thread(), 
                                     session->get_user_id());  
            ZDELETE(receiver_node.m_PublishMsg);
        }
    }

    ZDELETE(publish_msg);
    return 0;
}

int ServiceThread::on_kick_off_user (NLMessage*& msg)
{
    KickOff* kick = (KickOff*)msg;

    LOGINFO ("Service Thread on kick off user, peer fd is %d.", kick->m_UserFD);
    if (kick->m_UserFD < 0)
    {
        LOGWARN("Service Thread on_kick_off_user user fd is -1.");
        return -1;
    }

    Session* session = SessionMgrIns::instance()->get_session(kick->m_UserFD);
    if (NULL == session)
    {
        LOGWARN("Service Thread on_kick_off_user get session failed! peer fd is %d.", kick->m_UserFD);
        return -1;
    }

    if (session->is_kick_off())
    {
        session->close();
    }
    else
    {
        LOGWARN("Service Thread on_kick_off_user session kick off flag not set, need do nothing! peer fd is %d.", kick->m_UserFD);
    }
    return 0;
}

int ServiceThread::handle_input (int fd)
{
    LOGINFO ("thread handle input, peer fd is %d.", fd);
    Session* session = SessionMgrIns::instance()->get_session(fd);
    if (NULL == session)
    {
        LOGWARN("handle_input get session failed! peer fd is %d.", fd);
        return -1;
    }

    if (!session->valid())
    {
        LOGWARN("handle_input session invalid! peer fd is %d.", fd);
        if (!event_unregister(fd, EPOLLIN|EPOLLOUT))
        {
            LOGWARN("unregister FD[%d] event[EPOLLIN|EPOLLOUT] to FDDriver failed!", fd);
        }
        return -1;
    }

    if (0 != session->handle_input())
    {
        LOGWARN("session[%p:%d:%lld] handle input failed!",
        session, session->get_peer_fd(), session->get_user_id());
        return -1;
    }

    return 0;
}

int ServiceThread::handle_output(int fd)
{
    LOGINFO ("thread handle output, peer fd is %d.", fd);
    Session* session = SessionMgrIns::instance()->get_session(fd);
    if (NULL == session)
    {
        LOGWARN("handle_output get session failed! peer fd is %d.", fd);
        return -1;
    }

    if (!session->valid())
    {
        LOGWARN("handle_output session invalid! peer fd is %d.", fd);
        if (!event_unregister(fd, EPOLLIN|EPOLLOUT))
        {
            LOGWARN("unregister FD[%d] event[EPOLLIN|EPOLLOUT] to FDDriver failed!", fd);
        }
        return -1;
    }

    if (0 != session->handle_output())
    {
        LOGWARN("session[%p] handle output failed!", session);
        return -1;
    }

    return 0;
}

