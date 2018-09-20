
#include "znlmessage.h"
#include "zidispatch.h"
#include "zdispatch.h"

Dispatch  *Dispatch::m_instance = NULL;

Dispatch::Dispatch()
{
    for (int i = 0; i < MAX_TYPE; ++i)
    {
        threads[i].clear();
        threads_index[i] = threads_size[i] = 0;
    }
}

Dispatch::~Dispatch()
{
}

Dispatch *Dispatch::instance()
{
    if (NULL == m_instance)
    {
        ZNEW(m_instance, Dispatch);
    }

    return m_instance;
}

int Dispatch::post_message(NLMessage* msg, int type)
{
    if ((type >= 0) && (type < MAX_TYPE))
    {
        std::vector<IDispatch*> & thread_vec = threads[type];
        int index = threads_index[type];        
        if (0 != thread_vec[index]->post_message(msg))
        {
            return -1;
        }
        else
        {
            ++index;
            if (index >= threads_size[type])
            {
                index = 0;
            }
            threads_index[type] = index;
        }
        return 0;
    }
    else
    {
        return -1;
    }
}

int Dispatch::post_message(NLMessage* msg, int type, int index)
{
    if ((type >= 0) && (type < MAX_TYPE))
    {
        std::vector<IDispatch*> & thread_vec = threads[type];
        if ((index >= 0) && (index < thread_vec.size()))
        {
            if (0 == thread_vec[index]->post_message(msg))
            {
                return 0;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }    
}

int Dispatch::post_message_by_user(NLMessage* msg, int type, UserIDType user_id)
{
    if ((type >= 0) && (type < MAX_TYPE))
    {
        std::vector<IDispatch*> & thread_vec = threads[type];
        int index = user_id % thread_vec.size();
        if ((index >= 0) && (index < thread_vec.size()))
        {
            if (0 == thread_vec[index]->post_message(msg))
            {
                return 0;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }    
}

int Dispatch::register_thread(IDispatch* thread, int type)
{
    if ((type >= 0) && (type < MAX_TYPE))
    {
        threads[type].push_back(thread);
        threads_size[type] = threads[type].size();
        return 0;
    }
    else
    {
        return -1;
    }
}


