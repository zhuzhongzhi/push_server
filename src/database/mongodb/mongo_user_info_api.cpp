#include <string.h>
#include "zlogger.h"
#include "mongo_user_info_api.h"

Mongo_User_Info_API::Mongo_User_Info_API()
{
    m_UserInfoTableName = ".user_info";
    m_GroupTableName = ".group_data";
    m_Connection = NULL;
}

Mongo_User_Info_API::~Mongo_User_Info_API()
{
    if (NULL != m_Connection)
    {
        ZDELETE(m_Connection);
    }
}

int Mongo_User_Info_API::init(const DBInfo& info)
{
    m_Info = info;
    m_UserInfoTableName = info.m_DBName + m_UserInfoTableName;
    m_GroupTableName = info.m_DBName + m_GroupTableName;
    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mongo_User_Info_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mongo_User_Info_API::connect_db(const DBInfo& info)
{
    if (NULL != m_Connection)
    {
        ZDELETE(m_Connection);
    }

    ZNEW(m_Connection, mongo::DBClientConnection);
    
    char szDest[128] = {0};
    sprintf(szDest, "%s:%d", info.m_IP.c_str(), info.m_Port);
    std::string strErrorMsg;
    try
    {
        if (!m_Connection->connect(szDest, strErrorMsg))
        {
            LOGWARN("Mongo_User_Info_API connect db error: %s\n", strErrorMsg.c_str());
            return -1;
        }

        if (!m_Connection->auth(info.m_DBName, info.m_UserName, info.m_Password, strErrorMsg))
        {
            LOGWARN("Mongo_User_Info_API authorize check error: %s\n", strErrorMsg.c_str());
            return -1;
        }
    }
    catch(mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_User_Info_API init error: %s\n", strerrmsg.c_str());
        return -1;
    }

    return 0;
}

int Mongo_User_Info_API::query_user_info(long long user_id, std::string& password)
{
    
    /*  for test
       mongo::BSONObjBuilder builder1;
       try
       {
           long long user = 15996450077;
           builder1.append("UserID", user);
           builder1.append("Password", "zhanglingmei");
           m_Connection.insert(m_UserInfoTableName, builder1.obj());
       }
       catch(mongo::DBException &e)
       {
           std::string strerrmsg = e.what();
           LOGWARN("Mongo_User_Info_API query_user_info insert error: %s\n", strerrmsg.c_str());
           return -1;
       }
       */
    try
    {
        if (m_Connection->isFailed())
        {
            if (0 != connect_db(m_Info))
            {
                LOGWARN("query_user_info connect_db failed!");
                return -1;
            }
        }
        mongo::BSONObjBuilder builder;
        builder.append("UserID", user_id);
        mongo::Query query(builder.obj());
        std::auto_ptr<mongo::DBClientCursor> cursor = m_Connection->query(m_UserInfoTableName, query);
    
        while (cursor->more())
        {
            mongo::BSONObj p = cursor->next();
            password = p.getStringField("Password");
            LOGINFO("Get one UserID[%lld] Password[%s]", user_id, password.c_str());
        }
    }
    catch (mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_User_Info_API query_user_info error: %s\n", strerrmsg.c_str());
        return -1;
    }
    
    return 0;
}

int Mongo_User_Info_API::query_user_group_info(long long group_id, std::list<UserIDType>& group_users)
{
    int result = query_user_group_info_by_type(group_id, group_users, 1);
    if (0 == result)
    {
        LOGINFO("group_id[%lld] query as long success!\n", group_id);
    }
    else
    {
        LOGWARN("group_id[%lld] query as long failed, try query as int!\n", group_id);
        result = query_user_group_info_by_type(group_id, group_users, 0);
    }
    
    return result;
}

int Mongo_User_Info_API::query_user_group_info_by_type(long long group_id, std::list<UserIDType>& group_users, int type)
{
    std::auto_ptr<mongo::DBClientCursor> cursor;

    char* user_ids = NULL;
    try
    {
        if (m_Connection->isFailed())
        {
            if (0 != connect_db(m_Info))
            {
                LOGWARN("query_user_group_info_by_type connect_db failed!");
                return -1;
            }
        }
        
        if (0 == type)
        {
            cursor = m_Connection->query(m_GroupTableName, MONGO_QUERY("groupid" << (int)group_id));
        }
        else
        {
            cursor = m_Connection->query(m_GroupTableName, MONGO_QUERY("groupid" << group_id));
        }
        
        while (cursor->more())
        {
            mongo::BSONObj p = cursor->next();
            int user_num = p.getIntField("len");
            LOGINFO("group_id[%lld] user_num[%d]\n", group_id, user_num);
            
            mongo::BSONElement e = p.getField("users");
            std::vector<mongo::BSONElement> elements = e.Array();
            std::vector<mongo::BSONElement>::iterator it;
            for (it = elements.begin(); it != elements.end(); ++it)
            {
                mongo::BSONElement &ele = *it;
                UserIDType user_id = 0;
                if (mongo::NumberLong == ele.type())
                {
                    user_id = ele.Long();
                }
                else if (mongo::NumberInt == ele.type())
                {
                    user_id = (UserIDType)(ele.Int());
                }
                else
                {
                    LOGWARN("user id type is %d", (int)(ele.type()));
                    continue;
                }
                group_users.push_back(user_id);
                LOGINFO("one of user_id is %lld", user_id);
            }            
        }
    }
    catch (mongo::DBException &e)
    {
        std::string strerrmsg = e.what();
        LOGWARN("Mongo_User_Info_API query_user_group_info error: %s\n", strerrmsg.c_str());
        return -1;
    }
    catch (...)
    {
        LOGWARN("Mongo_User_Info_API query_user_group_info other exception");
        return -1;
    }

    return 0;
}



