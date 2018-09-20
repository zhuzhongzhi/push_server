#ifndef __NL_QUEUE_H__
#define __NL_QUEUE_H__

#include <string>
#include "ace/Atomic_Op.h"
#include "ace/Thread_Mutex.h"
#include "znlmessage.h"

class NLQueue
{
public:
    enum
    {
        NL_QUEUE_SIZE = 1024,
        NL_QUEUE_FULL_SIZE = 1024,
        NL_QUEUE_LAST_SLOT = 1024, // NL_QUEUE_SIZE - 1
    };

    NLQueue();
    virtual ~NLQueue();
    
public:
    bool put_message(NLMessage* msg);
    bool pop_message(NLMessage*& msg);

    inline bool is_empty()
    {
        return (m_iQueueSize == 0);
    }
    inline bool is_full()
    {
        return (m_iQueueSize >= NL_QUEUE_FULL_SIZE);
    }
    
private:
    int                                     m_iQueueHeader;
    ACE_Atomic_Op<ACE_Thread_Mutex, int>    m_iQueueWriteTail;
    ACE_Atomic_Op<ACE_Thread_Mutex, int>    m_iQueueSize;
    
    NLMessage*                              m_pQueueMessages[NL_QUEUE_SIZE];
};

#endif