#ifndef __MONGO_CHAT_RECORD_API_H__
#define __MONGO_CHAT_RECORD_API_H__

#include <string>
#include "chat_record_api.h"

class ODBC_Chat_Record_API : public Chat_Record_API
{
public:
    enum
    {
        PAYLOAD_BUF_SIZE = 4096,
    };
public:
    ODBC_Chat_Record_API();
    virtual ~ODBC_Chat_Record_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);

    //向数据库插入聊天记录 
    virtual int insert_chat_record(long long sender,
                                     const std::string& topic_name,
                                     unsigned short message_id,
                                     time_t send_time,
                                     char* msg_data,
                                     int   msg_length,
                                     bool is_group);
private:
    int connect_db(const DBInfo& info);
    
    void test(const char* table_name);
    
private:
    mongo::DBClientConnection     m_Connection;
    std::string                   m_TableName;
    //char                          m_PayloadBuffer[PAYLOAD_BUF_SIZE*4];
};
#endif
