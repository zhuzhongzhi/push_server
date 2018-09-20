#ifndef __ODBC_AUTO_DB_CONNECT_H__
#define __ODBC_AUTO_DB_CONNECT_H__

#include <string>
#include "db_info.h"
class otl_connect;

class auto_db_connect
{
public:
    auto_db_connect(otl_connect* db, const DBInfo& db_info);
    virtual ~auto_db_connect();
    otl_connect* get_connect();
    bool reconnect();
    
private:    
    otl_connect* create_db_connect();
    void release_db_connect(otl_connect* db);
public:
    DBInfo      _db_info;
    otl_connect* _db_connect;
    bool _need_release;
};

#ifndef OTL_BIGINT
#define OTL_BIGINT  long long
#endif

#ifndef OTL_STR_TO_BIGINT
#define OTL_STR_TO_BIGINT(str,n) \
{ \
  n=atoll(str); \
}
#endif

#ifndef OTL_BIGINT_TO_STR
#define OTL_BIGINT_TO_STR(n,str) \
{ \
  sprintf(str,"%lld", n); \
}
#endif

#endif
