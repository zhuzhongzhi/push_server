#include <sys/timeb.h>
#include "zutil.h"
#include "zbasedef.h"
#include "zlogoutput.h"
#include "zlogger.h"
#include "zmsgtype.h"

#define LOGGER_FORMAT(szBuff, MAXBufferSize, buffLen, logLevel, pszFormat)                                 \
    do                                                                              \
    {                                                                               \
        char* pszBuff = szBuff;                                                     \
        pthread_t theadId = pthread_self();                                         \
        struct timeb tb;                                                            \
        ftime(&tb);                                                                 \
        struct timeval tv;                                                          \
        tv.tv_sec = tb.time + tb.timezone;                                          \
        tv.tv_usec = tb.millitm * 1000;                                             \
        time_t now = tv.tv_sec;                                                     \
        struct tm now_tm;                                                           \
        localtime_r(&now, &now_tm);                                                 \
        int iWriteLen = zsnprintf(pszBuff,                                          \
                                 MAXBufferSize,                                     \
                                 "[%04d-%02d-%02d %02d:%02d:%02d.%03d][%lu][%s]",   \
                                 now_tm.tm_year + 1900,                             \
                                 now_tm.tm_mon + 1,                                 \
                                 now_tm.tm_mday,                                    \
                                 now_tm.tm_hour,                                    \
                                 now_tm.tm_min,                                     \
                                 now_tm.tm_sec,                                     \
                                 (int)(tv.tv_usec / 1000),                          \
                                 theadId,                                           \
                                 szLogLevel[logLevel]);                             \
                                                                                    \
        va_list argList;                                                            \
        va_start(argList, pszFormat);                                               \
        pszBuff = szBuff +  iWriteLen;                                              \
        buffLen = zvsnprintf(pszBuff,                                               \
                            MAXBufferSize - iWriteLen -1,                           \
                            pszFormat,                                              \
                            argList);                                               \
                                                                                    \
        buffLen += iWriteLen;                                                       \
        szBuff[buffLen++] = '\n';                                                   \
        szBuff[buffLen] = '\0';                                                     \
        va_end(argList);                                                            \
    }while(0)         
            

Logger *Logger::m_instance = NULL;
Logger *Logger::instance()
{
	if (NULL == m_instance)
	{
		ZNEW(m_instance, Logger);
	}

	return m_instance;
}

Logger::Logger()
{
    m_bInit = false;
    m_pLog = NULL;
    m_pDebug = NULL;
    m_eDebugLevel = DEBUG_LEVEL;
}

Logger::~Logger()
{
    ZDELETE(m_pLog);
    ZDELETE(m_pDebug);
}

int Logger::init(const TLogInfo& log_info)
{
    if (m_bInit)
    {
        LOGINFO("The Log Sever has been initialized by other people!");
        return 0;
    }
    
    {
        m_LogMutex.lock();
        LogMsg* msg= NULL;
        for (int i = 0; i < LOG_QUEUE_SIZE; ++i)
        {
            msg = new LogMsg(LOG_BUFFER_SIZE);
            m_LogBufferManager.push_back(msg);
        }
        m_LogMutex.unlock();
    }

    {
        m_BinMutex.lock();
        LogMsg* msg= NULL;
        for (int i = 0; i < BIN_QUEUE_SIZE; ++i)
        {
            msg = new LogMsg(BIN_BUFFER_SIZE);
            m_BinBufferManager.push_back(msg);
        }
        m_BinMutex.unlock();
	}

    TLogInfo tmpInfo = log_info;
	ZNEW_T(m_pLog, CLogOutput, CLogFile);
	tmpInfo.m_bTimeDetail = false;
	tmpInfo.m_bIsWriteNeedLock = false;
	if (!m_pLog->init(tmpInfo, "log"))
	{
        LOGWARN("log init failed!");
        return -1;
	}

	ZNEW_T(m_pDebug, CLogOutput, CLogFile);
	tmpInfo.m_bTimeDetail = true;
	tmpInfo.m_bIsWriteNeedLock = false;
	if (!m_pDebug->init(tmpInfo, "debug"))
	{
        LOGWARN("debug init failed!");
        return -1;
	}

	m_bInit = true;

	support(TYPE_LOGGER, std::bind(&Logger::on_logger, this, std::placeholders::_1));
	return 0;
}

void Logger::set_name_version(const char* proc_name, const char* version)
{
    m_pLog->set_name_version(proc_name, version);
    m_pDebug->set_name_version(proc_name, version);    
}

int Logger::on_logger(NLMessage*& msg)
{    
	LogMsg* log_msg = (LogMsg*)msg;
    if (!m_bInit)
    {
        printf("%s", log_msg->m_Buffer);
        return -1;
    }

    if ((NULL != m_pLog) && (log_msg->m_Level <= OFF_LEVEL))
    {
        m_pLog->output(log_msg->m_Buffer, log_msg->m_BufferSize);
    }

    if (NULL != m_pDebug)
    {
        m_pDebug->output(log_msg->m_Buffer, log_msg->m_BufferSize);
    }
    log_msg->clear();

    if (LOG_BUFFER_SIZE == log_msg->m_ArraySize)
    {
    	m_LogMutex.lock();
        m_LogBufferManager.push_back(log_msg);
        m_LogMutex.unlock();
    }
    else
    {
    	m_BinMutex.lock();
        m_BinBufferManager.push_back(log_msg);
        m_LogMutex.unlock();
    }
    log_msg = NULL;
    msg = NULL;
    return 0;
}

#define  DO_LOG_LEVEL(level) \
{ \
	if (m_bInit)\
	{\
        LogMsg * log_msg = NULL;\
        {\
            m_LogMutex.lock();\
            if (m_LogBufferManager.empty())\
            {\
                log_msg = NULL;\
            }\
            else\
            {\
                log_msg = m_LogBufferManager.front();\
                m_LogBufferManager.pop_front();\
            }\
            m_LogMutex.unlock();\
        }\
        if (NULL == log_msg)\
        {\
            printf("not enough log buffer!");\
            return;\
        }\
        char* szLogBuff = log_msg->m_Buffer;\
        szLogBuff[0] = '\0';\
        LOGGER_FORMAT(szLogBuff, LOG_BUFFER_SIZE, log_msg->m_BufferSize, level, format);\
        log_msg->m_Level = level;\
        if (0 != post_message(log_msg))\
        {\
            m_LogBufferManager.push_back(log_msg);\
            log_msg = NULL;\
        }\
        else\
        {\
            log_msg = NULL;\
        }\
	}\
	else\
	{\
        char szLogBuff[LOG_BUFFER_SIZE + 1];\
        szLogBuff[0] = '\0';\
        int iBuffLen = 0;\
        LOGGER_FORMAT(szLogBuff, LOG_BUFFER_SIZE, iBuffLen, level, format);\
        printf("%s", szLogBuff);\
	}\
	return;\
}

void Logger::doLogWarn(const char* format, ...)
{   
	DO_LOG_LEVEL(WARN_LEVEL);
}

void Logger::doLogInfo(const char* format, ...)
{
	DO_LOG_LEVEL(INFO_LEVEL);
}

void Logger::doDebugLog(const char* format, ...)
{    
	DO_LOG_LEVEL(DEBUG_LEVEL);
}

void Logger::doDebugBin(const char* bin, int bin_len, const char* data, ...)
{    
    if (m_bInit)
    {
    	LogMsg * log_msg = NULL;
		{
			m_BinMutex.lock();
			if (m_BinBufferManager.empty())
			{
				log_msg = NULL;
			}
			else
			{
				log_msg = m_BinBufferManager.front();
				m_BinBufferManager.pop_front();
			}
            m_BinMutex.unlock();
		}
        if (NULL == log_msg)
        {
            return;
        }
        char* szBinBuff = log_msg->m_Buffer;
        szBinBuff[0] = '\0';    
        int iDataLen = 0;

        LOGGER_FORMAT(szBinBuff, BIN_BUFFER_SIZE, log_msg->m_BufferSize, DEBUG_LEVEL, data);

        if (iDataLen > LOG_BUFFER_SIZE)
        {
            szBinBuff[LOG_BUFFER_SIZE - 1] = '\n';
            szBinBuff[LOG_BUFFER_SIZE] = '\0';
            iDataLen = LOG_BUFFER_SIZE;
        }
        char* pszOuput = szBinBuff + log_msg->m_BufferSize;
        
        iDataLen += formatBin(bin, ZMin(bin_len, BIN_BUFFER_SIZE - log_msg->m_BufferSize), pszOuput, BIN_BUFFER_SIZE - log_msg->m_BufferSize);
        log_msg->m_BufferSize += iDataLen;
        
        log_msg->m_Level = DEBUG_LEVEL;
        if (0 != post_message(log_msg))
        {
        	m_BinBufferManager.push_back(log_msg);
        	log_msg = NULL;
        }
        else
        {
        	log_msg = NULL;
        }
    }
    else
    {
     	char szBinBuff[BIN_BUFFER_SIZE];
		szBinBuff[0] = '\0';
		int iDataLen = 0;
		int buffser_size = 0;
		LOGGER_FORMAT(szBinBuff, BIN_BUFFER_SIZE, buffser_size, DEBUG_LEVEL, data);

		if (iDataLen > LOG_BUFFER_SIZE)
		{
			szBinBuff[LOG_BUFFER_SIZE - 1] = '\n';
			szBinBuff[LOG_BUFFER_SIZE] = '\0';
			iDataLen = LOG_BUFFER_SIZE;
		}

		char* pszOuput = szBinBuff + buffser_size;

		iDataLen += formatBin(bin, ZMin(bin_len, BIN_BUFFER_SIZE - buffser_size), pszOuput, BIN_BUFFER_SIZE - buffser_size);
		buffser_size += iDataLen;
		printf("%s", szBinBuff);
    }
    return; 
}

int Logger::formatBin(const char* bin, int bin_len, char*& output, const int buff_len)
{
    if ((NULL == bin) || (0 >= bin_len))
    {
        return 0;
    }

    char* pszTmp = NULL;
    int iDataLen = 0;
    
#define SAFE_APPEND_BUFF(data)                                              \
            do                                                              \
            {                                                               \
                if ((int)sizeof(data) < (buff_len - iDataLen))              \
                {                                                           \
                    memcpy(output + iDataLen, data, sizeof(data));       \
                    iDataLen += (sizeof(data) - 1);                         \
                }                                                           \
            }while(0)

#define BIN_LINE_NUMBER 16
                
    int i = 0;

    while (i < bin_len)
    {
        if (0 == i % BIN_LINE_NUMBER)
        {
            pszTmp = output + iDataLen;
            iDataLen += zsnprintf(pszTmp, ZMax(buff_len - iDataLen, 0), "%04d    ", i);
        }

        int j = 0;
        int k = 0;
        for (j = 0, k = i + j; j < BIN_LINE_NUMBER && k < bin_len; j++, k++)
        {
            pszTmp = output + iDataLen;
            iDataLen += zsnprintf(pszTmp, ZMax(buff_len - iDataLen, 0), "%02X ", bin[k] & 0xFF);

        }
        for (; j < BIN_LINE_NUMBER; j++)
        {
            SAFE_APPEND_BUFF("   ");
        }

        SAFE_APPEND_BUFF("    ");
        
        for (j = 0, k = i + j; j < BIN_LINE_NUMBER && k < bin_len; j++, k++)
        {
            unsigned char szTmp[2];
            unsigned char cTmp = (unsigned char)bin[k] & (unsigned char)0xFF;
            szTmp[0] = isprint(cTmp) ? cTmp : '.';
            szTmp[1] = '\0';
            SAFE_APPEND_BUFF(szTmp);

        }
        SAFE_APPEND_BUFF("\n");
        i += j;
    }

    return iDataLen;
}

