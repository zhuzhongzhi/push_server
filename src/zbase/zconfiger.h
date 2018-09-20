#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include "zbasedef.h"
#include "zconfig.h"
#include "zlogger.h"

#define ITEM_NAME(SEGMENT, ITEM)  m_##SEGMENT_##ITEM
#define FUNC_NAME(SEGMENT, ITEM)  get_##SEGMENT_##ITEM()

#define DEF_STRING_ITEM(SEGMENT, ITEM)  \
private:\
    STLString ITEM_NAME(SEGMENT, ITEM);\
public:\
    inline const STLString& FUNC_NAME(SEGMENT, ITEM)\
    {\
        return ITEM_NAME(SEGMENT, ITEM);\
    }

#define DEF_ARRAY_ITEM(SEGMENT, ITEM, SIZE)  \
private:\
    char ITEM_NAME(SEGMENT, ITEM)[SIZE + 1];\
public:\
    inline const char* FUNC_NAME(SEGMENT, ITEM)\
    {\
        return ITEM_NAME(SEGMENT, ITEM);\
    }

#define DEF_INT_ITEM(SEGMENT, ITEM)  \
private:\
    int ITEM_NAME(SEGMENT, ITEM);\
public:\
    inline int FUNC_NAME(SEGMENT, ITEM)\
    {\
        return ITEM_NAME(SEGMENT, ITEM);\
    }

#define DEF_UINT_ITEM(SEGMENT, ITEM)  \
private:\
    unsigned int ITEM_NAME(SEGMENT, ITEM);\
public:\
    inline unsigned int FUNC_NAME(SEGMENT, ITEM)\
    {\
        return ITEM_NAME(SEGMENT, ITEM);\
    }


#define READ_STRING_ITEM(SEGMENT, ITEM) \
    errCode = m_Config.read_string(#SEGMENT, #ITEM, ITEM_NAME(SEGMENT, ITEM));\
    if (CFG_OK != errCode)\
    {\
        LOGWARN("Read Segment[%s] Item[%s] [%s], result[%d]", RED_TEXT(SEGMENT), RED_TEXT(ITEM), RED_TEXT(FAILED), errCode);\
        return -1;\
    }\
    LOGINFO("Read Segment[%s] Item[%s] Value[%s].", GREEN_TEXT(SEGMENT), GREEN_TEXT(ITEM), ITEM_NAME(SEGMENT, ITEM).c_str());\
    
#define READ_ARRAY_ITEM(SEGMENT, ITEM) \
    errCode = m_Config.read_string(#SEGMENT, #ITEM, ITEM_NAME(SEGMENT, ITEM), sizeof(m_##ITEM));\
    if (CFG_OK != errCode)\
    {\
        LOGWARN("Read Segment[%s] Item[%s] [%s], result[%d]", RED_TEXT(SEGMENT), RED_TEXT(ITEM), RED_TEXT(FAILED), errCode);\
        return -1;\
    }\
    LOGINFO("Read Segment[%s] Item[%s] Value[%s].", GREEN_TEXT(SEGMENT), GREEN_TEXT(ITEM), ITEM_NAME(SEGMENT, ITEM));\

#define READ_INT_ITEM(SEGMENT, ITEM) \
    errCode = m_Config.read_int(#SEGMENT, #ITEM, ITEM_NAME(SEGMENT, ITEM));\
    if (CFG_OK != errCode)\
    {\
        LOGWARN("Read Segment[%s] Item[%s] [%s], result[%d]", RED_TEXT(SEGMENT), RED_TEXT(ITEM), RED_TEXT(FAILED), errCode);\
        return -1;\
    }\
    LOGINFO("Read Segment[%s] Item[%s] Value[%d].", GREEN_TEXT(SEGMENT), GREEN_TEXT(ITEM), ITEM_NAME(SEGMENT, ITEM));\

#define READ_UINT_ITEM(SEGMENT, ITEM) \
        errCode = m_Config.read_uint(#SEGMENT, #ITEM, ITEM_NAME(SEGMENT, ITEM));\
        if (CFG_OK != errCode)\
        {\
            LOGWARN("Read Segment[%s] Item[%s] [%s], result[%d]", RED_TEXT(SEGMENT), RED_TEXT(ITEM), RED_TEXT(FAILED), errCode);\
            return -1;\
        }\
        LOGINFO("Read Segment[%s] Item[%s] Value[%u].", GREEN_TEXT(SEGMENT), GREEN_TEXT(ITEM), ITEM_NAME(SEGMENT, ITEM);\

    
class Configer
{
public:
	Configer();
    virtual ~Configer();

public:
    int init(const char* work_dir);
    static Configer *instance();
    
// add item here
DEF_STRING_ITEM(Common, LocalIp);
DEF_INT_ITEM(Common, LocalPort);
DEF_STRING_ITEM(Common, ProcName);
DEF_STRING_ITEM(Common, Version);

DEF_INT_ITEM(Service, ThreadNum);
DEF_INT_ITEM(Service, DBThreadNum);
DEF_INT_ITEM(Service, AuthUser);
DEF_INT_ITEM(Service, MultLogin);
DEF_INT_ITEM(Service, ResendQos);

DEF_INT_ITEM(DB, Type);
DEF_STRING_ITEM(DB, DBIPAddress);
DEF_INT_ITEM(DB, DBPort);
DEF_STRING_ITEM(DB, DBName);
DEF_STRING_ITEM(DB, DBUser);
DEF_STRING_ITEM(DB, DBPassword);
DEF_STRING_ITEM(DB, DBCharset);

DEF_INT_ITEM(APNS, APNSThreadNum);
DEF_STRING_ITEM(APNS, Host);
DEF_STRING_ITEM(APNS, CertFile);
DEF_STRING_ITEM(APNS, MsgContent);

DEF_INT_ITEM(SESSION, AuthTimeOut);
DEF_INT_ITEM(SESSION, GroupTimeOut);
DEF_INT_ITEM(SESSION, GroupOutOfServiceTime);

DEF_STRING_ITEM(AUTH, PublishKey);
DEF_INT_ITEM(AUTH, TermOfValidty);


private:
    int  read_item()
    {
        ECfgErrCode errCode;

        READ_STRING_ITEM(Common, LocalIp);
        READ_INT_ITEM(Common, LocalPort);
        READ_STRING_ITEM(Common, ProcName);
        READ_STRING_ITEM(Common, Version);

        READ_INT_ITEM(Service, ThreadNum);
        READ_INT_ITEM(Service, DBThreadNum);
        READ_INT_ITEM(Service, AuthUser);
        READ_INT_ITEM(Service, MultLogin);
        READ_INT_ITEM(Service, ResendQos);

        READ_INT_ITEM(DB, Type);
        READ_STRING_ITEM(DB, DBIPAddress);
        READ_INT_ITEM(DB, DBPort);
        READ_STRING_ITEM(DB, DBName);
        READ_STRING_ITEM(DB, DBUser);
        READ_STRING_ITEM(DB, DBPassword);
        READ_STRING_ITEM(DB, DBCharset);

        READ_INT_ITEM(APNS, APNSThreadNum);
        READ_STRING_ITEM(APNS, Host);
        READ_STRING_ITEM(APNS, CertFile);
        READ_STRING_ITEM(APNS, MsgContent);

        READ_INT_ITEM(SESSION, AuthTimeOut);
        READ_INT_ITEM(SESSION, GroupTimeOut);
        READ_INT_ITEM(SESSION, GroupOutOfServiceTime);

        READ_STRING_ITEM(AUTH, PublishKey);
        READ_INT_ITEM(AUTH, TermOfValidty);

        return 0;
    }
    
private:
    ZConfig           m_Config;
    static Configer  *m_Instance;
};

#define  GET_CFG_ITEM(SEGMENT, ITEM)  Configer::instance()->FUNC_NAME(SEGMENT, ITEM)

#endif
