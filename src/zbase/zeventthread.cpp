
#include "zeventthread.h"

EventThread::EventThread() 
{
}

EventThread::~EventThread() 
{
}

int EventThread::init()
{
	return m_FdDriver.init(this);
}

void EventThread::on_event()
{
    if (0 == m_FdDriver.run())
    {
        QueueThread::on_event();
    }
}

bool EventThread::event_register (int fd, unsigned int event)
{
    return m_FdDriver.event_register (fd, event);
}

bool EventThread::event_unregister (int fd, unsigned int event)
{
    return m_FdDriver.event_unregister (fd, event);
}

int EventThread::handle_input(int fd)
{
	return 0;
}

int EventThread::handle_output(int fd)
{
	return 0;
}

