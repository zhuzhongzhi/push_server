
#include "otlv4.h"
#include "odbc_auto_db_connect.h"

otl_connect* auto_db_connect::create_db_connect()
{
    otl_connect* db = NULL;
    try
    {
        db = new otl_connect();
    }
    catch(...)
    {
        LOGWARN("new otl_connect failed, not enough memory!");
        return NULL;
    }
    
    try
    {           
        std::string connect_str = _db_info.m_UserName + "/" + _db_info.m_Password + "@" + _db_info.m_DBName;
        db->rlogon(connect_str.c_str(),1);// connect to the database user/psw/dsn
    }
    catch (otl_exception& p)
    { 
        LOGWARN("db rlogon exception, msg[%s],sql[%s],var[%s]",
            p.msg, p.stm_text, p.var_info);
        
        return NULL;
    }

    return db;
}

void auto_db_connect::release_db_connect(otl_connect* db)
{
    if (NULL != db)
    {
        db->logoff();
        delete db;
    }
}


auto_db_connect::auto_db_connect(otl_connect* db, const DBInfo& db_info)
{
    _db_info = db_info;
    if (NULL == db)
    {
        _db_connect = create_db_connect();
        _need_release = true;
    }
    else
    {
        _db_connect = db;
        _need_release = false;
    }
}

auto_db_connect::~auto_db_connect()
{
    if (_need_release)
    {
        release_db_connect(_db_connect);
    }
    _db_connect = NULL;
}

otl_connect* auto_db_connect::get_connect()
{
    return _db_connect;
}

bool auto_db_connect::reconnect()
{
    release_db_connect(_db_connect);
    if (NULL == create_db_connect())
    {
        return false;
    }

    return true;
}

