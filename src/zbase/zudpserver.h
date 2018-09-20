/*
 * udp_server.h
 *
 *  Created on: 2016年3月1日
 *      Author: zzz
 */

#ifndef SRC_SERVER_UDP_SERVER_H_
#define SRC_SERVER_UDP_SERVER_H_

#include "zbasedef.h"
#include "ztheadbase.h"
#include "udt.h"

#define UDP_LISTEN_NAN 10

class UDPServer :public ThreadBase
{
public:
  UDPServer ();
  virtual
  ~UDPServer ();


  int init(const char* ip, u_short port, int type = 0);

public:
  virtual int on_init(); //线程启动之后初始化
  virtual int on_run(); //线程主循环
  virtual int on_exit(); //线程退出处理

private:
  STLString   m_ServerIp;
  u_short     m_ServerPort;
  int         m_ThreadType;
  int         m_ListenFd;
  bool        m_StopFlag;

};

#endif /* SRC_SERVER_UDP_SERVER_H_ */
