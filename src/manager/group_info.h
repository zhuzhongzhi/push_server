#ifndef __GROUP_INFO_H__
#define __GROUP_INFO_H__

#include <list>
#include "zbasedef.h"

class GroupInfo
{
public:
    GroupInfo()
    {
        m_QueryTime = time(NULL);
    };
    virtual ~GroupInfo() {};

public:
    time_t                  m_QueryTime;
    std::list<UserIDType>   m_GroupUserIDs;
};
#endif