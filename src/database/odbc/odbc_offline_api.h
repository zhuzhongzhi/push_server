#ifndef __ODBC_OFFLINE_API_H__
#define __ODBC_OFFLINE_API_H__

#include <string>
#include "offline_api.h"

class auto_db_connect;
class ODBC_Offline_API : public Offline_API
{
public:
    enum
    {
        PAYLOAD_BUF_SIZE = 4096,
    };
public:
    ODBC_Offline_API();
    virtual ~ODBC_Offline_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);

    //向数据库插入offline message
    virtual int insert_offline_message(long long sender,
                                          long long receiver,
                                          const std::string& topic_name,
                                          unsigned short message_id,
                                          time_t send_time,
                                          char* msg_data,
                                          int   msg_length);
    
    // 查询接受方是某个用户的所有记录
    virtual int query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages);                                   
    
private:
    auto_db_connect*              m_Connection;
};
#endif