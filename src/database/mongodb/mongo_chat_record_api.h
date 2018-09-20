#ifndef __MONGO_CHAT_RECORD_API_H__
#define __MONGO_CHAT_RECORD_API_H__

#include <string>
#include "mongo/client/dbclient.h"
#include "chat_record_api.h"

class Mongo_Chat_Record_API : public Chat_Record_API
{
public:
    enum
    {
        PAYLOAD_BUF_SIZE = 4096,
    };
public:
    Mongo_Chat_Record_API();
    virtual ~Mongo_Chat_Record_API();

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
private:
    int connect_db(const DBInfo& info);
    
    void test(const char* table_name);
    
private:
    mongo::DBClientConnection*    m_Connection;
    std::string                   m_TableName;
};
#endif
