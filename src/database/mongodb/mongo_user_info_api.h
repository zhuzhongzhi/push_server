#ifndef __MONGO_USER_INFO_API_H__
#define __MONGO_USER_INFO_API_H__

#include <string>
#include "mongo/client/dbclient.h"
#include "user_info_api.h"

class Mongo_User_Info_API : public User_Info_API
{
public:
    Mongo_User_Info_API();
    virtual ~Mongo_User_Info_API();

public:
    // 初始化
    virtual int init(const DBInfo& info);


    // 查询所有用户信息并保持在内存结构中， 主要是用户名和密码对应， 用来进行鉴权的
    virtual int query_user_info(long long user_id, std::string& password);

    // 查询接受方是某个用户的所有记录
    virtual int query_user_group_info(long long group_id, std::list<UserIDType>& group_users);

    int query_user_group_info_by_type(long long group_id, std::list<UserIDType>& group_users, int type);
private:
    int connect_db(const DBInfo& info);
    
private:
    mongo::DBClientConnection*    m_Connection;
    std::string                   m_UserInfoTableName;
    std::string                   m_GroupTableName;
};
#endif
