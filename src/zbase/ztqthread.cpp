#include <unistd.h>
#include <stdlib.h>
#include "ace/Event_Handler.h"
#include "ace/Reactor.h"
#include "zbasedef.h"
#include "znlmessage.h"
#include "zlogger.h"
#include "ztqthread.h"

TQThread::TQThread()
{
	m_TimeId = -1;
    m_iTimeoutFlag = 0;
    m_CurrentTime = time (NULL);
}

TQThread::~TQThread()
{
}

int TQThread::init()
{
    return EventThread::init();
}

void TQThread::on_time ()
{
    if (m_iTimeoutFlag > 0)
    {
        // drive inner timer queue
    	m_CurrentTime = time (NULL);
        ACE_Time_Value expire_time (m_CurrentTime, 0);
        m_TimeQueue.expire(expire_time);

        m_iTimeoutFlag = 0;
    }
    else
    {
        EventThread::on_time();
    }
}

long TQThread::set_timer(TQHandler* handler, int future, int interval, void* act)
{
    LOGINFO ("thread set timer, handle[%p], future[%d], interval[%d].",
             handler,
             future,
             interval);
    long timer_id = -1;
    SET_QUEUE_TIMER(future, interval, handler, timer_id, act);
    return timer_id;
}

void TQThread::cancle_timer (long time_id)
{
    LOGINFO ("thread cancle timer id[%ld].", time_id);
    void* act = NULL;
    m_TimeQueue.cancel(time_id, (const void**) &act);
}

int TQThread::handle_timeout (const ACE_Time_Value &current_time,
                                const void *act)
{
    //LOGINFO("time pulse, create a time message and put it to inner queue.");
    m_iTimeoutFlag++;
    return 0;
}

int TQThread::on_init()
{
    LOGINFO ("thread on init.");
    // thread pulse timer
	SET_THREAD_TIMER(500000, 500000, m_TimeId);

    return ((long) (-1) == m_TimeId) ? -1 : 0;
}

time_t TQThread::get_current_time ()
{
    return m_CurrentTime;
}

