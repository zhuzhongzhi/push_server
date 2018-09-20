#ifndef __SESSION_MANGER_H__
#define __SESSION_MANGER_H__

#include "ace/Event_Handler.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include <list>
#include <map>
#include "zbasedef.h"
/**
fd��ȫ�ֵ�Ӧ�ò�����ͬһ��fd�ڲ�ͬ�߳��ڵ�
Ҳ���ǲ����ж���̷߳���ͬһ���ڴ�����
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

    /**  ���̻߳��������������Ч**/
    UserNode*                  m_UserFdMap[MAX_FD]; /**�����û�ID����session*/
    ACE_Thread_Mutex            m_UserFdMutex[MAX_FD];
};


typedef ACE_Singleton<SessionManager, ACE_Recursive_Thread_Mutex> SessionMgrIns;


#endif
