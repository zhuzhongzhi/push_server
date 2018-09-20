#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <deque>
#include "zmutex.h"
#include "zlogdef.h"
#include "zbasedef.h"
#include "zlogoutput.h"
#include "zqueuethread.h"

#define INITLOG(szWorkDir, szProcName, szVersion, failOper)                             \
            do                                                                          \
            {                                                                           \
                TLogInfo info;                                                          \
                info.m_logPath      = szWorkDir;                                        \
                info.m_procName     = szProcName;                                       \
                info.m_procVer      = szVersion;                                        \
                                                                                        \
                if (!Logger::instance()->init(info))                                    \
                {                                                                       \
                    failOper;                                                           \
                }                                                                       \
            }while(0)

#define LOGWARN        Logger::instance()->doLogWarn

#define LOGSERVICE     LOGWARN

#define LOGINFO        if (Logger::instance()->getDebugLevel() >= INFO_LEVEL)      \
                       Logger::instance()->doLogInfo

#define DEBUGLOG       if (Logger::instance()->getDebugLevel() >= DEBUG_LEVEL)     \
                        Logger::instance()->doDebugLog

#define DEBUGBIN       if (Logger::instance()->getDebugLevel() >= DEBUG_LEVEL)     \
                        Logger::instance()->doDebugBin


#define SET_DEBUG_LEVEL  Logger::instance()->setDebugLevel 


class Logger : public QueueThread
{
public:
    enum
    {
        LOG_QUEUE_SIZE = 1024,
        BIN_QUEUE_SIZE = 256,
        LOG_BUFFER_SIZE = 1024,
        BIN_BUFFER_SIZE = 4096,
    };
public:
    Logger();
    virtual ~Logger();

    static Logger *instance();

private:
    static Logger *m_instance;

public:
    int init(const TLogInfo& log_info);

    int on_logger(NLMessage*& msg);
    
    void doLogWarn(const char* format, ...) FORMAT_CHECK(printf, 2, 3);

    void doLogInfo(const char* format, ...) FORMAT_CHECK(printf, 2, 3);

    void doDebugLog(const char* format, ...) FORMAT_CHECK(printf, 2, 3);

    void doDebugBin(const char* bin, int bin_len, const char* data, ...) FORMAT_CHECK(printf, 4, 5);

    int formatBin(const char* bin, int bin_len, char*& output, const int buff_len);

    inline ELogLevel getDebugLevel()
    {
        return m_eDebugLevel;
    };

    inline void setDebugLevel(ELogLevel debugLevel)
    {
        m_eDebugLevel = debugLevel;
    }

    void set_name_version(const char* proc_name, const char* version);
    
private:
    bool                m_bInit;            
    CLogOutput*         m_pLog;             
    CLogOutput*         m_pDebug;           
    ThreadMutex         m_LogMutex;
    std::deque<LogMsg*> m_LogBufferManager;
    ThreadMutex         m_BinMutex;
    std::deque<LogMsg*> m_BinBufferManager;
    ELogLevel           m_eDebugLevel;
};

#endif
