
#include "zlogger.h"
#include "zepolldriver.h"

EpollDriver::EpollDriver()
:m_fEpollFD(-1), m_iValidFDNum(0)
{
    for (int i = 0; i < EPOLL_MAX_FD; ++i)
    {
        m_iEventFD[i] = 0;
    }
}

EpollDriver::~EpollDriver()
{
    if (-1 != m_fEpollFD)
    {
        close(m_fEpollFD);
        m_fEpollFD = -1;
    }
}

int EpollDriver::init(EpollInterface* thread)
{
    m_Thread = thread;
    m_fEpollFD = epoll_create(EPOLL_MAX_EVENTS);
    if (-1 == m_fEpollFD)
    {
        LOGWARN("Epoll_Driver init epoll_create failed!");
        return -1;
    }

    LOGWARN("Epoll_Driver init epoll_create OK!");
    return 0;
}

int EpollDriver::run()
{
    if (m_iValidFDNum <= 0)
    {
        //LOGINFO("no fd wait for event!");
        return 0;
    }
    struct epoll_event events[EPOLL_MAX_EVENTS];
    int event_nums =  epoll_wait(m_fEpollFD, events,
                            EPOLL_MAX_EVENTS, EPOLL_TIMEOUT);
    //LOGINFO("Epoll_Driver run epoll_wait's result is %d, m_iValidFDNum is %d.", event_nums, m_iValidFDNum);
    if (event_nums > 0)
    {
        for (int i=0; i<event_nums; ++i)
        {
            //LOGINFO("Epoll_Driver run FD[%d]'s event[%u].", events[i].data.fd, events[i].events);
            
            // 先处理in的
            if ((events[i].events & EPOLLIN) || (events[i].events & EPOLLPRI))
            {
                if (0 != m_Thread->handle_input(events[i].data.fd))
                {
                    // 异常了，不处理下面的事件了
                    continue;
                }
            }

            // 再处理out的
            if (events[i].events & EPOLLOUT)
            {
                if (0 != m_Thread->handle_output(events[i].data.fd))
                {
                    // 异常了，不处理下面的事件了
                    continue;
                }
            }
        }
    }
    
    return event_nums;
}

bool EpollDriver::event_register(int fd, unsigned int event)
{
    if (m_iEventFD[fd] & event)
    {
        // 已经设置了
        LOGINFO("FD[%d]'s event[%d] already register this event[%d]", 
           fd, m_iEventFD[fd], event);
        
        return true;
    }
    int op;
    if (m_iEventFD[fd])
    {
        op = EPOLL_CTL_MOD;        
    }
    else
    {
        op = EPOLL_CTL_ADD;
        ++m_iValidFDNum;
    }
    
    
    m_iEventFD[fd] = m_iEventFD[fd] | event;
    
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = m_iEventFD[fd]; //EPOLLIN|EPOLLET;
    int result = epoll_ctl(m_fEpollFD, op, fd, &ev);
    if (0 != result)
    {
        LOGWARN("FD[%d] register event[%u], epoll_ctl failed! errno is %d", fd, event, errno);
        return false;
    }

    LOGINFO("FD[%d] register event[%d] success, now event is %u", 
           fd, event, m_iEventFD[fd]);
    
    return true;
}

bool EpollDriver::event_unregister(int fd, unsigned int event)
{
    if (!(m_iEventFD[fd] & event))
    {
        // 已经取消设置了
        LOGINFO("FD[%d]'s event[%d] already unregister this event[%d]", 
           fd, m_iEventFD[fd], event);
        
        return true;
    }

    int op;
    if (m_iEventFD[fd] != event)
    {
        op = EPOLL_CTL_MOD;        
    }
    else
    {
        op = EPOLL_CTL_DEL;
        --m_iValidFDNum;
    }
    
    m_iEventFD[fd] = m_iEventFD[fd] & (~event);
    
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = m_iEventFD[fd]; //EPOLLIN|EPOLLET;
    
    if (0 != epoll_ctl(m_fEpollFD, op, fd, &ev))
    {
        LOGWARN("FD[%d] register event[%u], epoll_ctl failed!", fd, event);
        return false;
    }

    LOGINFO("FD[%d] unregister event[%d] success, now event is %u", 
           fd, event, m_iEventFD[fd]);
    
    return true;
}

