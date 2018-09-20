
#include "zlogger.h"
#include "zqueuethread.h"

QueueThread::QueueThread() 
{
	m_OnIdleFlag = 0;
	m_ExitFlag = 0;
}

QueueThread::~QueueThread() 
{
}

int QueueThread::post_message(NLMessage* msg)
{
	return (m_MessageQueue.put_message (msg)) ? 0 : -1;
}

void QueueThread::support(int type, const MsgFunc &handler)
{
	m_MsgMethods[type] = handler;
}

int QueueThread::on_message(NLMessage*& msg)
{
	// deal inner message
	if(m_MsgMethods.count(msg->get_type()))
	{
        return m_MsgMethods[msg->get_type()](msg);
	}
	return 0;
}

void QueueThread::on_run_queue()
{
	NLMessage* msg = NULL;
	int processd_nums = 0;
	processd_nums = 0;
	do
	{
        // get message from inner queue
        if (m_MessageQueue.pop_message(msg))
        {
            ++processd_nums;
            if (NULL != msg)
            {
                // deal inner message
                on_message (msg);
                if (NULL != msg)
                {
                    delete msg;
                    msg = NULL;
                }
            }
            else
            {
                LOGINFO ("thread inner count a null message.");
                break;
            }
        }
        else
        {
            //LOGINFO ("thread no inner message.");
            m_OnIdleFlag += 1;
            break;
        }
	}
	while (processd_nums < MAX_RUN_INNER_NUM);
}

void QueueThread::on_run_idle()
{
	if (m_OnIdleFlag > 2)
	{
        on_idle (100);
	}
	m_OnIdleFlag = 0;
}

void QueueThread::on_time()
{
	//LOGINFO ("QueueThread on time.");
	m_OnIdleFlag += 1;
}

void QueueThread::on_event()
{
	//LOGINFO ("QueueThread on event.");
	m_OnIdleFlag += 1;
}

int QueueThread::on_run()
{
	LOGINFO ("QueueThread on run.");
	while (true)
	{
        // drive inner timer queue
        on_time();

        // deal inner queue message
        on_run_queue();

        // epoll events
        on_event();

        // if need sleep
        on_run_idle();

        // if exit
        if (m_ExitFlag > 0)
        {
            LOGINFO ("thread exit flag is on, now exit thread.");
            break;
        }
	};

	return 0;
}
