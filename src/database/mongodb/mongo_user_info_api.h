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
    // ��ʼ��
    virtual int init(const DBInfo& info);


    // ��ѯ�����û���Ϣ���������ڴ�ṹ�У� ��Ҫ���û����������Ӧ�� �������м�Ȩ��
    virtual int query_user_info(long long user_id, std::string& password);

    // ��ѯ���ܷ���ĳ���û������м�¼
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
