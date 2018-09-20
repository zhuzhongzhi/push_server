#ifndef __OFFLINE_API_H__
#define __OFFLINE_API_H__

#include "db_info.h"

class NLMessage;
class Offline_API
{
public:
    Offline_API(){};
    virtual ~Offline_API(){};

public:
    virtual int init(const DBInfo& info) = 0;

    virtual int insert_offline_message(NLMessage* msg) = 0;
    virtual int query_user_offline_message(long long user_id, std::list<NLMessage*>& offline_messages) = 0;
    
protected:
    DBInfo   m_Info;
};
#endif
