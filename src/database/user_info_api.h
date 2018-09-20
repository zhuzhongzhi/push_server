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
    // ��ʼ��
    virtual int init(const DBInfo& info) = 0;

    // ��ѯ�����û���Ϣ���������ڴ�ṹ�У� ��Ҫ���û����������Ӧ�� �������м�Ȩ��
    virtual int query_user_info(long long user_id, std::string& password) = 0;
    
    // ��ѯ���ܷ���ĳ���û������м�¼
    virtual int query_user_group_info(long long group_id, std::list<UserIDType>& group_users) = 0;
    
protected:
    DBInfo   m_Info;
};
#endif
