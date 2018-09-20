
#ifndef SRC_THREAD_EVENT_THREAD_H_
#define SRC_THREAD_EVENT_THREAD_H_

#include "zepolldriver.h"
#include "zqueuethread.h"
#include "zidispatch.h"

class EventThread: public QueueThread, public EpollInterface
{
public:
	EventThread();
	virtual ~EventThread();

	virtual int init();

public:
    virtual void on_event();
    
	bool event_register(int fd, unsigned int event);
	bool event_unregister(int fd, unsigned int event);

    virtual int handle_input(int fd);
    virtual int handle_output(int fd);

protected:
	EpollDriver   m_FdDriver;
};

#endif /* SRC_THREAD_EVENT_THREAD_H_ */
