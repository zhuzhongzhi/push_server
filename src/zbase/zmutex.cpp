
#include "zmutex.h"

ThreadMutex::ThreadMutex()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    ::pthread_mutex_init(&m_mutex, &attr);
    pthread_mutexattr_destroy(&attr);
}

ThreadMutex::~ThreadMutex()
{
    ::pthread_mutex_destroy(&m_mutex);
}

void ThreadMutex::lock()
{
    ::pthread_mutex_lock(&m_mutex);
}

void ThreadMutex::unlock()
{
    ::pthread_mutex_unlock(&m_mutex);
}


TRWMutex::TRWMutex()
{
    ::pthread_rwlock_init(&m_rwMutex, NULL);
}

TRWMutex::~TRWMutex()
{
    ::pthread_rwlock_destroy(&m_rwMutex);
}

void TRWMutex::lockRead()
{
    ::pthread_rwlock_rdlock(&m_rwMutex);
}

void TRWMutex::lockWrite()
{
    ::pthread_rwlock_wrlock(&m_rwMutex);
}

void TRWMutex::unlock()
{
    ::pthread_rwlock_unlock(&m_rwMutex);
}

