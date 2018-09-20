
#include "zutil.h"


char* safe_copy_str(char * dst, const char * src, int maxlen)
{
    if ((maxlen < 1) || (NULL == src))
    {
        return NULL;
    }
    
    if ((NULL != dst) && (dst != src))
    {
        int iCopyLen = 0;

        int iSrcLen = strlen(src);
        iCopyLen = (iSrcLen <= maxlen) ? iSrcLen : (maxlen - 1);
        memcpy(dst, src, iCopyLen);

        dst[iCopyLen] = '\0';
    }

    return dst;
}



bool create_dir(const TPath& path)
{
    TPath tmpPath;

    const char* pOffset = path.data();
    
    if (*pOffset != '/')
    {
        return false;
    }

    tmpPath.append('/');

    ++pOffset;
    
    while (NULL != pOffset)
    {
        const char* pTmp = pOffset;
        pOffset = strchr(pTmp, '/');

        if (NULL == pOffset)
        {
            tmpPath.append(pTmp, path.len() - (pTmp - path.data()));
        }
        else
        {
            tmpPath.append(pTmp, pOffset - pTmp);
            ++pOffset;
        }
        mkdir(tmpPath.data(), S_IRUSR | S_IWUSR | S_IXUSR);

        tmpPath.append('/');
    }

    return true;
}

bool create_dir(const char* pszDir)
{
    if (NULL == pszDir)
    {
        return false;
    }

    TPath path(pszDir);
    return create_dir(path);
}

int  zsnprintf(char*& str, size_t size, const char *format, ...)
{
    va_list argList;
    va_start(argList, format);
    int iRetSize = zvsnprintf(str, size, format, argList);
    va_end(argList);
    return iRetSize;
}

int  zvsnprintf(char*& str, size_t size, const char *format, va_list& ap)
{
    if (0 == size)
    {
        return 0;
    }

    int iRetSize = vsnprintf(str, size, format, ap);

    if (iRetSize >= (int)size)
    {
        iRetSize = size - 1;
        str[iRetSize] = '\0';
    }

    return iRetSize;
}

void trim_space(STLString& str)
{
    if (str.empty())
    {
        return;
    }

    STLString::size_type iStartPos = 0;
    STLString::size_type iEndPos   = str.length() - 1;

    for ( ; iStartPos < str.length(); ++iStartPos)
    {
        if (0 == isspace(*(str.c_str() + iStartPos)))
        {
            break;
        }
    }

    for( ; iEndPos >= iStartPos; --iEndPos)
    {
        if (0 == isspace(*(str.c_str() + iEndPos)))
        {
            break;
        }

        if (iStartPos == iEndPos)
        {
            str = "";
            return;
        }
    }
    str = str.substr(iStartPos, iEndPos - iStartPos + 1);

    return;
    
}

int read_line(int fd, STLString& strData)
{
    char ch;

    strData.erase();

    do
    {

        ssize_t iReadNum = read(fd, &ch, sizeof(ch));

        if (iReadNum < 0)
        {
            return -1;
        }
        else if (0 == iReadNum)
        {
            break;
        }
        else
        {
            strData += ch;
        }

        if ('\n' == ch)
        {
            break;
        }
    
    }while(true);

    return strData.length();
}

int split_string(const char * str, char filter, std::list<STLString>& out_list)
{
    const char * start = str;
    const char * pos = str;
    while ('\0' != *pos)
    {
        if (filter == *pos)
        {
            if (start != pos)
            {
                out_list.push_back(STLString(start, pos - start));
            }
            ++pos;
            start = pos;
        }
        else
        {
            // not filter char
            ++pos;
        }
    }

    //push last string
    if (start != pos)
    {
        out_list.push_back(STLString(start, pos - start));
    }

    return (out_list.empty())? -1 : 0;
}
