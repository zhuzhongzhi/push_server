#include "base64.h"
#include "publish_message.h"
#include "logger.h"
#include "odbc_auto_db_connect.h"
#include "odbc_offline_api.h"

ODBC_Offline_API::ODBC_Offline_API()
{
}

ODBC_Offline_API::~ODBC_Offline_API()
{    
}

int ODBC_Offline_API::init(const DBInfo& info)
{
    m_Info = info;
    m_Connection = new auto_db_connect(NULL, m_Info);
    if (NULL != m_Connection)
    {
        LOGWARN("ODBC_Offline_API init create m_Connection failed!");
        return -1;
    }

    return 0;
}

int ODBC_Offline_API::insert_offline_message(long long sender,
                                                 long long receiver,
                                                 const std::string& topic_name,
                                                 unsigned short message_id,
                                                 time_t send_time,
                                                 char* msg_data,
                                                 int   msg_length)
{    
    otl_connect* db = m_Connection->get_connect();
    if (NULL == db)
    {
        LOGWARN("ODBC_User_Info_API query_user_info otl_connect is NULL, need reconnect!");
        if (!m_Connection->reconnect())
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect failed!");
            return -1;
        }
        else
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect success!");
            db = m_Connection->get_connect();
        }
    }

    std::string insert_offline_sql;
    long long group = atoll(topic_name.c_str());
    if (receiver == group)
    {
          group = 0;
    }
    char buffer[240] = {0};
    sprintf(buffer, "insert into user_offline_record values(%lld,%lld,%lld,%d,%lld)", 
         sender, receiver, group, message_id, send_time);
    std::string insert_offline_sql(buffer);

    otl_stream in_insert_offline(1, //buffer size
                  insert_offline_sql.c_str(),// SELECTstatement
                  *db // connectobject
                  );
    LOGINFO("Ecec Sql is %s", insert_offline_sql.c_str()); 
    
    try
    {
        mongo::BSONObjBuilder builder;
        builder.append("Sender", sender);
        builder.append("Receiver", receiver);
        long long group = atoll(topic_name.c_str());
        if (receiver == group)
        {
              group = 0;
        }
        builder.append("GroupID", group);
        builder.append("MessageID", message_id);
        builder.append("DateTime", (long long)send_time);
        /* base64
        int dst_length = 0;
        char *dst = NULL;
        bool need_delete = false;
        if (msg_length <= PAYLOAD_BUF_SIZE)
        {
            dst = m_PayloadBuffer;
            dst_length = PAYLOAD_BUF_SIZE*4;
        }
        else
        {
            dst = new char[msg_length*4];
            need_delete = true;
        }

        CBase64::encode(msg_data, msg_length, dst, dst_length);
        dst[dst_length] = '\0';
        builder.append("Payload", dst);
        if (need_delete)
        {
            delete[] dst;
            dst = NULL;
        }
        */        
        builder.appendBinData("Payload", msg_length, mongo::BinDataGeneral, (const void *)msg_data);
        m_Connection.insert(m_TableName, builder.obj());
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Offline_API insert_offline_message error: %s\n", strerrmsg.c_str());
        return -1;
    }

    return 0;
}

int ODBC_Offline_API::query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages)
{
    char szDest[128] = {0}; 
    sprintf(szDest, "this.Receiver == %lld", user_id);
    mongo::Query query;
    query.where(szDest);
    try
    {        
        std::auto_ptr<mongo::DBClientCursor> cursor = m_Connection.query(m_TableName, query);
        while (cursor->more()) 
        {
            mongo::BSONObj p = cursor->next();
            MQTTPublish* publish = NULL;
            ZNEW(MQTTPublish,publish);

            publish->m_SenderID = p.getField("Sender").Long();
            long long group = p.getField("GroupID").Long();
            if (0 == group)
            {
                group = user_id;
            }
            sprintf(szDest, "%lld", group);
            publish->m_TopicName = szDest;
            publish->m_MessageID = (unsigned short)(p.getIntField("MessageID"));
            publish->m_MsgTime = (time_t)(p.getField("DateTime").Long());
            /*
            const char * src = p.getStringField("Payload");
            int src_length = strlen(src);            
            int dst_length = src_length;
            char *dst = NULL;
            dst = new char[src_length + 1];                   

            CBase64::decode(src, src_length, dst, dst_length);

            publish->m_PublishData = dst;
            dst = NULL;
            publish->m_PublishDataLength = dst_length;
            */
            mongo::BSONElement e = p.getField("Payload");
            const char * data = e.binData(publish->m_PublishDataLength);
            try
            {
                publish->m_PublishData = new char[publish->m_PublishDataLength + 1];
            }
            catch(...)
            {
                publish->m_PublishData = NULL;
            }
            if (NULL == publish->m_PublishData)
            {
                LOGWARN("new MQTTPublish payload blob failed, not enough memory.");
                return -1;
            }
            else
            {            
                memcpy(publish->m_PublishData, data, publish->m_PublishDataLength);
                publish->m_PublishData[publish->m_PublishDataLength] = 0;
            }
            
            // publish ��Ϣ�ĳ���
            publish->m_RemainingLength = publish->m_TopicName.length() + sizeof(unsigned short)
                 + sizeof(unsigned short) + publish->m_PublishDataLength;
            offline_messages.push_back(publish);
        }
    }
    catch(mongo::DBException &e )
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Offline_API delete offline msg error: %s\n", strerrmsg.c_str());
        return -1;
    }

    // ��ѯ���ˣ� ɾ���Ѿ������ļ�¼
    if (!offline_messages.empty())
    {
        LOGINFO("Mongo_Offline_API now delete user[%lld]'s offline message.", user_id);
        try
        {
            m_Connection.remove(m_TableName, query);
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
