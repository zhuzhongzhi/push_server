/*
 * udp_server.cpp
 *
 *  Created on: 2016年3月1日
 *      Author: zzz
 */
#include "ace/OS.h"
#include "ace/INET_Addr.h"
#include "zudpserver.h"
#include "zlogger.h"
#include "zdispatch.h"

UDPServer::UDPServer ()
{
    // TODO Auto-generated constructor stub
    m_ServerPort = 0;
    m_ListenFd = -1;
    m_ThreadType = 0;
    m_StopFlag = false;
}

UDPServer::~UDPServer ()
{
    // TODO Auto-generated destructor stub
}

int UDPServer::init(const char* ip, u_short port, int type)
{
    m_ServerIp = ip;
    m_ServerPort = port;
    m_ThreadType = type;
    return 0;
}

int UDPServer::on_init()
{
    m_ListenFd = UDT::socket(AF_INET, SOCK_STREAM, 0);//创建socket
    if (-1 == m_ListenFd)
    {
        LOGWARN("udp server socket failed!");
        return -1;
    }

    ACE_INET_Addr serv_addr(m_ServerPort, m_ServerIp.c_str());
    if (UDT::bind(m_ListenFd,
                  reinterpret_cast<sockaddr *> (serv_addr.get_addr()),
                  serv_addr.get_size ()) == UDT::ERROR)
    {
        LOGWARN("udp server bind to [%s:%d] failed!", m_ServerIp.c_str(), m_ServerPort);
        return -1;
    }

    // 第二个参数 是TCP模块允许的已完成三次握手过程(TCP模块完成)但还没来得及被应用程序accept的最大链接数
    if (UDT::listen(m_ListenFd, UDP_LISTEN_NAN) < 0)
    {
        LOGWARN("udp server listen failed!");
        return -1;
    }

    LOGINFO("udp server[%s:%u] on init success!", m_ServerIp.c_str(), m_ServerPort);
    return 0;
}

int UDPServer::on_run()
{
    /* 循环监听 */
    struct sockaddr_in clnt_addr;
    int len = sizeof(clnt_addr);
    while (1)
    {
        UDTSOCKET peer_fd = UDT::accept(m_ListenFd, (struct sockaddr *)&clnt_addr, &len);/* 接收连接 */
        if (-1 == peer_fd)
        {
            LOGINFO("udp server on run accept peer fd is invalid!");
            continue;
        }

        if (!m_StopFlag)
        {
            LOGINFO("udp server dispatch connection[%d]!", peer_fd);

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
            LOGWARN("accept a udp connection, fd is %d, but udp server stop falg is true, not recept udp connections!", peer_fd);
        }
    }

    LOGINFO("udp server on run end!");
    return 0;
}

int UDPServer::on_exit()
{
    LOGINFO("udp server on exit!");
    if (ACE_INVALID_HANDLE != m_ListenFd)
    {
        LOGINFO("udp server on exit, colse socket[%d].", m_ListenFd);
        UDT::close(m_ListenFd);
        m_ListenFd = ACE_INVALID_HANDLE;
    }

    return 0;
}
