
#include "zlogger.h"
#include "mysql_user_info_api.h"

Mysql_User_Info_API::Mysql_User_Info_API()
{
    m_TableName = "user_info";
    m_GroupTableName = "group_data";
}

Mysql_User_Info_API::~Mysql_User_Info_API()
{    
}

int Mysql_User_Info_API::init(const DBInfo& info)
{
    m_Info = info;

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_User_Info_API init connect_db failed!");
        return -1;
    }

    return 0;
}

int Mysql_User_Info_API::connect_db(const DBInfo& info)
{
    LOGINFO("Enter Mysql_User_Info_API connect_db.");
#ifdef USE_MYSQL
    if (NULL == mysql_init(&m_Connection))
    {
        LOGWARN("Mysql_User_Info_API init db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
    int timeout = 5;
    //设置链接超时时间.
    mysql_options(&m_Connection, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
    //设置查询数据库(select)超时时间
    mysql_options(&m_Connection, MYSQL_OPT_READ_TIMEOUT, &timeout);
    //设置写数据库(update,delect,insert,replace等)的超时时间。
    mysql_options(&m_Connection, MYSQL_OPT_WRITE_TIMEOUT, &timeout);

    if (NULL == mysql_real_connect(&m_Connection, info.m_IP.c_str(), info.m_UserName.c_str(), info.m_Password.c_str(), info.m_DBName.c_str(), info.m_Port, NULL, 0))
    {
        LOGWARN("Mysql_User_Info_API connect db error: %s\n", mysql_error(&m_Connection));
        return -1;
    }
#endif
    return 0;
}

int Mysql_User_Info_API::query_user_info(long long user_id, std::string& password)
{
    LOGINFO("Enter Mysql_User_Info_API query_user_info.");
#ifdef USE_MYSQL
    if (0 != check_db_conn())
    {
        return -1;
    }

    char sql_buffer[128] = {0};
    sprintf(sql_buffer,
        "select Password from %s where UserID = %lld",
        m_TableName.c_str(), user_id);

    LOGINFO("Mysql_User_Info_API query_user_info sql is %s.", sql_buffer);
    if (0 != mysql_query(&m_Connection, sql_buffer))
    {
        LOGWARN("Mysql_User_Info_API select users mysql_query error: %s\n", mysql_error(&m_Connection));
        return -1;
    }

    MYSQL_RES* pResult = mysql_store_result(&m_Connection);
    if (!pResult)
    {
        LOGWARN("Mysql_User_Info_API select users mysql_store_result error: %s!", mysql_error(&m_Connection));
        return -1;
    }
    
    MYSQL_ROW row = mysql_fetch_row(pResult);
    while (NULL != row)
    {
        password = row[0];
        LOGINFO("Get one UserID[%lld] Password[%s]", user_id, password.c_str());
    }
    
#endif
    return 0;
}

int Mysql_User_Info_API::query_user_group_info(long long group_id, std::list<UserIDType>& group_users)
{
    LOGINFO("Enter Mysql_User_Info_API query_user_group_info.");
#ifdef USE_MYSQL
    if (0 != check_db_conn())
    {
        return -1;
    }

    char szSql[256] = {0};

    sprintf(szSql, "select users from %s where groupid = %lld", 
              m_TableName.c_str(), group_id);

    if (0 != mysql_query(&m_Connection, szSql))
    {
        LOGWARN("Mysql_User_Info_API select group uses mysql_query error: %s!", mysql_error(&m_Connection));
        return -1;
    }

    MYSQL_RES* pResult = mysql_store_result(&m_Connection);
    if (!pResult)
    {
        LOGWARN("Mysql_User_Info_API select group users mysql_store_result error: %s!", mysql_error(&m_Connection));
        return -1;
    }
    
    std::string users_string;
    MYSQL_ROW row = mysql_fetch_row(pResult);
    while (NULL != row)
    {
        users_string = row[0];
        LOGINFO("Mysql_User_Info_API query_user_group_info get one uses[%s].", 
                      users_string.c_str());
        break;
    }

    // 解析user_string ,  以|分割
    char user_id_buffer[64];
    int start, end;
    start = end = 0;
    for (int i = 0; i < users_string.length(); ++i)
    {
        if (users_string[i] == '|')
        {
            end = i;
            memcpy(user_id_buffer, users_string.c_str() + start, end - start);
            user_id_buffer[end - start] = '\0';
            UserIDType user_id = atoll(user_id_buffer);
            group_users.push_back(user_id);
            LOGINFO("Mysql_User_Info_API query_user_group_info get one use id[%lld].", 
                      user_id);
            end ++;
            start = end;
        }
    }
    
#endif
    return 0;
}

int Mysql_User_Info_API::check_db_conn()
{
    LOGINFO("Enter Mysql_User_Info_API check_db_conn.");
#ifdef USE_MYSQL
    if (0 == mysql_ping(&m_Connection))
    {
        return 0;
    }

    if (0 != connect_db(m_Info))
    {
        LOGWARN("Mysql_User_Info_API init connect_db failed!");
        return -1;
    }
#endif
    return 0;
}

