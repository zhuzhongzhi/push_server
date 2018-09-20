
#include "zconfiger.h"

Configer  *Configer::m_Instance = NULL;

Configer::Configer()
{
}

Configer::~Configer()
{
}

Configer *Configer::instance()
{
    if (NULL == m_Instance)
    {
         ZNEW(m_Instance, Configer); 
    }

    return m_Instance;
}

int Configer::init(const char* work_dir)
{
    STLString strFile = work_dir;
    strFile += "/config/common.ini";

    ECfgErrCode errCode = m_Config.open_file(strFile.c_str());
    if (CFG_OK != errCode)
    {
        LOGWARN("Configer open file[%s] [%s], resulr[%d].", strFile.c_str(), RED_TEXT(FAILED), errCode);
        return -1;
    }

    LOGINFO("Configer open file[%s] [%s].", strFile.c_str(), GREEN_TEXT(OK));

    if (0 != read_item())
    {
        LOGWARN("Configer read item from file[%s] [%s].", strFile.c_str(), RED_TEXT(FAILED));
        return -1;
    }
    LOGWARN("Configer read item from file[%s] [%s].", strFile.c_str(), GREEN_TEXT(OK));
    m_Config.close_file(strFile.c_str());   
    return 0;
}

