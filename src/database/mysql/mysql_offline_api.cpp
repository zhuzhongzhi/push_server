#include "chat_record_api.h"
#include "zlogger.h"
#include "zmqttpublish.h"
#include "mysql_offline_api.h"

Mysql_Offline_API::Mysql_Offline_API()
{
    m_TableName = "user_offline_record";
}

Mysql_Offline_API::~Mysql_Offline_API()
{    
}

int Mysql_Offline_API::init(const DBInfo& info)
{
    m_Info = info;

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_Offline_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mysql_Offline_API::connect_db(const DBInfo& info)
{
    LOGINFO("Enter Mysql_Chat_Record_API connect_db.");
#ifdef USE_MYSQL
    if (NULL == mysql_init(&m_Connection))
    {
        LOGWARN("Mysql_Offline_API init db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
    int timeout = 5;
    mysql_options(&m_Connection, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    mysql_options(&m_Connection, MYSQL_OPT_READ_TIMEOUT, &timeout);
    mysql_options(&m_Connection, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (NULL == mysql_real_connect(&m_Connection, info.m_IP.c_str(), info.m_UserName.c_str(), info.m_Password.c_str(), info.m_DBName.c_str(), info.m_Port, NULL, 0))
    {
        LOGWARN("Mysql_Offline_API connect db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
#endif
    return 0;
}

int Mysql_Offline_API::insert_offline_message(NLMessage* msg)
{
    LOGINFO("Enter Mysql_Chat_Record_API insert_offline_message.");
#ifdef USE_MYSQL

    if (0 != check_db_conn())
    {
        return -1;
    }
    
    char *blob_sql = new char[RECORD_BLOB_BUFFER_SIZE];
    memset(blob_sql, 0, RECORD_BLOB_BUFFER_SIZE);
    MQTTPublish* publish = (MQTTPublish*)msg;
    int offset = publish->m_PublishData - publish->m_RemainingData;
    sprintf(blob_sql,
        "insert into %s(Sender, Receiver, MultReceiver, Topic, SenderName, GroupName, MessageType, ReceiverType, MessageID, Offset, TimeStamp, Payload) "
        "values  (%lld, %lld, %s, %s, %s, %s, %d, %d, %d, %d, %u, '",
        m_TableName.c_str(), 
        publish->m_SenderID, 
        publish->m_ReceiverID, 
        publish->m_MultReceivers.c_str(),
        publish->m_TopicName.c_str(),
        publish->m_SenderName.c_str(),
        publish->m_GroupName.c_str(),
        publish->m_MessageType,
        publish->m_ReceiverType,
        publish->m_SendMessageID,
        offset, 
        publish->m_TimeStamp);
    char * end = blob_sql + strlen(blob_sql);
    end += mysql_real_escape_string(&m_Connection, end, 
                                    (const char*)(publish->m_RemainingData), 
                                    (unsigned long)(publish->m_RemainingLength));
    *end++ = '\'';
    *end++ = ')';

    LOGINFO("Mysql_Offline_API insert_offline_message sql is %s.", blob_sql);
    if (0 != mysql_query(&m_Connection, blob_sql))
    {
        LOGWARN("Mysql_Offline_API insert_offline_message error: %s\n", mysql_error(&m_Connection));
        delete[] blob_sql;
        return -1;
    }
    delete[] blob_sql;
#endif
    return 0;
}

int Mysql_Offline_API::query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages)
{
    LOGINFO("Enter Mysql_Chat_Record_API query_user_offline_message.");
#ifdef USE_MYSQL
    if (0 != check_db_conn())
    {
        return -1;
    }

    char szDest[64] = {0}; 
    char szSql[256] = {0};

    sprintf(szSql, "select Sender, Receiver, MultReceiver, Topic, SenderName, GroupName, MessageType, ReceiverType, MessageID, Offset, TimeStamp, Payload from %s where Receiver = %lld", 
              m_TableName.c_str(), user_id);

    if (0 != mysql_query(&m_Connection, szSql))
    {
        printf("[log] select offline msg error: %s\n", mysql_error(&m_Connection));
        return -1;
    }

    MYSQL_RES* pResult = mysql_store_result(&m_Connection);
    if (!pResult)
    {
        printf("[log] select offline msg error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
    
    MYSQL_ROW row = mysql_fetch_row(pResult);
    while (NULL != row)
    {
        MQTTPublish* publish = NULL;
        ZNEW(publish, MQTTPublish);

        publish->m_SenderID = atoll(row[0]);
        publish->m_ReceiverID= atoll(row[1]);
        publish->m_MultReceivers = row[2];
        publish->m_TopicName = row[3];
        publish->m_SenderName = row[4];
        publish->m_GroupName = row[5];
        publish->m_MessageType = atol(row[6]);
        publish->m_ReceiverType = atol(row[7]);
        publish->m_SendMessageID = atol(row[8]);
        int offset = atol(row[9]);
        publish->m_TimeStamp = atol(row[10]);
        

        unsigned long *FieldLength = mysql_fetch_lengths(pResult);
        publish->m_RemainingLength = FieldLength[11];
        publish->m_PublishDataLength = publish->m_RemainingLength - offset;

        ZNEW_S(publish->m_RemainingData, uchar, publish->m_RemainingLength);
        memcpy(publish->m_RemainingData, row[11], publish->m_RemainingLength);
        publish->m_PublishData = publish->m_RemainingData + offset;

        offline_messages.push_back(publish);

        row = mysql_fetch_row(pResult);
    }

    sprintf(szSql, "delete from %s where Receiver = %lld", m_TableName.c_str(), user_id);

    if (0 != mysql_query(&m_Connection, szSql))
    {
        LOGWARN("Mysql_Offline_API delete offline msg error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
#endif
    return 0;
}

int Mysql_Offline_API::check_db_conn()
{
    LOGINFO("Enter Mysql_Chat_Record_API check_db_conn.");
#ifdef USE_MYSQL
    if (0 == mysql_ping(&m_Connection))
    {
        return 0;
    }

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_Offline_API init connect_db failed!");
        return -1;
    }
#endif
    return 0;

}

