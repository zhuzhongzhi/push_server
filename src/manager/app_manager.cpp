#include "zdispatch.h"
#include "zlogger.h"
#include "zconfiger.h"
#include "ztcpserver.h"
#include "db_thread.h"
#include "service_thread.h"
#include "apns_manager.h"
#include "app_manager.h"


AppManager::AppManager():m_pServer(NULL)
{
    
}

AppManager::~AppManager()
{
    ZDELETE(m_pServer);
}

int AppManager::init()
{    
    LOGWARN("App Manager init start...");
    
    int thread_num = GET_CFG_ITEM(Service, ThreadNum);
    for (int i = 0; i < thread_num; ++i)
    {
        ServiceThread* svc = NULL;
        ZNEW(svc, ServiceThread);
        Dispatch::instance()->register_thread(svc, T_MQTT);
        if (0 != svc->init(i))
        {
            LOGWARN("Mqtt Thread NO.%d thread init [%s]", i, RED_TEXT(FAILED));
            return -1;
        }
        
        if (0 != svc->create_thread())
        {
            LOGWARN("Mqtt Thread Create NO.%d thread [%s]", i, RED_TEXT(FAILED));
            return -1;
        }

        LOGWARN("Mqtt Thread[%d] init start [%s].", i, GREEN_TEXT(OK));
    }

    int db_thread_num = GET_CFG_ITEM(Service, DBThreadNum);
    for (int j = 0; j < db_thread_num; ++j)
    {
        DBThread* db_thread = NULL;
        ZNEW(db_thread, DBThread);        
        Dispatch::instance()->register_thread(db_thread, T_DB);

        if (0 != db_thread->init(j))
        {
            LOGWARN("DB Thread NO.%d thread init [%s]", j, RED_TEXT(FAILED));
            return -1;
        }
        
        if (0 != db_thread->create_thread())
        {
            LOGWARN("DB Thread Create NO.%d thread [%s]", j, RED_TEXT(FAILED));
            return -1;
        }

        LOGWARN("DB Thread [%d] init start [%s].", j, GREEN_TEXT(OK));
    }

    int apns_thread_num = GET_CFG_ITEM(APNS, APNSThreadNum);
    for (int n = 0; n < apns_thread_num; ++n)
    {
        ApnsManager* apns_mgr = NULL;
        ZNEW(apns_mgr, ApnsManager);        
        Dispatch::instance()->register_thread(apns_mgr, T_APNS);
        if (0 != apns_mgr->init(n))
        {
            LOGWARN("APNS Manger NO.%d thread init [%s]", n, RED_TEXT(FAILED));
            return -1;
        }
        
        if (0 != apns_mgr->create_thread())
        {
            LOGWARN("APNS Create NO.%d thread [%s]", n, RED_TEXT(FAILED));
            return -1;
        }

        LOGWARN("APNS Manager[%d] init start [%s].", n, GREEN_TEXT(OK));
    }
    
    // create listing thread
    STLString local_ip = GET_CFG_ITEM(Common, LocalIp);
    int local_port = GET_CFG_ITEM(Common, LocalPort);
    ZNEW(m_pServer, TcpServer);
    if (0 != m_pServer->init(local_ip.c_str(), (u_short)local_port, T_MQTT))
    {
        LOGWARN("App Manger start server[%s:%d] thread [%s]", local_ip.c_str(), local_port, RED_TEXT(FAILED));
        return -1;
    }
    if (0 != m_pServer->create_thread())
    {
        LOGWARN("App Manger Create server[%s:%d] thread [%s]", local_ip.c_str(), local_port, RED_TEXT(FAILED));
        return -1;
    }
    LOGWARN("Listen[%s:%d] init start [%s].", local_ip.c_str(), local_port, GREEN_TEXT(OK));
    
    LOGWARN("App Manager init start [%s].", GREEN_TEXT(OK));
    
    return 0;
}


