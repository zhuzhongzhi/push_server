
#ifndef _X_STRING_H_
#define _X_STRING_H_

#include <string.h>
#include <stdarg.h>

template<int CAPACITY> class XString
{
public:
    XString();
    XString(const char* pszData);
    ~XString();
    XString(const XString& other);
    XString&            operator=(const XString& other);
    inline void         clear();
    inline XString&     append(const char* pszData);
    XString&            append(const char* pszData, int iLen);
    inline XString&     append(const XString& other);
    inline XString&     append(const char ch);
    XString&            appendFmt(const char* pszFormat, ...);
    XString&            append(const char* pszFormat, va_list& arglist);
    inline XString&     assign(const char* pszData);
    inline XString&     assign(const char* pszData, int iLen);
    inline XString&     assign(const XString& other);
    XString&            assignFmt(const char* pszFormat, ...);

    const char*         data()               const      {return m_szData;};
    const int           len()                const       {return m_iLen;};
    const int           capacity()          const       {return CAPACITY;};
    const int           dataSize()          const       {return sizeof(m_szData);};        

private:
    char                m_szData[CAPACITY + 1];
    int                 m_iLen;
    
};


template<int CAPACITY> 
XString<CAPACITY>::XString()
{
    this->clear();
}


template<int CAPACITY>
XString<CAPACITY>::XString(const char * pszData)
{
    this->assign(pszData);
}

template<int CAPACITY>
XString<CAPACITY>::XString(const XString & other)
{
    this->assign(other);
}

template<int CAPACITY>
XString<CAPACITY>::~XString()
{
    this->clear();
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::operator=(const XString& other)
{
    if (this == &other)
    {
        return *this;
    }
    return this->assign(other);
}


template<int CAPACITY>
void XString<CAPACITY>::clear()
{
    this->m_szData[0]           = '\0';
    this->m_iLen                = 0;
}


template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::assign(const char * pszData)
{
    this->clear();
    return this->append(pszData);
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::assign(const XString & other)
{
    this->clear();
    return this->append(other);
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::assign(const char * pszData, int iLen)
{
    this->clear();
    return this->append(pszData, iLen);
}

template<int CAPACITY> 
XString<CAPACITY>& XString<CAPACITY>::append(const char * pszData)
{
    if (NULL != pszData)
    {
        this->append(pszData, strlen(pszData));
    }
    return *this;
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::append(const XString & other)
{
    return this->append(other.data(), other.len());
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::append(const char ch)
{
    return this->append(&ch, 1);
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::append(const char * pszData, int iLen)
{
    if ((NULL != pszData) && (0 < iLen) && (pszData != m_szData))
    {
        int iCopyLen = (iLen < CAPACITY - iLen) ? iLen : (CAPACITY - iLen);
        if (0 < iCopyLen)
        {
            memcpy(m_szData + m_iLen, pszData, iCopyLen);
            m_iLen += iCopyLen;
            m_szData[m_iLen] = '\0';
        }
    }

    return *this;
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::appendFmt(const char * pszFormat, ...)
{
    va_list argList;
    va_start(argList, pszFormat);
    this->append(pszFormat, argList);
    va_end(argList);
    return *this;   
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::append(const char * pszFormat, va_list & arglist)
{
    if (m_iLen < CAPACITY)
    {
        m_iLen += vsnprintf(m_szData + m_iLen, this->dataSize() - m_iLen, pszFormat, arglist);
        if (m_iLen >= dataSize())
        {
            m_iLen = CAPACITY;
        }
        m_szData[m_iLen] = '\0';
    }
    return *this;
}

template<int CAPACITY>
XString<CAPACITY>& XString<CAPACITY>::assignFmt(const char * pszFormat, ...)
{
    this->clear();
    va_list argList;
    va_start(argList, pszFormat);
    this->append(pszFormat, argList);
    va_end(argList);
    return *this;
}

#endif

