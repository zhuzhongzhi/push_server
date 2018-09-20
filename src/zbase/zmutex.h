
#ifndef _X_MUTEX_H_
#define _X_MUTEX_H_

#include <pthread.h>
#include "zbasedef.h"

//base mutex
class TMutex
{
public:    
    TMutex(){};
    virtual ~TMutex(){};

    //lock mutex
    virtual void lock() = 0;

    //unlock mutex
    virtual void unlock() = 0;
private:
    DEF_COPY_AND_ASSIGN(TMutex);
};

class TNULLMutex: public TMutex
{
public:    
    TNULLMutex(){};
    virtual ~TNULLMutex(){};

    //lock mutex
    virtual void lock(){return; };

    //unlock mutex
    virtual void unlock(){return; };
private:
    DEF_COPY_AND_ASSIGN(TNULLMutex);

};

class ThreadMutex : public TMutex
{
public:    
    ThreadMutex();
    virtual ~ThreadMutex();

    //lock mutex
    virtual void lock();

    //unlock mutex
    virtual void unlock();
private:
    DEF_COPY_AND_ASSIGN(ThreadMutex);

    pthread_mutex_t                         m_mutex;     
};

class TRWMutex
{
public:    
    TRWMutex();
    virtual ~TRWMutex();

    //read lock mutex
    void lockRead();

    //write lock mutex
    void lockWrite();
    
    //unlock mutex
    void unlock();
private:
    DEF_COPY_AND_ASSIGN(TRWMutex);

    pthread_rwlock_t                        m_rwMutex;     
};    


template<class T>
class TMutexGuard
{
public:
    TMutexGuard(T & mutex) : m_mutex(mutex)
    {
        m_mutex.lock();
    };
    
    ~TMutexGuard(){m_mutex.unlock();};

private:
    DEF_COPY_AND_ASSIGN(TMutexGuard);
    
private:
    T &                                     m_mutex;
};

#endif

