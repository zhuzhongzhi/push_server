
#include "zlogger.h"
#include "session.h"
#include "service_thread.h"
#include "session_manger.h"

UserNode::UserNode()
{
    m_UserID = -1;
    m_Session = NULL;
    m_Next = NULL;
}

UserNode::~UserNode()
{
    m_UserID = -1;
    m_Session = NULL;
    m_Next = NULL;
}

SessionManager::SessionManager()
{
    for (int i = 0; i < MAX_FD; ++i)
    {
        m_Session[i] = NULL;
        m_UserFdMap[i] = NULL;        
    }
}

SessionManager::~SessionManager()
{
}

int SessionManager::init()
{
    for (int i = 0; i < MAX_FD; ++i)
    {
        m_Session[i] = NULL;
        m_UserFdMap[i] = NULL;        
    }

    return 0;
}

Session* SessionManager::create_session(ACE_HANDLE fd, ServiceThread* thread)
{
    if (NULL != m_Session[fd])
    {
        if (m_Session[fd]->valid())
        {
            m_Session[fd]->unregister_all();
        }
        m_Session[fd]->clear();
        m_Session[fd]->set(fd, thread);
        //delete m_Session[fd];
    }
    else
    {
        try
        {
            m_Session[fd] = new Session(fd, thread);
            m_Session[fd]->init();
            m_Session[fd]->set(fd, thread);
        }
        catch(...)
        {
            return NULL;
        }
    }
    return m_Session[fd];
}

int SessionManager::map_userid_and_session(Session* session)
{
    // 考虑用户多点登录的问题    
    int slot_id = session->get_user_id() % MAX_FD;
    LOGINFO("Session Manager User[%lld] slot id[%d].", session->get_user_id(), slot_id);

    ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
    UserNode* unode = m_UserFdMap[slot_id];
    if (NULL == unode)
    {
        LOGINFO("Add first new user node.");
        //ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
        ZNEW(unode, UserNode);
        m_UserFdMap[slot_id] = unode;
        unode->m_UserID = session->get_user_id();
        unode->m_Session = session;
        return 0;
    }
    else
    {
        do
        {
            // 找一个空的位置放进去
            if (unode->m_UserID == -1)
            {
                LOGINFO("user a free node.");
                unode->m_UserID = session->get_user_id();
                unode->m_Session = session;
                return 0;
            }
            else
            {
                if (NULL != unode->m_Next)
                {
                    unode = unode->m_Next;
                }
                else
                {
                    LOGINFO("Add new user node.");
                    //ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
                    ZNEW(unode->m_Next, UserNode);
                    unode->m_Next->m_UserID = session->get_user_id();
                    unode->m_Next->m_Session = session;
                    return 0;
                }
            }
        }
        while (1);
    }

    return 0;
}


Session* SessionManager::get_session(ACE_HANDLE fd)
{
    return m_Session[fd];
}


// 查找接收方, 这个是全局的需要锁
void SessionManager::get_session(UserIDType use_id, std::list<Session*>& sessions)
{
    // 考虑多点登录的问题的话， 这里可能找到多个session
    sessions.clear();
    Session* one_session = NULL;
    int slot_id = use_id % MAX_FD;
    LOGINFO("Session Manager get one session by user id[%lld], slot id is %d.", use_id, slot_id);
    ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
    UserNode* unode = m_UserFdMap[slot_id];
    if (NULL == unode)
    {
        LOGINFO("User[%lld] can out one session, UserNode is NULL.", use_id);
        return;
    }
    else
    {
        do
        {
            if (unode->m_UserID == use_id)
            {
                // session 是不会删除的，会重用， 每次clear一下
                // 因此用的地方需要在判断一下状态
                LOGINFO("Got one session[%p].", unode->m_Session);
                one_session = unode->m_Session;
                if ((NULL != one_session) && (one_session->valid()))
                {
                    sessions.push_back(one_session);
                }
                
                if (NULL != unode->m_Next)
                {
                    unode = unode->m_Next;
                    one_session = NULL;
                }
                else
                {
                    LOGINFO("End to find User[%lld], find %d session.", use_id, sessions.size());
                    return;
                }
                
                //return;
            }
            else
            {
                if (NULL != unode->m_Next)
                {
                    unode = unode->m_Next;
                    one_session = NULL;
                }
                else
                {
                    LOGINFO("End to find User[%lld], find %d session.", use_id, sessions.size());
                    return;
                }
            }
        }
        while (1);
    }
}

void SessionManager::get_session(UserIDType use_id, Session*& sessions)
{
    // 不考虑多点接入        
    int slot_id = use_id % MAX_FD;
    LOGINFO("Session Manager get one session by user id[%lld], slot id is %d.", use_id, slot_id);
    ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
    UserNode* unode = m_UserFdMap[slot_id];
    if (NULL == unode)
    {
        LOGINFO("User[%lld] can out one session, UserNode is NULL.", use_id);
        sessions = NULL;
        return;
    }
    else
    {
        do
        {
            if (unode->m_UserID == use_id)
            {
                // session 是不会删除的，会重用， 每次clear一下
                // 因此用的地方需要在判断一下状态
                LOGINFO("Got one session[%p].", unode->m_Session);
                sessions = unode->m_Session;
                return;
            }
            else
            {
                if (NULL != unode->m_Next)
                {
                    unode = unode->m_Next;
                }
                else
                {
                    LOGINFO("User[%lld] can out one session.", use_id);
                    sessions = NULL;
                    return;
                }
            }
        }
        while (1);
    }
}

void SessionManager::release_session(ACE_HANDLE fd, UserIDType user_id)
{
    if (NULL != m_Session[fd])
    {
        // 从user id和session的对应关系中去掉该记录
        int slot_id = user_id % MAX_FD;
        ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
        UserNode* unode = m_UserFdMap[slot_id];
        if (NULL == unode)
        {
            // 还没有来得及解析出UserID
            m_Session[fd]->clear();
            return;
        }
        do
        {
            if ((unode->m_UserID == user_id) 
             && (unode->m_Session->get_peer_fd() == fd))
            {
                LOGINFO("release_session[%p] User[%d:%lld].", unode->m_Session, fd, user_id);
                //ACE_Guard<ACE_Thread_Mutex>  guard(m_UserFdMutex[slot_id]);
                unode->m_Session = NULL;
                unode->m_UserID = -1;
            }
            else
            {
                if (NULL != unode->m_Next)
                {
                    unode = unode->m_Next;
                }
                else
                {
                    break;
                }
            }
        }
        while (1);
        
        m_Session[fd]->clear();
        //delete m_Session[fd];
    }
}


