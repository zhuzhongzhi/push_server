#ifndef __APP_MANAGER_H__
#define __APP_MANAGER_H__

class TcpServer;
class AppManager
{
public:
    AppManager();
    virtual ~AppManager();

public:
    int init();

private:
    TcpServer*  m_pServer;
};

#endif