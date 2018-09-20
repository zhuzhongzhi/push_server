
#include "zlogimpl.h"
#include "zlogoutput.h"
#include "ztracelog.h"
#include "zutil.h"

const int           MAX_LOG_LEN                 = 1024;        
const int           MAX_DEBUG_LEN               = 1024 * 4; 
const int           MAX_BIN_LEN                 = 2048; 
const int           MAX_BIN_TEXT_LEN            = 1024;  

const int           BIN_LINE_NUM                = 16;


#define FORMAT_LOG(szBuff, buffLen, logLevel, pszFormat)                                    \
            do                                                                              \
            {                                                                               \
                char* pszBuff = szBuff;                                                     \
                pthread_t theadId = pthread_self();                                         \
                                                                                            \
                int iWriteLen = zsnprintf(pszBuff,                                          \
                                         sizeof(szBuff),                                    \
                                         "[0520-05-20 05:20:00.520][%lu][%s]",              \
                                         theadId,                                           \
                                         szLogLevel[logLevel]);                             \
                                                                                            \
                va_list argList;                                                            \
                va_start(argList, pszFormat);                                               \
                pszBuff = szBuff +  iWriteLen;                                              \
                buffLen = zvsnprintf(pszBuff,                                               \
                                    sizeof(szBuff) - iWriteLen -1,                          \
                                    pszFormat,                                              \
                                    argList);                                               \
                                                                                            \
                buffLen += iWriteLen;                                                       \
                szBuff[buffLen++] = '\n';                                                   \
                szBuff[buffLen] = '\0';                                                     \
                va_end(argList);                                                            \
            }while(0)                                                                       



CLogImpl*  CLogImpl::m_pInstance        = NULL;

CLogImpl*  CLogImpl::getInstance()
{
    if (NULL == m_pInstance)
    {
        ZNEW(m_pInstance, CLogImpl);
    }

    return m_pInstance;   
}


CLogImpl::CLogImpl()
{
    m_pLog                      = NULL;
    m_pDebug                    = NULL;
    m_bInit                     = false;
}

CLogImpl::~CLogImpl()
{
    ZDELETE(m_pLog);
    ZDELETE(m_pDebug);
}

bool CLogImpl::init(const TLogInfo& logInfo)
{
    if (m_bInit)
    {
        LOGINFO_S("The Log Sever has been initialized by other people!");
        return true;
    }

    do 
    {
        TLogInfo tmpInfo = logInfo;
        ZNEW_T(m_pLog, CLogOutput, CLogFile);
        tmpInfo.m_bTimeDetail = false;
        if (!m_pLog->init(tmpInfo, "log"))
        {
            return false;
        }
        
        ZNEW_T(m_pDebug, CLogOutput, CLogFile);
        tmpInfo.m_bTimeDetail = true;
        if (!m_pDebug->init(tmpInfo, "debug"))
        {
            return false;
        }

        m_bInit = true;
        
        return true;

    }while(0);

    ZDELETE(m_pLog);
    ZDELETE(m_pDebug);

    return false;
    
}

void  CLogImpl::doLogWarn(const char* pszFormat, ...)
{   
    SAFE_STATIC char szLogBuff[MAX_LOG_LEN + 1];
    szLogBuff[0] = '\0';
    int iBuffLen = 0;
    FORMAT_LOG(szLogBuff, iBuffLen, WARN_LEVEL, pszFormat);

    if (!m_bInit)
    {
        printf("%s", szLogBuff);
        return;
    }

    if (NULL != m_pLog)
    {
        m_pLog->output(szLogBuff, iBuffLen);
    }

    if ((m_eDebugLevel > OFF_LEVEL) && (NULL != m_pDebug))
    {
        m_pDebug->output(szLogBuff, iBuffLen);
    }

    return;
}

void  CLogImpl::doLogInfo(const char* pszFormat, ...)
{
    SAFE_STATIC char szLogBuff[MAX_LOG_LEN + 1];
    szLogBuff[0] = '\0';
    int iBuffLen = 0;
    FORMAT_LOG(szLogBuff, iBuffLen, INFO_LEVEL, pszFormat);

    if (!m_bInit)
    {
        printf("%s", szLogBuff);
        return;
    }

    if (NULL != m_pLog)
    {
        m_pLog->output(szLogBuff, iBuffLen);
    }

    if ((m_eDebugLevel > OFF_LEVEL) && (NULL != m_pDebug))
    {
        m_pDebug->output(szLogBuff, iBuffLen);
    }
    return;
}

void  CLogImpl::doDebugLog(const char* pszFormat, ...)
{
    SAFE_STATIC char szDebugBuff[MAX_DEBUG_LEN + 1];
    szDebugBuff[0] = '\0';
    int iBuffLen = 0;
    FORMAT_LOG(szDebugBuff, iBuffLen, DEBUG_LEVEL, pszFormat);
    if (!m_bInit)
    {
        printf("%s", szDebugBuff);
        return;
    }

    if (NULL != m_pDebug)
    {
        m_pDebug->output(szDebugBuff, iBuffLen);
    }

    return;
}

void  CLogImpl::doDebugBin(const char* pszBin, int iBinLen, const char* pszData, ...)
{
    SAFE_STATIC char szBinBuff[MAX_DEBUG_LEN * 5 + MAX_BIN_TEXT_LEN + 1];
    szBinBuff[0] = '\0';
    int iDataLen = 0;
    FORMAT_LOG(szBinBuff, iDataLen, DEBUG_LEVEL, pszData);

    if (iDataLen > MAX_BIN_TEXT_LEN)
    {
        szBinBuff[MAX_BIN_TEXT_LEN - 1] = '\n';
        szBinBuff[MAX_BIN_TEXT_LEN] = '\0';
        iDataLen = MAX_BIN_TEXT_LEN;
    }

    char* pszOuput = szBinBuff + iDataLen;    
    iDataLen += formatBin(pszBin, ZMin(iBinLen, MAX_DEBUG_LEN), pszOuput, sizeof(szBinBuff) - iDataLen);

    if (!m_bInit)
    {
        printf("%s", szBinBuff);
        return;
    }
    if (NULL != m_pDebug)
    {
        m_pDebug->output(szBinBuff, iDataLen);
    }

    return; 
}

int  CLogImpl::formatBin(const char* pszBin, int iBinLen, char*& pszOutput, const int iBuffLen)
{
    if ((NULL == pszBin) || (0 >= iBinLen))
    {
        return 0;
    }

    char* pszTmp = NULL;
    int iDataLen = 0;
    
#define SAFE_APPEND_BUFF(data)                                              \
            do                                                              \
            {                                                               \
                if ((int)sizeof(data) < (iBuffLen - iDataLen))                     \
                {                                                           \
                    memcpy(pszOutput + iDataLen, data, sizeof(data));       \
                    iDataLen += (sizeof(data) - 1);                         \
                }                                                           \
            }while(0)

    
    int i = 0;

    while (i < iBinLen)
    {
        if (0 == i % BIN_LINE_NUM)
        {
            pszTmp = pszOutput + iDataLen;
            iDataLen += zsnprintf(pszTmp, ZMax(iBuffLen - iDataLen, 0), "%04d    ", i);
        }

        int j = 0;
        int k = 0;
        for (j = 0, k = i + j; j < BIN_LINE_NUM && k < iBinLen; j++, k++)
        {
            pszTmp = pszOutput + iDataLen;
            iDataLen += zsnprintf(pszTmp, ZMax(iBuffLen - iDataLen, 0), "%02X ", pszBin[k] & 0xFF);

        }

        for (; j < BIN_LINE_NUM; j++)
        {
            SAFE_APPEND_BUFF("   ");
        }

        SAFE_APPEND_BUFF("    ");
        
        for (j = 0, k = i + j; j < BIN_LINE_NUM && k < iBinLen; j++, k++)
        {
            unsigned char szTmp[2];
            unsigned char cTmp = (unsigned char)pszBin[k] & (unsigned char)0xFF;
            szTmp[0] = isprint(cTmp) ? cTmp : '.';
            szTmp[1] = '\0';
            SAFE_APPEND_BUFF(szTmp);

        }
        SAFE_APPEND_BUFF("\n");
        i += j;
    }

    return iDataLen;
}

void CLogImpl::setDebugLevel(ELogLevel debugLevel)
{
    m_eDebugLevel = (debugLevel == DEBUG_LEVEL) ? DEBUG_LEVEL : OFF_LEVEL;
}

