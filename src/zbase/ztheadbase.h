#ifndef __THREAD_BASE_H__
#define __THREAD_BASE_H__

#include "zbasedef.h"

void* my_thread_func(void *arg);

class ThreadBase
{
public:
	ThreadBase();
    virtual ~ThreadBase();

public:
    int create_thread();

    virtual int on_init();
    virtual int on_run(); 
    virtual int on_exit(); 
    void on_idle(int ms);

    INLINE pthread_t get_thread_id()
    {
        return m_ThreadId;
    }
    
    INLINE int get_index()
    {
        return m_ThreadIndex;
    }

    INLINE void set_index(int index)
    {
        m_ThreadIndex = index;
    }
private:
    pthread_t   m_ThreadId;
    int         m_ThreadIndex;
};



#endif
