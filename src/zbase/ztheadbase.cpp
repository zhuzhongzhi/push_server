#include <sys/select.h>
#include <pthread.h>
#include "zlogger.h"
#include "ztheadbase.h"

void* my_thread_func(void *arg)
{    
	ThreadBase * pThread = (ThreadBase *)arg;
    LOGINFO("thread start,  ThreadBase[%p].", pThread);
    
    if (0 == pThread->on_init())
    {
        pThread->on_run();
    }    
    
    pThread->on_exit();

    LOGINFO("thread stop, delete ThreadBase[%p].", pThread);
    ZDELETE(pThread);
    return 0;
}

ThreadBase::ThreadBase()
{
    m_ThreadId = 0;
    m_ThreadIndex = 0;
}

ThreadBase::~ThreadBase()
{
}

int ThreadBase::on_init()
{
    LOGINFO("ThreadBase on_init.");
    return 0;
}

int ThreadBase::on_run()
{
    LOGINFO("ThreadBase on_run.");
    return 0;
}

int ThreadBase::on_exit()
{
    LOGINFO("ThreadBase on_exit.");
    return 0;
}

void ThreadBase::on_idle(int ms)
{
    struct timeval timeWait;
    timeWait.tv_sec = ms / 1000;
    timeWait.tv_usec = (ms % 1000) * 1000;

    select(0, (fd_set*)0, (fd_set*)0, (fd_set*)0, &timeWait);
}


int ThreadBase::create_thread()
{
	int ret = pthread_create(&m_ThreadId, NULL, my_thread_func, (void*)this);
	if (ret != 0)
	{
        LOGWARN("pthread_create() failed: %d", ret);
        return -1;
	}
	return 0;
}


