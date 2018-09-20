#ifndef __TQ_SERVICE_THREAD_H__
#define __TQ_SERVICE_THREAD_H__

#include "zbasedef.h"
#include "zeventthread.h"
#include "ztimerqueue.h"

class TQThread : public EventThread, public TQHandler
{
public:
	TQThread();
    virtual ~TQThread();

    virtual int init();
public:
    virtual int on_init();
    virtual void on_time();
    long set_timer(TQHandler* handler, int future, int interval, void* act);
    void cancle_timer(long time_id);
    virtual int handle_timeout(const ACE_Time_Value &current_time, const void *act);
    time_t get_current_time();

protected:
    long           m_TimeId;
    Timer_Queue    m_TimeQueue;
    time_t         m_CurrentTime;
    int            m_iTimeoutFlag;
};

#endif
