#ifndef __USER_INFO_API_H__
#define __USER_INFO_API_H__

#include <string>
#include <list>
#include "zbasedef.h"
#include "db_info.h"

class User_Info_API
{
public:
    User_Info_API(){};
    virtual ~User_Info_API(){};

public:
    // 初始化
    virtual int init(const DBInfo& info) = 0;

    // 查询所有用户信息并保持在内存结构中， 主要是用户名和密码对应， 用来进行鉴权的
    virtual int query_user_info(long long user_id, std::string& password) = 0;
    
    // 查询接受方是某个用户的所有记录
    virtual int query_user_group_info(long long group_id, std::list<UserIDType>& group_users) = 0;
    
protected:
    DBInfo   m_Info;
};
#endif
