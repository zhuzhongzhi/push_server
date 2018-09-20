
#include "zconfig.h"
#include "zmutex.h"
#include "ztracelog.h"
#include "zutil.h"


ZConfig::ZConfig()
{
    m_iLineNo           = 0;
    m_bOpen             = false;
    m_fd                = -1;
}


ZConfig::~ZConfig()
{
    clear();
}

ECfgErrCode ZConfig::open_file(const char * file)
{
    if (NULL == file)
    {
        return CFG_INPUT_INVALID;
    }

    if (m_bOpen)
    {
        return CFG_FILE_OPENED;
    }

    clear();

    m_strFileName = file;

    m_fd = open(m_strFileName.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

    if (-1 == m_fd)
    {
        LOGSYSWARN_S("Failed to open %s", m_strFileName.c_str());
        return CFG_OPEN_FILE_FAILED;
    }
    
    lseek(m_fd, 0, SEEK_SET);

    STLString   strTmpComment;
    STLString   strLine;

    try
    {
        do
        {
            ECfgLineType lineType = get_line(strLine);

            if (CFG_LINE_COMMENT == lineType)
            {
                strTmpComment += strLine;
                strTmpComment += '\n';
                continue;
            }
            else if(CFG_LINE_SECTION == lineType)
            {
                break;
            }
            else 
            {
                LOGWARN_S("Find the item before we find the section, please check the config file format!");
                throw CFG_FORMAT_ERROR;
            }       
            
        }while(true);

        do
        {
            TCfgSection* pSec = parser_section(strLine);

            pSec->m_strComment = strTmpComment;
            strTmpComment.erase();

            do
            {
                ECfgLineType lineType = get_line(strLine);

                if (CFG_LINE_COMMENT == lineType)
                {
                    strTmpComment += strLine;
                    strTmpComment += '\n';
                    continue;
                }
                else if(CFG_LINE_SECTION == lineType)
                {
                    break;
                }
                else 
                {
                    TCfgItem* pItem = parser_item(strLine, pSec);
                    pItem->m_strComment = strTmpComment;
                    strTmpComment.erase();
                }  
            }while(true);
            
            

        }while(true);
        
    }
    catch (ECfgErrCode errCode)
    {
        if (CFG_OK != errCode)
        {
            LOGWARN_S("Failed to open file %s, errCode=%d", m_strFileName.c_str(), errCode);
            clear();
            return errCode;
        }

        m_strLastComment = strTmpComment;
    }

    return CFG_OK;
}

ECfgErrCode ZConfig::close_file(const char* file)
{
    clear();
    return CFG_OK;
}

void ZConfig::clear()
{
    CLOSE_FD(m_fd);
    m_bOpen = false;  

    for (std::list<TCfgSection*>::iterator iterSec = m_listSec.begin(); iterSec != m_listSec.end(); ++iterSec)
    {
        TCfgSection* & pSec = *iterSec;


        for (std::list<TCfgItem*>::iterator iterItem = pSec->m_strItemList.begin(); iterItem != pSec->m_strItemList.end(); ++iterItem)
        {
            TCfgItem* & pItem = *iterItem;

            ZDELETE(pItem);
            
        } 

        ZDELETE(pSec);
    }
    m_listSec.clear();
    m_iLineNo = 0;
    m_strLastComment.erase();
    return;
}

ECfgErrCode ZConfig::save_file()
{    
    STLString strTmpFile = m_strFileName + ".xbak";
    int iFd = open(strTmpFile.c_str(), O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
    if (-1 == iFd)
    {
        LOGSYSWARN_S("Failed to open %s", strTmpFile.c_str());
        return CFG_OPEN_FILE_FAILED;
    }
    lseek(iFd , 0, SEEK_SET);
    std::string strFileContent;
    for (std::list<TCfgSection*>::iterator iterSec = m_listSec.begin(); iterSec != m_listSec.end(); ++iterSec)
    {
        TCfgSection* & pSec = *iterSec;
        strFileContent += pSec->m_strComment;
        strFileContent += '[';
        strFileContent += pSec->m_strSecName;
        strFileContent += "]\n";

        for (std::list<TCfgItem*>::iterator iterItem = pSec->m_strItemList.begin(); iterItem != pSec->m_strItemList.end(); ++iterItem)
        {
            TCfgItem* & pItem = *iterItem;
            strFileContent += pItem->m_strComment;
            strFileContent += pItem->m_strItemName;
            strFileContent += '=';
            strFileContent += pItem->m_strItemValue;
            strFileContent += "\n\n";            
        } 
        strFileContent += '\n';
    }

    strFileContent += m_strLastComment;

    if ((ssize_t)(strFileContent.length()) != write(iFd, strFileContent.c_str(), strFileContent.length()))
    {
        CLOSE_FD(iFd);
        LOGSYSWARN_S("Write %s error!", strTmpFile.c_str());
        return CFG_SYS_ERROR;
    }

    CLOSE_FD(iFd);

    remove(m_strFileName.c_str());
    rename(strTmpFile.c_str(), m_strFileName.c_str());
    return CFG_OK;
}


bool  ZConfig::check_sec_exist(const STLString& sec_name, TCfgSection* & sec)
{
    std::list<TCfgSection*>::iterator iter = m_listSec.begin();
    for ( ; iter != m_listSec.end(); ++iter)
    {
        TCfgSection* & pTmpSec = *iter;
        if (sec_name == pTmpSec->m_strSecName)
        {
            sec = pTmpSec;
            return true;
        }
    }
    sec = NULL;
    return false;
}


bool ZConfig::check_item_exist(const TCfgSection* sec, const STLString& item_name)
{
    std::list<TCfgItem*>::const_iterator iter = sec->m_strItemList.begin();
    for ( ; iter != sec->m_strItemList.end(); ++iter)
    {
        TCfgItem* const & pTmpItem = *iter;
        if (item_name == pTmpItem->m_strItemName)
        {
            return true;
        }
    }

    return false;
}

ECfgLineType ZConfig::get_line(STLString & line)
{
    ECfgLineType lineType = CFG_LINE_INVALID;
    for (; ;)
    {
        ++m_iLineNo;
        int iRet = read_line(m_fd, line);
        if (-1 == iRet)
        {
            throw CFG_SYS_ERROR;
        }

        if (0 == iRet)
        {
            throw CFG_OK;
        }

        trim_space(line);

        if (line.empty())
        {
            continue;
        }

        if ((0 == line.compare(0, 1, "["))
            && (0 == line.compare(line.length() - 1, 1, "]")))
        {
            lineType = CFG_LINE_SECTION;
            break;
        }

        if ((0 != line.compare(0, 1, "="))
            && (STLString::npos != line.find("=")))
        {
            //  LocalIp = 1.1.1.1 
            lineType = CFG_LINE_ITEM;
            break;
        }

        if (0 == line.compare(0, 1, "#"))
        {
            //  #Local Ip.
            lineType = CFG_LINE_COMMENT;
            break;
        }

        LOGWARN_S("The file  format error, lineno=%d, content=%s",
                m_iLineNo, line.c_str());
        throw CFG_FORMAT_ERROR;
    }

    return lineType;
}


TCfgSection* ZConfig::parser_section(const STLString & line)
{
    STLString strSecName = line.substr(1, line.length() - 2);
    trim_space(strSecName);

    if (strSecName.empty())
    {
        LOGWARN_S("Section Name is empty! lineno = %d", m_iLineNo);
        throw CFG_FORMAT_ERROR;
    }
    
    TCfgSection * pSec = NULL;
    if (check_sec_exist(strSecName, pSec))
    {
        LOGWARN_S("Section(%s) is repeat!", strSecName.c_str());
        throw CFG_SECTION_REPEAT;
    }
    
    ZNEW(pSec, TCfgSection);
    pSec->m_strSecName = strSecName;
    m_listSec.push_back(pSec);
    return pSec;
}

TCfgItem* ZConfig::parser_item(const STLString & line, TCfgSection * & sec)
{
    STLString::size_type pos = line.find("=");
    if (STLString::npos == pos)
    {
        throw CFG_FORMAT_ERROR;
    }

    STLString strItemName = line.substr(0, pos);
    trim_space(strItemName);

    if (check_item_exist(sec, strItemName))
    {
        LOGWARN_S("Item(%s) is repeat! lineno=%d", strItemName.c_str(), m_iLineNo);
        throw CFG_ITEM_REPEAT;
    }

    TCfgItem* pItem = NULL;
    ZNEW(pItem, TCfgItem);

    pItem->m_strItemName = strItemName;
    if (pos != line.length() - 1)
    {
        pItem->m_strItemValue =  line.substr(pos + 1, line.length() - pos - 1);
        trim_space(pItem->m_strItemValue);
    }

    sec->m_strItemList.push_back(pItem);
    return pItem;
    
}


ECfgErrCode  ZConfig::read_string_i(const char* sec_name, const char* item_name, STLString& value)
{
    if ((NULL == sec_name) || (NULL == item_name))
    {
        return CFG_INPUT_INVALID;
    }

    STLString strSecName = sec_name;   
    TCfgSection * pSec = NULL;
    if (!check_sec_exist(strSecName, pSec))
    {
        return CFG_ITEM_NOT_EXIST;
    }
    
    std::list<TCfgItem*>::iterator iter = pSec->m_strItemList.begin();
    for ( ; iter != pSec->m_strItemList.end(); ++iter)
    {
        TCfgItem* & pTmpItem = *iter;
        if (item_name == pTmpItem->m_strItemName)
        {
            value = pTmpItem->m_strItemValue;
            return CFG_OK;
        }
    }

    return CFG_ITEM_NOT_EXIST;
}

ECfgErrCode ZConfig::modify_string_i(const char* sec_name, const char* item_name, const STLString& value)
{
    if ((NULL == sec_name) || (NULL == item_name))
    {
        return CFG_INPUT_INVALID;
    }

    STLString strSecName = sec_name;   
    TCfgSection * pSec = NULL;
    if (!check_sec_exist(strSecName, pSec))
    {
        return CFG_ITEM_NOT_EXIST;
    }

    std::list<TCfgSection*>::iterator sec_iter = m_listSec.begin();
    for ( ; sec_iter != m_listSec.end(); ++sec_iter)
    {
        TCfgSection* & pTmpSec = *sec_iter;
        if (sec_name == pTmpSec->m_strSecName)
        {
            pSec = pTmpSec;
            break;
        }
    }

    if (NULL == pSec)
    {
        return CFG_INPUT_INVALID;
    }

    std::list<TCfgItem *>::iterator iter = pSec->m_strItemList.begin();
    for ( ; iter != pSec->m_strItemList.end(); ++iter)
    {
        TCfgItem* & pTmpItem = *iter;
        if (item_name == pTmpItem->m_strItemName)
        {
            pTmpItem->m_strItemValue = value;
            return save_file();
        }
    }    

    return CFG_ITEM_NOT_EXIST;
}


ECfgErrCode ZConfig::read_string(const char* sec_name, const char* item_name, STLString& value)
{
    return read_string_i(sec_name, item_name, value);
}

ECfgErrCode ZConfig::read_string(const char * sec_name, const char * item_name, char* value, int iLen)
{
    if ((NULL == sec_name) || (NULL == item_name) || (NULL == value) || (0 >= iLen))
    {
        return CFG_INPUT_INVALID;
    }

    STLString strValue;
    ECfgErrCode errCode = read_string_i(sec_name, item_name, strValue);
    if (CFG_OK != errCode)
    {
        return errCode;
    }

    if ((int)(strValue.length()) >= iLen)
    {
        return CFG_VALIE_INVALID;
    }

    memcpy(value, strValue.c_str(), strValue.length());
    value[strValue.length()] = '\0';
    return CFG_OK;
    
}

ECfgErrCode ZConfig::read_int(const char * sec_name, const char * item_name, int & value, int min_value, int max_value)
{
    STLString strValue;

    ECfgErrCode errCode = read_string_i(sec_name, item_name, strValue);
    if (CFG_OK != errCode)
    {
        return errCode;
    }

    int iTmpValue = atoi(strValue.c_str());
    char szBuff[MIN_BUFF_LEN + 1] = {0};
    snprintf(szBuff, sizeof(szBuff), "%d", iTmpValue);
    if (szBuff != strValue)
    {
        return CFG_VALIE_INVALID;
    }

    if ((INT_MIN != min_value) && (iTmpValue < min_value))
    {
        return CFG_VALIE_INVALID;
    }

    if ((INT_MAX != max_value) && (iTmpValue > max_value))
    {
        return CFG_VALIE_INVALID;
    }
    value = iTmpValue;
    return CFG_OK;
}

ECfgErrCode ZConfig::read_uint(const char * sec_name, const char * item_name, unsigned int & value, unsigned int min_value, unsigned int max_value)
{
    STLString strValue;
    ECfgErrCode errCode = read_string_i(sec_name, item_name, strValue);
    if (CFG_OK != errCode)
    {
        return errCode;
    }

    unsigned int iTmpValue = (unsigned int)strtoul(strValue.c_str(), NULL, 10);
    char szBuff[MIN_BUFF_LEN + 1] = {0};
    snprintf(szBuff, sizeof(szBuff), "%u", iTmpValue);
    if (szBuff != strValue)
    {
        return CFG_VALIE_INVALID;
    }

    if ((0 != min_value) && (iTmpValue < min_value))
    {
        return CFG_VALIE_INVALID;
    }

    if ((UINT_MAX != max_value) && (iTmpValue > max_value))
    {
        return CFG_VALIE_INVALID;
    }
    value = iTmpValue;
    return CFG_OK;
}


ECfgErrCode ZConfig::modify_string(const char* sec_name, const char* item_name, const STLString& value)
{
    return modify_string_i(sec_name, item_name, value);
}

ECfgErrCode ZConfig::modify_string(const char* sec_name, const char* item_name, const char* value)
{
    if (NULL == value)
    {
        return CFG_INPUT_INVALID;
    }

    STLString strValue = value;
    return modify_string_i(sec_name, item_name, strValue);    
}

ECfgErrCode ZConfig::modify_int(const char* sec_name, const char* item_name, const int value)
{
    char szBuff[MIN_BUFF_LEN + 1] = {0};
    snprintf(szBuff, sizeof(szBuff), "%d", value);
    STLString strValue = szBuff;
    return modify_string_i(sec_name, item_name, strValue);       
}

ECfgErrCode ZConfig::modify_uint(const char* sec_name, const char* item_name, const unsigned int value)
{
    char szBuff[MIN_BUFF_LEN + 1] = {0};
    snprintf(szBuff, sizeof(szBuff), "%u", value);
    STLString strValue = szBuff;
    return modify_string_i(sec_name, item_name, strValue);      
}

