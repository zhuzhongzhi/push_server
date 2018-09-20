#ifndef __MYSQL_OFFLINE_API_H__
#define __MYSQL_OFFLINE_API_H__

#include <string>
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif
#include "offline_api.h"

class Mysql_Offline_API : public Offline_API
{
public:
    Mysql_Offline_API();
    virtual ~Mysql_Offline_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);

    //向数据库插入offline message
    virtual int insert_offline_message(NLMessage* msg);
    
    // 查询接受方是某个用户的所有记录
    virtual int query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages);                                   

    virtual int check_db_conn();

private:
    int connect_db(const DBInfo& info);
    
private:
#ifdef USE_MYSQL    
    MYSQL                 m_Connection;
#endif
    std::string                   m_TableName;
};
#endif
