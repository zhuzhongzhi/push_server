#ifndef __MONGO_OFFLINE_API_H__
#define __MONGO_OFFLINE_API_H__

#include <string>
#include "mongo/client/dbclient.h"
#include "offline_api.h"

class Mongo_Offline_API : public Offline_API
{
public:
    enum
    {
        PAYLOAD_BUF_SIZE = 4096,
    };
public:
    Mongo_Offline_API();
    virtual ~Mongo_Offline_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);

    //向数据库插入offline message
    virtual int insert_offline_message(NLMessage* msg);
    
    // 查询接受方是某个用户的所有记录
    virtual int query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages);                                   

private:
    int connect_db(const DBInfo& info);
    
private:
    mongo::DBClientConnection*    m_Connection;
    std::string                   m_TableName;
};
#endif