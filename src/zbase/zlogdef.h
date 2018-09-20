
#ifndef _X_LOG_DEF_H_
#define _X_LOG_DEF_H_

#include "zbasedef.h"

//define log level
enum ELogLevel
{
    WARN_LEVEL          = 0 ,               
    INFO_LEVEL              ,               
    OFF_LEVEL               ,   
    DEBUG_LEVEL             ,  
    LEVEL_NUM                 
};

const char* const szLogLevel[LEVEL_NUM] = {" WARN",
                                           " INFO",
                                           "  OFF",
                                           "DEBUG"};

const int       DEFAULT_LOGFILE_SIZE                    = 10;           
const int       DEFAULT_LOGFILE_NUM                     = 10;           



struct TLogInfo
{
public:
    TLogInfo()
    {
        m_iLogFileSize  = DEFAULT_LOGFILE_SIZE;
        m_iLogFileNum   = DEFAULT_LOGFILE_NUM;
        m_bTimeDetail   = false;
        m_bIsWriteNeedLock = true;
    };

    TPath                               m_logPath;              
    TProcName                           m_procName;             
    TProcVer                            m_procVer;              
    int                                 m_iLogFileSize;         
    int                                 m_iLogFileNum;          
    bool                                m_bTimeDetail;          
    bool                                m_bIsWriteNeedLock;
};

#endif
