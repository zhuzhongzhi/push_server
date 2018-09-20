#ifndef __MYSQL_CHAT_RECORD_H__
#define __MYSQL_CHAT_RECORD_H__

#include <string>
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif
#include "chat_record_api.h"
class Mysql_Chat_Record_API : public Chat_Record_API
{
public:
    Mysql_Chat_Record_API();
    virtual ~Mysql_Chat_Record_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);

    //向数据库插入聊天记录 
    virtual int insert_chat_record(long long sender,
                                   long long receiver,
                                   long long extend,
                                     unsigned short message_id,
                                     time_t send_time,
                                     char* msg_data,
                                     int   msg_length,
                                     bool is_group);
    virtual int check_db_conn();
private:
    int connect_db(const DBInfo& info);
    
private:
#ifdef USE_MYSQL     
    MYSQL                        m_Connection;
#endif
    std::string                   m_TableName;
};
#endif
