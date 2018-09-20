
#ifndef _X_LOG_IMPL_H
#define _X_LOG_IMPL_H

#include "zlogdef.h"
#include "zbasedef.h"

class CLogOutput;

class CLogImpl
{
public:
    CLogImpl();
    ~CLogImpl();
    bool                    init(const TLogInfo& logInfo);
    static CLogImpl*        getInstance();
    void                    doLogWarn(const char* pszFormat, ...)FORMAT_CHECK(printf, 2, 3);
    void                    doLogInfo(const char* pszFormat, ...)FORMAT_CHECK(printf, 2, 3);
    void                    doDebugLog(const char* pszFormat, ...)FORMAT_CHECK(printf, 2, 3);
    void                    doDebugBin(const char* pszBin, int iBinLen, const char* pszData, ...)FORMAT_CHECK(printf, 4, 5);
    INLINE ELogLevel        getDebugLevel(){return m_eDebugLevel;};
    void                    setDebugLevel(ELogLevel debugLevel);

private:
    DEF_COPY_AND_ASSIGN(CLogImpl);

    int                     formatBin(const char* pszBin, int iBinLen, char* & pszOutput, const int iBuffLen);

private:

    static CLogImpl*                            m_pInstance;   
    bool                                        m_bInit;
    CLogOutput*                                 m_pLog;
    CLogOutput*                                 m_pDebug;
    ELogLevel                                   m_eDebugLevel;   
};

#endif
