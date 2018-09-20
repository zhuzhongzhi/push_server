
#ifndef _X_LOG_OUTPUT_H_
#define _X_LOG_OUTPUT_H_

#include "zlogdef.h"

class ThreadMutex;
class CLogOutput
{
public:
    CLogOutput();
    virtual ~CLogOutput();
    virtual bool            init(const TLogInfo& logInfo, const char* szSuffix);
    virtual void            output(char* pszLog, const int iLogLen) = 0;
    virtual void set_name_version(const char* proc_name, const char* version) = 0;
protected:
    bool                    check_new_day();
    bool                    modify_log_time(char* & pszLog);

private:
    DEF_COPY_AND_ASSIGN(CLogOutput);

protected:
    ThreadMutex*            m_pMutex;
    TLogInfo                m_logInfo;
    THugBuff                m_fileHeader;
    int                     m_iLastDay;
};


class CLogFile : public CLogOutput
{
public:
    CLogFile();
    virtual ~CLogFile();
    virtual bool            init(const TLogInfo& logInfo, const char* szSuffix);
    virtual void            output(char* pszLog, const int iLogLen);

private:
    DEF_COPY_AND_ASSIGN(CLogFile);
    bool                    check_file(bool& bWriteHead);
    bool                    read_seq();
    bool                    save_seq();
    void                    update_file_name();   
    virtual void set_name_version(const char* proc_name, const char* version);

private: 
    TFileName                       m_tmpLogFileName;
    TFileName                       m_curLogFileName;
    TFileName                       m_bakLogFileName;
    TFileName                       m_seqFileName;
    int                             m_iCurSeqNo;
    int                             m_logFd;
};

#endif
