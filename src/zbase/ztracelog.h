
#ifndef _X_TRACELOG_H_
#define _X_TRACELOG_H_

#include "zlogimpl.h"


#define INITLOG_S(szWorkDir, szProcName, szVersion, failOper)                             \
            do                                                                          \
            {                                                                           \
                TLogInfo info;                                                   \
                info.m_logPath      = szWorkDir;                                        \
                info.m_procName     = szProcName;                                       \
                info.m_procVer      = szVersion;                                        \
                                                                                        \
                if (!CLogImpl::getInstance()->init(info))                        \
                {                                                                       \
                    failOper;                                                           \
                }                                                                       \
            }while(0)

#define LOGWARN_S CLogImpl::getInstance()->doLogWarn

#define LOGINFO_S CLogImpl::getInstance()->doLogInfo

#define DEBUGLOG_S if (CLogImpl::getInstance()->getDebugLevel() >= DEBUG_LEVEL)     \
                        CLogImpl::getInstance()->doDebugLog

#define DEBUGBIN_S if (CLogImpl::getInstance()->getDebugLevel() >= DEBUG_LEVEL)     \
                        CLogImpl::getInstance()->doDebugBin


#define SET_DEBUG_LEVEL_S  CLogImpl::getInstance()->setDebugLevel 

#define LOGSYSWARN_S(fmt, ...)  LOGWARN_S(fmt" [ErrMsg=%d:%s]", ##__VA_ARGS__, errno, strerror(errno))
#endif

