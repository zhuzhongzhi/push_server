#include <string.h>
#include "logger.h"
#include "otlv4.h"
#include "stringtools.h"
#include "odbc_auto_db_connect.h"
#include "odbc_user_info_api.h"

ODBC_User_Info_API::ODBC_User_Info_API()
{
}

ODBC_User_Info_API::~ODBC_User_Info_API()
{
    
}

int ODBC_User_Info_API::init(const DBInfo& info)
{
    m_Info = info;
    m_Connection = new auto_db_connect(NULL, m_Info);
    if (NULL != m_Connection)
    {
        LOGWARN("ODBC_User_Info_API init create m_Connection failed!");
        return -1;
    }

    return 0;
}

int ODBC_User_Info_API::query_user_info(long long user_id, std::string& password)
{
    otl_connect* db = m_Connection->get_connect();
    if (NULL == db)
    {
        LOGWARN("ODBC_User_Info_API query_user_info otl_connect is NULL, need reconnect!");
        if (!m_Connection->reconnect())
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect failed!");
            return -1;
        }
        else
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect success!");
            db = m_Connection->get_connect();
        }
    }

    std::string query_user_sql("select Password from user_info where UserID = ");
    char buffer[24] = {0};
    sprintf(buffer, "%lld", user_id);
    query_user_sql += buffer;
    
    otl_stream in_query_user(1, //buffer size
                  query_user_sql.c_str(),// SELECTstatement
                  *db // connectobject
                  );
    LOGINFO("Ecec Sql is %s", query_user_sql.c_str()); 

    std::string password;
    try
    {
        while (!in_query_user.eof())
        {
            // while not end-of-data            
            in_query_user>>password;            
            LOGINFO("query_user_info User[%lld] Password[%s]!", user_id, password.c_str());
        }
    }
    catch (otl_exception& p)
    { 
        LOGWARN("query_user_info in_query_user exception, msg[%s],sql[%s],var[%s]",
            p.msg, p.stm_text, p.var_info);

        if (!m_Connection->reconnect())
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect failed!");
            return -1;
        }

        // try it again
        otl_stream in_query_user_re(1, //buffer size
                  query_user_sql.c_str(),// SELECTstatement
                  *db // connectobject
                  );
        LOGINFO("ReEcec Sql is %s", query_user_sql.c_str()); 
        try
        {
            while (!in_query_user_re.eof())
            {
                // while not end-of-data            
                in_query_user_re>>password;            
                LOGINFO("query_user_info User[%lld] Password[%s]!", user_id, password.c_str());
            }
        }
        catch (otl_exception& p)
        {
            LOGWARN("query_user_info retry in_query_user exception, msg[%s],sql[%s],var[%s]",
            p.msg, p.stm_text, p.var_info);
            return -1;
        }
    }    
    
    return 0;
}

int ODBC_User_Info_API::query_user_group_info(long long group_id, std::list<UserIDType>& group_users)
{
    otl_connect* db = m_Connection->get_connect();
    if (NULL == db)
    {
        LOGWARN("ODBC_User_Info_API query_user_info otl_connect is NULL, need reconnect!");
        if (!m_Connection->reconnect())
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect failed!");
            return -1;
        }
        else
        {
            LOGWARN("ODBC_User_Info_API query_user_info reconnect success!");
            db = m_Connection->get_connect();
        }
    }

    std::string query_group_sql("select userlist from group_data where groupid = ");
    char buffer[24] = {0};
    sprintf(buffer, "%lld", group_id);
    query_group_sql += buffer;
    
    otl_stream in_query_group(1, //buffer size
                  query_group_sql.c_str(),// SELECTstatement
                  *db // connectobject
                  );
    LOGINFO("Ecec Sql is %s", query_group_sql.c_str()); 

    std::string user_list;
    try
    {
        while (!in_query_group.eof())
        {
            // while not end-of-data            
            in_query_group>>user_list;            
            LOGINFO("query_user_info GroupId[%lld] UserList[%s]!", group_id, user_list.c_str());
        }
    }
    catch (otl_exception& p)
    { 
        LOGWARN("query_user_group_info in_query_group exception, msg[%s],sql[%s],var[%s]",
            p.msg, p.stm_text, p.var_info);
        
        if (!m_Connection->reconnect())
        {
            LOGWARN("ODBC_User_Info_API query_user_group_info reconnect failed!");
            return -1;
        }

        // try it again
        otl_stream in_query_group_re(1, //buffer size
                  query_group_sql.c_str(),// SELECTstatement
                  *db // connectobject
                  );
        LOGINFO("ReEcec Sql is %s", query_group_sql.c_str()); 
        try
        {
            while (!in_query_group_re.eof())
            {
                // while not end-of-data            
                in_query_group_re>>user_list;            
                LOGINFO("query_user_info GroupId[%lld] UserList[%s]!", group_id, user_list.c_str());
            }
        }
        catch (otl_exception& p)
        { 
            LOGWARN("query_user_group_info in_query_group exception, msg[%s],sql[%s],var[%s]",
                p.msg, p.stm_text, p.var_info);
            return -1;
        }        
    }    

    // 解析所有的user id
    if (!user_list.empty())
    {
        std::list<std::string> out_list;
        if (0 == split_string(user_list.c_str(), ';', out_list))
        {
            std::list<std::string>::iterator iter;
            for (iter = out_list.begin(); iter != out_list.end(); ++iter)
            {
                std::string& userid_string = *iter;
                UserIDType user_id = atoll(userid_string.c_str());
                LOGINFO("query_user_info Group[%lld] include User[%lld]!", group_id, user_id);
                group_users.push_back(user_id);
            }
        }
    }
    
    return 0;    
}


