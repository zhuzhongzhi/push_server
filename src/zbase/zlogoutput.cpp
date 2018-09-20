
#include "zmutex.h"
#include "zlogoutput.h"
#include "zutil.h"

CLogOutput::CLogOutput()
{
    m_iLastDay                  = 0;
    ZNEW(m_pMutex, ThreadMutex);
}

CLogOutput::~CLogOutput()
{
    ZDELETE(m_pMutex);
}

bool CLogOutput::check_new_day()
{
    return false;
}

bool CLogOutput::init(const TLogInfo & logInfo, const char * /*szSuffix*/)
{
    m_logInfo = logInfo;
    m_logInfo.m_logPath.append("/log");
    m_logInfo.m_iLogFileSize *= (1024 * 1024);

    if (0 != access(m_logInfo.m_logPath.data(), F_OK))
    {
        create_dir(m_logInfo.m_logPath);
    }

    m_fileHeader.assignFmt("================================================================================\n"
                           "%s %s Module.\n"
                           "Copyright 2014 zhuzhongzhi. All Rights Reserved!\n"
                           "================================================================================\n",
                           m_logInfo.m_procVer.data(),
                           m_logInfo.m_procName.data());

    
    return true;
}

bool  CLogOutput::modify_log_time(char* & pszLog)
{
    struct timeval tv;
    struct tm curTm;

    if (m_logInfo.m_bTimeDetail)
    {
        gettimeofday(&tv, NULL);
    }
    else
    {
        tv.tv_sec = time(NULL);
        tv.tv_usec = 0L;
    }
    
    localtime_r(&(tv.tv_sec), &curTm);

    bool bNewDay = (m_iLastDay != curTm.tm_yday);

    m_iLastDay  = curTm.tm_yday;
    int iLen = sprintf(pszLog + 1, 
                       "%04d-%02d-%02d %02d:%02d:%02d.%03d",
                       curTm.tm_year + BASE_YEAR, 
                       curTm.tm_mon + BASE_MONTH, 
                       curTm.tm_mday,
                       curTm.tm_hour, 
                       curTm.tm_min, 
                       curTm.tm_sec,
                       (int)(tv.tv_usec / 1000));

    pszLog[iLen + 1] = ']';

    return bNewDay;
}



CLogFile::CLogFile()
{
    m_iCurSeqNo                 = 0;
    m_logFd                     = -1;   
}


CLogFile::~CLogFile()
{
    CLOSE_FD(m_logFd);
}

bool CLogFile::init(const TLogInfo & logInfo, const char * szSuffix)
{
    if (!CLogOutput::init(logInfo, szSuffix))
    {
        return false;
    }

    m_seqFileName.append(m_logInfo.m_logPath);
    m_seqFileName.append("/.~");
    m_seqFileName.append(m_logInfo.m_procName.data(), m_logInfo.m_procName.len());
    m_seqFileName.append('.');
    m_seqFileName.append(szSuffix);
    m_seqFileName.append(".seq");

    m_tmpLogFileName.append(m_logInfo.m_procName.data(), m_logInfo.m_procName.len());
    m_tmpLogFileName.append('.');
    m_tmpLogFileName.append(szSuffix);

    if (!read_seq())
    {
        return false;
    }

    return true;
    
}


bool CLogFile::read_seq()
{
    int iSeqFd = open(m_seqFileName.data(), O_APPEND | O_CREAT | O_RDWR, S_IRUSR |S_IWUSR | S_IRGRP);

    if (-1 == iSeqFd)
    {
        printf("Failed to open %s, ErrMsg=%d:%s\n", m_seqFileName.data(), errno, strerror(errno));
        return false;
    }

    char szBuff[MIN_BUFF_LEN + 1] = {0};
    ssize_t iNum = ::read(iSeqFd, szBuff, MIN_BUFF_LEN);
    CLOSE_FD(iSeqFd);

    if (0 < iNum)
    {
        m_iCurSeqNo = atoi(szBuff);
    }
    else if (!save_seq())
    {
        return false;
    }

    update_file_name();    
    return true;    
}


bool CLogFile::save_seq()
{
    int iSeqFd = open(m_seqFileName.data(), O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR|S_IRGRP);
    if (-1 == iSeqFd)
    {
        return false;
    }

    TMinBuff seqBuff;
    seqBuff.assignFmt("%d", m_iCurSeqNo);
    write(iSeqFd, seqBuff.data(), seqBuff.len());
    
    CLOSE_FD(iSeqFd);
    
    return true;
}


void CLogFile::output(char* pszLog, const int iLogLen)
{
    if (m_logInfo.m_bIsWriteNeedLock)
    {
        m_pMutex->lock();
    }
    bool bWriteHead = false;
    //bWriteHead = modifyLogTime(pszLog);

    check_file(bWriteHead);

    if (-1 != m_logFd)
    {
        ::write(m_logFd, pszLog, iLogLen);
    }

    if (m_logInfo.m_bIsWriteNeedLock)
    {
        m_pMutex->unlock();
    }  
}

bool CLogFile::check_file(bool& bWriteHead)
{
    struct stat buf;
    memset((void *) &buf, 0, sizeof(buf));

    if (0 == stat(m_curLogFileName.data(), &buf))
    {
        if (buf.st_size > m_logInfo.m_iLogFileSize)
        {
            CLOSE_FD(m_logFd);

            remove(m_bakLogFileName.data());

            rename(m_curLogFileName.data(), m_bakLogFileName.data());

            if (++m_iCurSeqNo >= m_logInfo.m_iLogFileNum)
            {
                m_iCurSeqNo = 0;
            }

            update_file_name();

            remove(m_curLogFileName.data());

            save_seq();
        }
    }
    else
    {
        CLOSE_FD(m_logFd);

        if (0 != access(m_logInfo.m_logPath.data(), F_OK))
        {
            create_dir(m_logInfo.m_logPath);
        }
    }

    if (-1 == m_logFd)
    {
        m_logFd = open(m_curLogFileName.data(), O_APPEND | O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP);
        if (-1 == m_logFd)
        {
            printf("Failed to open %s, ErrMsg=%d:%s\n", m_curLogFileName.data(), errno, strerror(errno));
            return false;
        }

        ::write(m_logFd, m_fileHeader.data(), m_fileHeader.len());

        bWriteHead = false;
    }

    return true;
    
}

void CLogFile::update_file_name()
{
    
    m_curLogFileName.assignFmt("%s/%s.%d", 
                                m_logInfo.m_logPath.data(),
                                m_tmpLogFileName.data(),
                                m_iCurSeqNo);

    m_bakLogFileName.assignFmt("%s/.%s.%d", 
                                m_logInfo.m_logPath.data(),
                                m_tmpLogFileName.data(),
                                m_iCurSeqNo);   
}

void CLogFile::set_name_version(const char* proc_name, const char* version)
{
    m_logInfo.m_procName = proc_name;
    m_logInfo.m_procVer = version;
    m_fileHeader.clear();
    m_fileHeader.assignFmt("================================================================================\n"
                           "%s %s Module.\n"
                           "Copyright 2014 zhuzhongzhi. All Rights Reserved!\n"
                           "================================================================================\n",
                           m_logInfo.m_procVer.data(),
                           m_logInfo.m_procName.data());

    ::write(m_logFd, m_fileHeader.data(), m_fileHeader.len());
}

