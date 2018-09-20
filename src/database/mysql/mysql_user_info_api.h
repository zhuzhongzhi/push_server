#ifndef __MYSQL_USER_INFO_API_H__
#define __MYSQL_USER_INFO_API_H__

#include <string>
#ifdef USE_MYSQL
#include <mysql/mysql.h>
#endif
#include "user_info_api.h"

class Mysql_User_Info_API : public User_Info_API
{
public:
    Mysql_User_Info_API();
    virtual ~Mysql_User_Info_API();

public:
    // ��ʼ��
    virtual int init(const DBInfo& info);

    // ��ѯ�����û���Ϣ���������ڴ�ṹ�У� ��Ҫ���û����������Ӧ�� �������м�Ȩ��
    virtual int query_user_info(long long user_id, std::string& password);
    
    // ��ѯ���ܷ���ĳ���û������м�¼
    virtual int query_user_group_info(long long group_id, std::list<UserIDType>& group_users);                                 

    virtual int check_db_conn();

private:
    int connect_db(const DBInfo& info);
    
private:
#ifdef USE_MYSQL    
    MYSQL                 m_Connection;
#endif
    std::string                   m_TableName;
    std::string                   m_GroupTableName;

};
#endif
