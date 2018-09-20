#include "zbasedef.h"
#include "zdispatch.h"
#include "zlogger.h"
#include "zconfiger.h"
#include "znlmessage.h"
#include "zmsgtype.h"
#include "zmqttdef.h"
#include "zmqttpublish.h"
#include "zservicemessage.h"
#include "mongo_user_info_api.h" 
#include "mongo_chat_record_api.h"
#include "mongo_offline_api.h"
#include "mysql_user_info_api.h" 
#include "mysql_chat_record_api.h"
#include "mysql_offline_api.h"
#include "db_thread.h"

DBThread::DBThread ()
{
}

DBThread::~DBThread ()
{
}

int DBThread::init (int id)
{
    support(TYPE_SERVICE, std::bind(&DBThread::on_service, this, std::placeholders::_1));
    
    set_index(id);

    int type = GET_CFG_ITEM(DB, Type);
    DBInfo info;
    info.m_IP = GET_CFG_ITEM(DB, DBIPAddress);
    info.m_Port = GET_CFG_ITEM(DB, DBPort);
    info.m_DBName = GET_CFG_ITEM(DB, DBName);
    info.m_UserName = GET_CFG_ITEM(DB, DBUser);
    info.m_Password = GET_CFG_ITEM(DB, DBPassword);
    info.m_Charset = GET_CFG_ITEM(DB, DBCharset);
    
    switch(type)
    {
        case TYPE_MONGODB:
        {
            ZNEW(m_UserInfoAPI, Mongo_User_Info_API);
            ZNEW(m_RecordAPI, Mongo_Chat_Record_API);
            ZNEW(m_OfflineAPI, Mongo_Offline_API);
            break;
        }
        case TYPE_MYSQL:
        {            
            ZNEW(m_UserInfoAPI, Mysql_User_Info_API);
            ZNEW(m_RecordAPI, Mysql_Chat_Record_API);
            ZNEW(m_OfflineAPI, Mysql_Offline_API);
            break;
        }
        default:
        {
            LOGWARN("UserInfoManager on init not support DB type[%d].", type);
            return -1;
        }        
    }

    if (0 != m_UserInfoAPI->init(info))
    {
        LOGWARN("UserInfoManager m_UserInfoAPI init failed.");
        return -1;
    }

    if (0 != m_RecordAPI->init(info))
    {
        LOGWARN("UserInfoManager m_RecordAPI init failed.");
        return -1;
    }

    if (0 != m_OfflineAPI->init(info))
    {
        LOGWARN("UserInfoManager m_OfflineAPI init failed.");
        return -1;
    }

    return TQThread::init();
}


int DBThread::on_service(NLMessage*& msg)
{
    int result = 0;
    ServiceMsg *service = (ServiceMsg*)msg;
    switch (service->m_ServiceType)
    {
        case ST_AUTH_REQ:
        {
            result = on_auth_req(msg);
            break;
        }
        case ST_GROUP_QUERY:
        {
            result = on_group_req(msg);
            break;
        }
        case ST_CHAR_RECORD:
        {
            result = on_chat_record(msg);
            break;
        }
        case ST_ONLINE_REQ:
        {
            result = on_online_req(msg);
            break;
        }
        case ST_OFFLINE:
        {
            result = on_offline(msg);
            break;
        }
        default:
        {
            break;
        }
    }
    return result;
}


int DBThread::on_auth_req(NLMessage*& msg)
{
    AuthMsgReq* auth_req = (AuthMsgReq*)msg;
    
    AuthMsgRsp* auth_rsp = NULL;
    ZNEW(auth_rsp, AuthMsgRsp);
    auth_rsp->m_UserID = auth_req->m_UserID;
    auth_rsp->m_DeviceFD = auth_req->m_DeviceFD;
    auth_rsp->m_AuthResult = Connection_Accepted;
    std::string password = auth_req->m_Password;
    if (0 != m_UserInfoAPI->query_user_info(auth_req->m_UserID, password))
    {
        LOGWARN("User info Manager query user[%lld] info from DB failed!",
                auth_req->m_UserID);
        auth_rsp->m_AuthResult = Bad_UserName_OR_Password;
    }
    else
    {
        LOGINFO("User info Manager query user[%lld] info from DB success, password is %s, auth password is %s!",
                auth_req->m_UserID, password.c_str(), auth_req->m_Password.c_str());

        if (password != auth_req->m_Password)
        {
            auth_rsp->m_AuthResult = Bad_UserName_OR_Password;
        }
    }

    
    // 回给响应线程
    if (0 != Dispatch::instance()->post_message(auth_rsp, T_MQTT, auth_req->m_ThreadIndex))
    {
        LOGWARN("UserInfoManager on_auth_req dispatch auth_rsp to thread[%d] failed!", 
                auth_req->m_ThreadIndex);
        ZDELETE(auth_rsp);
    }
    
    return 0;
}

int DBThread::on_group_req(NLMessage*& msg)
{
    // 查询用户群组中用户列表
    GroupQuery* group_query = (GroupQuery*)msg;

    int MAX_OOST = GET_CFG_ITEM(SESSION, GroupOutOfServiceTime);
    if (0 != MAX_OOST)
    {
        // 优先从本地查询
        std::map<UserIDType, GroupInfo*>::iterator iter;
        iter = m_GroupInfos.find(group_query->m_GroupID);
        if (iter != m_GroupInfos.end())
        {
            GroupInfo* infos = iter->second;
            time_t current = get_current_time();        
            if ((infos->m_QueryTime + MAX_OOST) > current)
            {
                // expire
                LOGINFO("Group[%lld] expire, need query again!",
                        group_query->m_GroupID);
                ZDELETE(infos);
                m_GroupInfos.erase(iter);
            }
            else
            {
                group_query->m_UserIDs = infos->m_GroupUserIDs;
            }
        }
    }
    else
    {
        LOGINFO("GroupOutOfServiceTime is 0, not store group info to m_GroupInfos!");
    }
    
    if (group_query->m_UserIDs.empty())
    {
        LOGINFO("Query Group[%lld] info from DB!",
                group_query->m_GroupID);
        if (0 != m_UserInfoAPI->query_user_group_info(group_query->m_GroupID, group_query->m_UserIDs))
        {
            LOGWARN("User info Manager user[%lld] query group[%lld] info from DB failed!",
                    group_query->m_UserID, group_query->m_GroupID);
        }
        else
        {
            LOGINFO("User info Manager user[%lld] query group[%lld] info from DB success!",
                    group_query->m_UserID, group_query->m_GroupID);

            if (0 != MAX_OOST)
            {
                typedef std::map<UserIDType, GroupInfo*>::value_type group_value_type;
                GroupInfo* infos = NULL;
                ZNEW(infos, GroupInfo);
                infos->m_GroupUserIDs = group_query->m_UserIDs;
                LOGINFO("insert group[%lld] info[%d users] to m_GroupInfos, time is %u.",
                        group_query->m_UserID, infos->m_GroupUserIDs.size(), infos->m_QueryTime);

                m_GroupInfos.insert(group_value_type(group_query->m_GroupID, infos));
            }
        }
    }
    
    // 回给响应线程
    if (0 != Dispatch::instance()->post_message(group_query, T_MQTT, group_query->m_ThreadIndex))
    {
        LOGWARN("UserInfoManager on_group_req dispatch group_rsp to thread[%d] failed!", 
                 group_query->m_ThreadIndex);
        ZDELETE(group_query);
    }
    
    return 0;
}

int DBThread::on_chat_record(NLMessage*& msg)
{
    int result = 0;
    ChatRecord* record = (ChatRecord*)msg;    
    MQTTPublish *publish = (MQTTPublish*)(record->m_RecordMsg);
    
    if (0 != m_RecordAPI->insert_chat_record(publish->m_SenderID,
                                    publish->m_ReceiverID,
                                    0,
                                    publish->m_RecvMessageID,
                                    publish->m_TimeStamp,
                                    (char*)(publish->m_PublishData), 
                                    publish->m_PublishDataLength,
                                    publish->m_ReceiverType))
    {
        LOGWARN("chat record [Sender is %lld, receiver is %lld, extend is %lld, message id is %u] to DB failed!",
                publish->m_SenderID,
                publish->m_ReceiverID,
                0,
                publish->m_RecvMessageID);
        DEBUGBIN((const char*)(publish->m_PublishData), publish->m_PublishDataLength,
                "Payload is :");
        result = -1;
    }
    else
    {
        LOGINFO("chat record [Sender is %lld, receiver is %lld, extend is %lld, message id is %u] to DB success!",
                publish->m_SenderID,
                publish->m_ReceiverID,
                0,
                publish->m_RecvMessageID);
    }

    ZDELETE(publish);
    record->m_RecordMsg = NULL;
    return 0;
}

int DBThread::on_online_req(NLMessage*& msg)
{
    OnlineReq* online_req = (OnlineReq*)msg;
    std::list<NLMessage*>  messages;
    if (0 != m_OfflineAPI->query_user_offline_message(online_req->m_UserID, 
                                                      messages))
    {
        LOGWARN("Offline Manager query user[%lld] offline message from DB failed!",
                online_req->m_UserID);
    }
    else
    {
        LOGWARN("Offline Manager query user[%lld] offline message from DB success!",
                online_req->m_UserID);
    }
    
    if (messages.empty())
    {
        LOGINFO("Offline Manager User[%lld] no offline message.", online_req->m_UserID);
    }
    else
    {        
        messages.sort(compare_offline);
        OnlineRsp* online_rsp = NULL;
        ZNEW(online_rsp, OnlineRsp);
        online_rsp->m_UserID = online_req->m_UserID;
        online_rsp->m_DeviceFD = online_req->m_DeviceFD;
        online_rsp->m_Messages = messages;
        
        if (0 != Dispatch::instance()->post_message(online_rsp, T_MQTT, online_req->m_ThreadIndex))
        {
            LOGWARN("Offline Manager send online respose send to service thread[%d] failed.", 
                    online_req->m_ThreadIndex);
            ZDELETE(online_rsp);
        }
        else
        {
            LOGWARN("Offline Manager send online respose send to service thread[%d] success.", 
                    online_req->m_ThreadIndex);
            online_rsp = NULL;
        }
    }

    ZDELETE(online_req);
    msg = NULL;
    return 0;
}

int DBThread::on_offline(NLMessage*& msg)
{
    OfflineMsg* offline = (OfflineMsg*)msg;
    MQTTPublish* publish_msg = (MQTTPublish*)(offline->m_PublishMsg);
    DEBUGBIN((const char*)(publish_msg->m_PublishData), 
             publish_msg->m_PublishDataLength, 
             "offline message [Time is %u, Sender is %lld, Receiver is %lld, TopocName is %s, message id is %u]",
             publish_msg->m_TimeStamp,
             publish_msg->m_SenderID, 
             publish_msg->m_ReceiverID,
             publish_msg->m_TopicName.c_str(), 
             publish_msg->m_RecvMessageID);

    
    if (0 != m_OfflineAPI->insert_offline_message(publish_msg))
    {
        LOGWARN("Offline Manager insert user[%lld] offline message to DB failed!",
                publish_msg->m_ReceiverID);
    }
    else
    {
        LOGWARN("Offline Manager insert user[%lld] offline message to DB success!",
                publish_msg->m_ReceiverID);
    }    

    if (0 != Dispatch::instance()->post_message_by_user(offline, T_APNS, publish_msg->m_ReceiverID))
    {
        LOGWARN("Service Process Publish Message send offline message failed.");
        ZDELETE(offline);
    }
    else
    {
        LOGINFO("Service Process Publish Message send offline message Success.");
    }           

    //DELETE_OBJ(Offline_Msg,offline);
    msg = NULL;
    return 0;
}


bool compare_offline (const NLMessage* first, const NLMessage* second)
{
    return ( first->message_time() < second->message_time() );
}

