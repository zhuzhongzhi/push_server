#ifndef __SERVICE_THREAD_H__
#define __SERVICE_THREAD_H__

#include "ztqthread.h"

class NLMessage;
class ServiceThread : public TQThread
{
public:
    enum
    {
        MAX_INNER_NUM = 30,
    };
public:
    ServiceThread();
    virtual ~ServiceThread();

    virtual int init(int id);
public:
    int on_connect(NLMessage*& msg);
    int on_mqtt(NLMessage*& msg);
    int on_service(NLMessage*& msg);
    int on_control(NLMessage*& msg);
    int on_online_rsp(NLMessage*& msg);
    int on_auth_rsp(NLMessage*& msg);
    int on_group_rsp(NLMessage*& msg);
    int on_kick_off_user(NLMessage*& msg);

    // EpollInterface
    virtual int handle_input(int fd);
    virtual int handle_output(int fd);
};

#endif
