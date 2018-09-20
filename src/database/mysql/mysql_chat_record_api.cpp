
#include "zlogger.h"
#include "mysql_chat_record_api.h"

Mysql_Chat_Record_API::Mysql_Chat_Record_API()
{
    m_TableName = "user_chat_record";
}

Mysql_Chat_Record_API::~Mysql_Chat_Record_API()
{
    
}

int Mysql_Chat_Record_API::init(const DBInfo& info)
{
    m_Info = info;

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_Chat_Record_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mysql_Chat_Record_API::connect_db(const DBInfo& info)
{
#ifdef USE_MYSQL
    if (NULL == mysql_init(&m_Connection))
    {
        LOGWARN("Mysql_Chat_Record_API init db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }

    //设置链接超时时间.
    int timeout = 5;
    mysql_options(&m_Connection, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    //设置查询数据库(select)超时时间
    mysql_options(&m_Connection, MYSQL_OPT_READ_TIMEOUT, &timeout);
    //设置写数据库(update,delect,insert,replace等)的超时时间。
    mysql_options(&m_Connection, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (NULL == mysql_real_connect(&m_Connection, 
                                                         info.m_IP.c_str(), 
                                                         info.m_UserName.c_str(), 
                                                         info.m_Password.c_str(), 
                                                         info.m_DBName.c_str(), 
                                                         info.m_Port, 
                                                         NULL, 
                                                         0))
    {
        LOGWARN("Mysql_Chat_Record_API connect db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
#endif
    return 0;
}

int Mysql_Chat_Record_API::insert_chat_record(long long sender,
                                              long long receiver,
                                              long long extend,
                                     unsigned short message_id,
                                     time_t send_time,
                                     char* msg_data,
                                     int   msg_length,
                                     bool is_group)
{
    LOGINFO("Enter Mysql_Chat_Record_API insert_chat_record.");
#ifdef USE_MYSQL
    if (0 != check_db_conn())
    {
        return -1;
    }

    char *blob_sql = new char[RECORD_BLOB_BUFFER_SIZE];
    memset(blob_sql, 0, RECORD_BLOB_BUFFER_SIZE);
    
    sprintf(blob_sql,
        "insert into %s(Sender, Receiver, Extend, MessageID, DateTime, Payload) values (%lld, %lld, %lld, %d, %lld, '",
        m_TableName.c_str(), sender, receiver, extend, message_id, (long long)send_time);
    char * end = blob_sql + strlen(blob_sql);
    end += mysql_real_escape_string(&m_Connection, end, msg_data, msg_length);
    *end++ = '\'';
    *end++ = ')';

    LOGINFO("Mysql_Chat_Record_API insert_chat_record sql is %s.", blob_sql);
    if (0 != mysql_query(&m_Connection, blob_sql))
    {
        LOGWARN("Mysql_Chat_Record_API insert_chat_record error: %s\n", mysql_error(&m_Connection));
        delete[] blob_sql;
        return -1;
    }

    delete[] blob_sql;
#endif
    return 0;
}

int Mysql_Chat_Record_API::check_db_conn()
{
    LOGINFO("Enter Mysql_Chat_Record_API check_db_conn.");

#ifdef USE_MYSQL
    if (0 == mysql_ping(&m_Connection))
    {
        return 0;
    }

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_Chat_Record_API init connect_db failed!");
        return -1;
    }
#endif
    return 0;
}


