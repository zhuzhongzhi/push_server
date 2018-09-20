
#include "zbase64.h"
#include "zlogger.h"
#include "mongo_chat_record_api.h"

Mongo_Chat_Record_API::Mongo_Chat_Record_API()
{
    m_TableName = ".chat_record";
    m_Connection = NULL;
}

Mongo_Chat_Record_API::~Mongo_Chat_Record_API()
{
    if (NULL != m_Connection)
    {
        ZDELETE(m_Connection);
    }
}

int Mongo_Chat_Record_API::init(const DBInfo& info)
{
    m_Info = info;
    m_TableName = info.m_DBName + m_TableName;
    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mongo_Chat_Record_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mongo_Chat_Record_API::connect_db(const DBInfo& info)
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
            LOGWARN("Mongo_Chat_Record_API connect db error: %s\n", strErrorMsg.c_str());
            return -1;
        }

        if (!m_Connection->auth(info.m_DBName, info.m_UserName, info.m_Password, strErrorMsg))
        {
            LOGWARN("Mongo_Chat_Record_API authorize check error: %s\n", strErrorMsg.c_str());
            return -1;
        }
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Chat_Record_API init error: %s\n", strerrmsg.c_str());
        return -1;
    }

    //test("im_server.test");
    //test("test");
    return 0;
}

int Mongo_Chat_Record_API::insert_chat_record(long long sender,
                                     long long receiver,
                                     long long extend,
                                     unsigned short message_id,
                                     time_t send_time,
                                     char* msg_data,
                                     int   msg_length,
                                     bool is_group)
{
    //long long receiver = atoll(topic_name.c_str());
    LOGINFO("Mongo_Chat_Record_API::insert_chat_record Sender[%lld] Receiver[%lld] Extend[%lld] ID[%u] Length[%d]",
            sender, receiver, extend, message_id, msg_length);
    try
    {
        if (m_Connection->isFailed())
        {
            if (0 != connect_db(m_Info))
            {
                LOGWARN("insert_chat_record connect_db failed!");
                return -1;
            }
        }
        
        mongo::BSONObjBuilder builder;
        builder.append("Sender", sender);
        builder.append("IsGroup", is_group);
        builder.append("Receiver", receiver);
        builder.append("Extend", extend);
        builder.append("MessageID", message_id);
        builder.append("DateTime", (long long)send_time);
        /*
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
        m_Connection->insert(m_TableName, builder.obj());
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_Chat_Record_API insert_chat_record error: %s\n", strerrmsg.c_str());
        return -1;
    }

    return 0;
}


void Mongo_Chat_Record_API::test(const char* table_name)
{
    LOGINFO("Test for table %s.", table_name);
    std::string tableName(table_name);
    mongo::BSONObjBuilder builder1;
    try
    {
       long long user = 15996450077;
       builder1.append("UserID", user);
       builder1.append("Password", "zhanglingmei");
       m_Connection->insert(tableName, builder1.obj());
    }
    catch(mongo::DBException &e)
    {
       std::string strerrmsg = e.what();
       LOGWARN("Mongo_User_Info_API query_user_info insert error: %s\n", strerrmsg.c_str());
       return;
    }

    LOGINFO("test Insert recore success, now query...");

    char szDest[128] = {0}; 
    sprintf(szDest, "this.UserID == %lld", 15996450077);
    mongo::Query query;
    query.where(szDest);
    try
    {        
       std::auto_ptr<mongo::DBClientCursor> cursor = m_Connection->query(tableName, query);
       while (cursor->more()) 
       {
           mongo::BSONObj p = cursor->next();
           std::string password = p.getStringField("Password");
           LOGINFO("Get one UserID[%lld] Password[%s]", 15996450077, password.c_str());
       }
    }
    catch(mongo::DBException &e )
    {
       std::string strerrmsg = e.what();
       LOGWARN("Mongo_Offline_API delete offline msg error: %s\n", strerrmsg.c_str());
       return;
    }
}


