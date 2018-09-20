#ifndef __SESSION_MANGER_H__
#define __SESSION_MANGER_H__

#include "ace/Event_Handler.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include <list>
#include <map>
#include "zbasedef.h"
/**
fd是全局的应该不存在同一个fd在不同线程内的
也就是不会有多个线程访问同一块内存的情况
*/
#define MAX_FD 65535
class Session;
class ServiceThread;

class UserNode
{
public:
    UserNode();
    virtual ~UserNode();

    
public:
    UserIDType  m_UserID;
    Session*    m_Session;
    UserNode*  m_Next;
};

class SessionManager
{
public:
    SessionManager();
    virtual ~SessionManager();

public:
    int init();
    Session* create_session(ACE_HANDLE fd, ServiceThread* thread);
    int map_userid_and_session(Session* session);
    Session* get_session(ACE_HANDLE fd);
    void get_session(UserIDType use_id, std::list<Session*>& sessions);
    void get_session(UserIDType use_id, Session*& sessions);
    void release_session(ACE_HANDLE fd, UserIDType user_id);
    
private:
    Session*                    m_Session[MAX_FD];

    /**  多线程环境下如何做到高效**/
    UserNode*                  m_UserFdMap[MAX_FD]; /**根据用户ID查找session*/
    ACE_Thread_Mutex            m_UserFdMutex[MAX_FD];
};


typedef ACE_Singleton<SessionManager, ACE_Recursive_Thread_Mutex> SessionMgrIns;


#endif
