#ifndef __CHAT_RECORD_API_H__
#define __CHAT_RECORD_API_H__

#include "db_info.h"
#define RECORD_BLOB_BUFFER_SIZE 409600
class Chat_Record_API
{
public:
    Chat_Record_API(){};
    virtual ~Chat_Record_API(){};

public:
    // 初始化
    virtual int init(const DBInfo& info) = 0;

    //向数据库插入聊天记录 
    virtual int insert_chat_record(long long sender,
                                   long long receiver,
                                   long long extend,
                                     unsigned short message_id,
                                     time_t send_time,
                                     char* msg_data,
                                     int   msg_length,
                                     bool is_group) = 0;
    
protected:
    DBInfo   m_Info;
};
#endif