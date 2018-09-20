#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "ace/Basic_Types.h"
#include "zbasedef.h"
#include "ztheadbase.h"

#define LISTEN_NAN 20
class TcpServer :public ThreadBase
{
public:
  TcpServer();
    virtual ~TcpServer();

    int init(const char* ip, u_short port, int type = 0);

public:
    virtual int on_init(); 
    virtual int on_run(); 
    virtual int on_exit();
    
private:
    STLString   m_ServerIp;
    u_short     m_ServerPort;
    int         m_ThreadType;
    ACE_HANDLE  m_ListenFd;
    bool        m_StopFlag;
};
#endif
