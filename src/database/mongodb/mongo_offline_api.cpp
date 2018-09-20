
#include "zbase64.h"
#include "zmqttdef.h"
#include "zmqttpublish.h"
#include "zlogger.h"
#include "mongo_offline_api.h"

Mongo_Offline_API::Mongo_Offline_API()
{
    m_TableName = ".offline_message";
    m_Connection = NULL;
}

Mongo_Offline_API::~Mongo_Offline_API()
{    
    if (NULL != m_Connection)
    {
        ZDELETE(m_Connection);
    }
}

int Mongo_Offline_API::init(const DBInfo& info)
{
    m_Info = info;
    m_TableName = info.m_DBName + m_TableName;
    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mongo_Offline_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mongo_Offline_API::connect_db(const DBInfo& info)
{
    if (NULL != m_Connection)
    {
        ZDELETE(m_Connection);
    }

    ZNEW(m_Connection, mongo::DBClientConnection);
    
    char szDest[128] = {0};
    sprintf(szDest, "%s:%d", info.m_IP.c_str(), info.m_Port);
    std::string strErrorMsg;

    try
    {
        if (!m_Connection->connect(szDest, strErrorMsg))
        {
            LOGWARN("Mongo_Offline_API connect db error: %s\n", strErrorMsg.c_str());
            return -1;
        }

        if (!m_Connection->auth(info.m_DBName, info.m_UserName, info.m_Password, strErrorMsg))
        {
            LOGWARN("Mongo_Offline_API authorize check error: %s\n", strErrorMsg.c_str());
            return -1;
        }
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Offline_API init error: %s\n", strerrmsg.c_str());
        return -1;
    }

    return 0;
}

int Mongo_Offline_API::insert_offline_message(NLMessage* msg)
{
    try
    {
        if (m_Connection->isFailed())
        {
            if (0 != connect_db(m_Info))
            {
                LOGWARN("insert_offline_message connect_db failed!");
                return -1;
            }
        }
        MQTTPublish* publish = (MQTTPublish*)msg;
        mongo::BSONObjBuilder builder;
        builder.append("Sender", publish->m_SenderID);
        builder.append("Receiver", publish->m_ReceiverID);
        builder.append("MultReceiver", publish->m_MultReceivers);
        builder.append("Topic", publish->m_TopicName);
        builder.append("SenderName", publish->m_SenderName);
        builder.append("GroupName", publish->m_SenderName);
        builder.append("MessageType", publish->m_MessageType);
        builder.append("ReceiverType", publish->m_ReceiverType);
        builder.append("MessageID", publish->m_SendMessageID);
        int offset = publish->m_PublishData - publish->m_RemainingData;
        builder.append("offset", offset);
        builder.append("TimeStamp", (long long)(publish->m_TimeStamp));
        builder.appendBinData("Payload", publish->m_RemainingLength, mongo::BinDataGeneral, (const void *)(publish->m_RemainingData));
        m_Connection->insert(m_TableName, builder.obj());
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Offline_API insert_offline_message error: %s\n", strerrmsg.c_str());
        return -1;
    }

    return 0;
}

int Mongo_Offline_API::query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages)
{
    char szDest[128] = {0}; 
    sprintf(szDest, "this.Receiver == %lld", user_id);
    mongo::Query query;
    query.where(szDest);
    try
    {        
        if (m_Connection->isFailed())
        {
            if (0 != connect_db(m_Info))
            {
                LOGWARN("query_user_offline_message connect_db failed!");
                return -1;
            }
        }
        
        std::auto_ptr<mongo::DBClientCursor> cursor = m_Connection->query(m_TableName, query);
        while (cursor->more()) 
        {
            mongo::BSONObj p = cursor->next();
            MQTTPublish* publish = NULL;
            ZNEW(publish, MQTTPublish);

            publish->m_SenderID = p.getField("Sender").Long();
            publish->m_ReceiverID = p.getField("Receiver").Long();
            publish->m_MultReceivers = p.getField("MultReceiver").String();
            publish->m_MultReceivers = p.getField("Topic").String();
            publish->m_SenderName = p.getField("SenderName").String();
            publish->m_SenderName = p.getField("GroupName").String();
            publish->m_MessageType= (unsigned short)(p.getIntField("MessageType"));
            publish->m_ReceiverType= (unsigned short)(p.getIntField("ReceiverType"));
            publish->m_SendMessageID = (unsigned short)(p.getIntField("MessageID"));
            int offset = p.getIntField("offset");
            publish->m_TimeStamp = (time_t)(p.getField("TimeStamp").Long());
            mongo::BSONElement e = p.getField("Payload");
            const char * data = e.binData(publish->m_RemainingLength);
            ZNEW_S(publish->m_RemainingData, uchar, publish->m_RemainingLength);
            memcpy(publish->m_RemainingData, data, publish->m_RemainingLength);
            publish->m_PublishData = publish->m_RemainingData + offset;
            publish->m_PublishDataLength = publish->m_RemainingLength - offset;
            
            offline_messages.push_back(publish);
        }
    }
    catch(mongo::DBException &e )
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Offline_API delete offline msg error: %s\n", strerrmsg.c_str());
        return -1;
    }

    if (!offline_messages.empty())
    {
        LOGINFO("Mongo_Offline_API now delete user[%lld]'s offline message.", user_id);
        try
        {
            m_Connection->remove(m_TableName, query);
        }
        catch(mongo::DBException &e )
        {
            std::string strerrmsg = e.what();
            LOGWARN("Mongo_Offline_API delete offline msg error: %s\n", strerrmsg.c_str());
            return -1;
        }
    }
    
    return 0;
}
