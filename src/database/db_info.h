#ifndef __DB_INFO__H__
#define __DB_INFO__H__

#include "zbasedef.h"

enum DBType
{
    TYPE_MONGODB = 0,
    TYPE_MYSQL
};

class DBInfo
{
public:
    DBInfo(){};
    virtual ~DBInfo(){};

    DBInfo& operator=(const DBInfo& r)
    {
        m_IP = r.m_IP;
        m_Port = r.m_Port;
        m_DBName = r.m_DBName;
        m_UserName = r.m_UserName;
        m_Password = r.m_Password;
        m_Charset = r.m_Charset;
        return *this;
    }
public:
    STLString         m_IP;  
    unsigned short    m_Port;  
    STLString         m_DBName;
    STLString         m_UserName;
    STLString         m_Password;
    STLString         m_Charset;
};

#endif
