
#include "ace/OS.h"
#include "ace/INET_Addr.h"
#include "zdispatch.h"
#include "znlmessage.h"
#include "zlogger.h"
#include "ztcpserver.h"

TcpServer::TcpServer()
{
    m_ServerPort = 0;
    m_ListenFd = ACE_INVALID_HANDLE;
    m_ThreadType = 0;
    m_StopFlag = false;
}

TcpServer::~TcpServer()
{
}

int TcpServer::init(const char* ip, u_short port, int type)
{
    m_ServerIp = ip;
    m_ServerPort = port;
    m_ThreadType = type;
    return 0;
}


int TcpServer::on_init()
{    
    m_ListenFd = ACE_OS::socket(AF_INET, SOCK_STREAM, 0);//创建socket
    if (ACE_INVALID_HANDLE == m_ListenFd)
    {
        LOGWARN(" tcp server socket failed!");
        return -1;
    }

    // 设置reuse
    int one = 1;
    if (ACE_OS::setsockopt (m_ListenFd,
                            SOL_SOCKET,
                            SO_REUSEADDR,
                            (const char*) &one,
                            sizeof one) == -1)
    {
        LOGWARN("tcp server socket set SO_REUSEADDR failed!");
        return -1;
    }
    
    ACE_INET_Addr serv_addr(m_ServerPort, m_ServerIp.c_str());
    if (ACE_OS::bind(m_ListenFd,
                     reinterpret_cast<sockaddr *> (serv_addr.get_addr()),
                     serv_addr.get_size ()) < 0) 
    {
        LOGWARN("tcp server bind to [%s:%d] failed!", m_ServerIp.c_str(), m_ServerPort);
        return -1;
    }

    // 第二个参数 是TCP模块允许的已完成三次握手过程(TCP模块完成)但还没来得及被应用程序accept的最大链接数
    if (ACE_OS::listen(m_ListenFd, LISTEN_NAN) < 0)
    {
        LOGWARN("tcp server listen failed!");
        return -1;
    }

    LOGINFO("tcp server[%s:%u] on init success!", m_ServerIp.c_str(), m_ServerPort);
    return 0;    
}

int TcpServer::on_run()
{
    /* 循环监听 */
    struct sockaddr_in clnt_addr;
    int len = sizeof(clnt_addr);
    while (1) 
    {
        ACE_HANDLE peer_fd = ACE_OS::accept(m_ListenFd, (struct sockaddr *)&clnt_addr, &len);/* 接收连接 */
        if (ACE_INVALID_HANDLE == peer_fd) 
        {
            LOGINFO("tcp server on run accept peer fd is invalid!");
            continue;
        }
        
        if (!m_StopFlag)
        {
            LOGINFO("tcp server dispatch connection[%d]!", peer_fd);

            ConnectMsg * msg = NULL;
            try
            {
                msg = new ConnectMsg();
            }
            catch(...)
            {
                msg = NULL;
            }
            if (NULL != msg)
            {
                msg->m_PeerFd = peer_fd;
                if (0 != Dispatch::instance()->post_message(msg, m_ThreadType))
                {
                    delete msg;
                    msg = NULL;
                }
            }
            else
            {
                delete msg;
                msg = NULL;
            }
            
        }
        else
        {
            LOGWARN("accept a tcp connection, fd is %d, but tcp server stop falg is true, not recept tcp connections!", peer_fd);
        }
    }

    LOGINFO("tcp server on run end!");
    return 0;
}

int TcpServer::on_exit()
{
    LOGINFO("tcp server on exit!");
    if (ACE_INVALID_HANDLE != m_ListenFd)
    {
        LOGINFO("tcp server on exit, colse socket[%d].", m_ListenFd);
        ACE_OS::closesocket(m_ListenFd);
        m_ListenFd = ACE_INVALID_HANDLE;
    }

    return 0;
}

