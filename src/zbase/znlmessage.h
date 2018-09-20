#ifndef __NL_MESSAGE_H__
#define __NL_MESSAGE_H__

#include <list>
#include "ace/Atomic_Op.h"
#include "ace/Thread_Mutex.h"
#include "zbasedef.h"

class NLMessage
{
public:
    NLMessage();
    virtual ~NLMessage();
    
    int get_type();
    void set_type(int t);

    virtual time_t message_time() const
    {
        return 0;
    }
    
    INLINE void reference()
    {
        m_RefNum++;
    }
    
    INLINE void release()
    {
        m_RefNum--;
    }

public:
    int                                     m_Type;
    ACE_Atomic_Op<ACE_Thread_Mutex, int>    m_RefNum;
    long                                    m_TimerID;
};

struct less_nl
{
    bool operator()(const NLMessage*& _X, const NLMessage*&  _Y) const
    {
        return (_X->message_time() < _Y->message_time());
    }
};

class ConnectMsg : public NLMessage
{
public:
    ConnectMsg();    
    virtual ~ConnectMsg();

public:
    int m_PeerFd;
};

class LogMsg : public NLMessage
{
public:
    enum
    {
        DEFAULT_BUFFER_SIZE = 1024,
    };
public:
    LogMsg(int size);    
    virtual ~LogMsg();
    void clear();

public:
    int                     m_Level;
    int                     m_BufferSize;
    char*                   m_Buffer;
    int                     m_ArraySize;
};

#endif