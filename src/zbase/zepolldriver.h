#ifndef __EPOLL_DRIVER_H__
#define __EPOLL_DRIVER_H__

#include <sys/epoll.h>

#define ALL_EVENT EPOLLIN|EPOLLPRI|EPOLLOUT|EPOLLRDHUP|EPOLLHUP
#define READ_EVENT EPOLLIN|EPOLLPRI|EPOLLRDHUP|EPOLLHUP
#define WRITE_EVENT EPOLLOUT|EPOLLRDHUP|EPOLLHUP

class EpollInterface
{
public:
    virtual int handle_input(int fd) = 0;
    virtual int handle_output(int fd) = 0;
};

class EpollDriver
{
public:
    enum
    {
        EPOLL_MAX_FD = 65535,
        EPOLL_MAX_EVENTS = 6000,
        EPOLL_TIMEOUT = 500,
    };
    
public:
    EpollDriver();
    virtual ~EpollDriver();

public:
    int init(EpollInterface* thread);
    int run();
    bool event_register(int fd, unsigned int event);
    bool event_unregister(int fd, unsigned int event);

    inline int get_valid_Fd_Num()
    {
        return m_iValidFDNum;
    }
public:
    EpollInterface       *m_Thread; 
    int                  m_fEpollFD;
    unsigned int         m_iEventFD[EPOLL_MAX_FD];
    int                  m_iValidFDNum;
    
};

#endif
