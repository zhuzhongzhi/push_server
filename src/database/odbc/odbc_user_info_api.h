#ifndef __ODBC_USER_INFO_API_H__
#define __ODBC_USER_INFO_API_H__

#include <string>
#include "user_info_api.h"

class auto_db_connect;

class ODBC_User_Info_API : public User_Info_API
{
public:
    ODBC_User_Info_API();
    virtual ~ODBC_User_Info_API();

public:
    // ��ʼ��
    virtual int init(const DBInfo& info);


    // ��ѯ�����û���Ϣ���������ڴ�ṹ�У� ��Ҫ���û����������Ӧ�� �������м�Ȩ��
    virtual int query_user_info(long long user_id, std::string& password);

    // ��ѯ���ܷ���ĳ���û������м�¼
    virtual int query_user_group_info(long long group_id, std::list<UserIDType>& group_users);    
    
private:
    auto_db_connect*              m_Connection;
};
#endif
