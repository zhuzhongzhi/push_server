#ifndef _DISPATCH_H__
#define _DISPATCH_H__

#include <vector>
#include "zbasedef.h"

enum TThreadType
{
    T_LOGGER = 0,
    T_MQTT,
    T_DB,
    T_APNS
};

class NLMessage;
class IDispatch;

typedef std::vector<IDispatch*>  THREAD_TYPE;
class Dispatch
{
public:
    enum
    {
        MAX_TYPE = 20
    };
    
public:
    Dispatch();
    virtual ~Dispatch();

    static Dispatch* instance();

public:
    int post_message(NLMessage* msg, int type);

    int post_message(NLMessage* msg, int type, int index);
    
    int post_message_by_user(NLMessage* msg, int type, UserIDType user_id);

    int register_thread(IDispatch* thread, int type);
        
private:
    THREAD_TYPE    threads[MAX_TYPE];
    int threads_index[MAX_TYPE];
    int threads_size[MAX_TYPE];

    static Dispatch  *m_instance;
};

#endif
