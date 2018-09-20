

#ifndef SRC_THREAD_QUEUE_THREAD_H_
#define SRC_THREAD_QUEUE_THREAD_H_

#include <map>
#include <functional>
#include <iostream>
#include "ztheadbase.h"
#include "zidispatch.h"
#include "znlqueue.h"
#include "znlmessage.h"

#define MAX_RUN_INNER_NUM 10

typedef std::function<int(NLMessage*&)>   MsgFunc;

class QueueThread: public ThreadBase ,public IDispatch
{
public:
	QueueThread();
	virtual ~QueueThread();

public:
	virtual int post_message(NLMessage* msg);
	virtual int on_message(NLMessage*& msg);
	virtual int on_run();
	virtual void on_run_queue();
	virtual void on_run_idle();
	virtual void on_time();
	virtual void on_event();

	void support(int type, const MsgFunc &handler);

private:
	NLQueue     m_MessageQueue;

protected:
	int          m_OnIdleFlag;
	int          m_ExitFlag;
	std::map<int, MsgFunc> m_MsgMethods;
};

#endif /* SRC_THREAD_QUEUE_THREAD_H_ */
