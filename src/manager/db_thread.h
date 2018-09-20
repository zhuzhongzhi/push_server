#ifndef __DB_THREAD_H__
#define __DB_THREAD_H__
#include <map>
#include "group_info.h"
#include "ztqthread.h"

class User_Info_API;
class Offline_API;
class Chat_Record_API;
class DBThread : public TQThread
{
public:
    DBThread();
    virtual ~DBThread();

public:
    virtual int init(int id);
    int on_service(NLMessage*& msg);
    
    int on_auth_req(NLMessage*& msg);
    int on_group_req(NLMessage*& msg);
    int on_chat_record(NLMessage*& msg);
    int on_online_req(NLMessage*& msg);
    int on_offline(NLMessage*& msg);
        
private:
    User_Info_API*    m_UserInfoAPI;
    Offline_API*      m_OfflineAPI;
    Chat_Record_API*  m_RecordAPI;
    std::map<UserIDType, GroupInfo*>  m_GroupInfos;
};

bool compare_offline (const NLMessage* first, const NLMessage* second);


#endif

