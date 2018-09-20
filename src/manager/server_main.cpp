
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "ace/Reactor.h"
#include "ace/Sig_Handler.h"
#include "ace/TP_Reactor.h"
#include "ace/Timer_Queuefwd.h"
#include "ace/Timer_Wheel.h"
#include "app_manager.h"
#include "zbasedef.h"
#include "zconfiger.h"
#include "zsignalhandle.h"
#include "ztimerqueue.h"
#include "zlogger.h"
#include "mongo/client/dbclient.h"


ActiveTimer   g_Timer;

/**
main loop
*/

int svc()  
{   
    int nRet = 0;
    while (ACE_Reactor::instance()->reactor_event_loop_done() == 0)  
    {  
        ACE_Reactor::instance()->owner( ACE_OS::thr_self() );  
        nRet = ACE_Reactor::instance()->run_reactor_event_loop();  
        LOGINFO("ACE_Reactor run_reactor_event_loop result is %d.", nRet);
    }
    
    return 0;
}  

int main()
{
    // init mongo api
    mongo::client::initialize();

    /*
     * Lets get nice error messages
     */
    SSL_load_error_strings();
    ERR_load_BIO_strings();
    OpenSSL_add_all_algorithms();
    
    /*
     * Setup all the global SSL stuff
     */
    SSL_library_init();

    
    printf("Main start ...\n");
    g_Timer.activate();
    printf("Main ACE TIMER START [%s].\n", PRT_RED_TEXT(OK));
    
    char* pszWorkDir = getenv("WORK_DIR");
    if (NULL == pszWorkDir)
    {
        printf("Main getenv WORK_DIR [%s].\n", PRT_RED_TEXT(FAILED));
        return -1;
    }
    printf("Main getenv WORK_DIR [%s].\n", PRT_RED_TEXT(OK));

    const char* pszProcName = getenv("PROC_NAME");
    if (NULL == pszWorkDir)
    {
        printf("Main getenv PROC_NAME [%s]. Set PROC_NAME im_server.\n", PRT_RED_TEXT(FAILED));
        pszProcName = "push_server";
    }
    else
    {
        printf("Main getenv PROC_NAME [%s].\n", PRT_RED_TEXT(OK));
    }

    const char* pszProcVer = getenv("PROC_VER");
    if (NULL == pszProcVer)
    {
        printf("Main getenv PROC_VER [%s].\n", PRT_RED_TEXT(FAILED));
        pszProcVer = "version 1.1";
    }
    else
    {
        printf("Main getenv PROC_VER [%s].\n", PRT_RED_TEXT(OK));
    }
    
    Signal_Handle::sig_routine();
    printf("Main set signal handle [%s].\n", PRT_RED_TEXT(OK));    
    
    TLogInfo info;                                                   
    info.m_logPath      = pszWorkDir;                                        
    info.m_procName     = pszProcName;                                     
    info.m_procVer      = pszProcVer;
    printf("Main config init PATH[%s], NAME[%s], VERSION[%s].\n", 
            info.m_logPath.data(),
            info.m_procName.data(),
            info.m_procVer.data());
    Logger* logger = Logger::instance();
    if (0 != logger->init(info))                        
    {       
        printf("Main log init [%s].\n", PRT_RED_TEXT(FAILED));
        return -1;
    }

    if (0 != logger->create_thread())
    {
        printf("Main Create log thread [%s]", PRT_RED_TEXT(FAILED));
        return -1;
    }
    
    SET_DEBUG_LEVEL(DEBUG_LEVEL);
    printf("Main log init [%s].\n", PRT_RED_TEXT(OK));

    
    if (0 != Configer::instance()->init(pszWorkDir))
    {
        sleep(5);
        printf("Main config init [%s].\n", PRT_RED_TEXT(FAILED));
        return -1;
    }
    printf("Main config init [%s].\n", PRT_RED_TEXT(OK));

    logger->set_name_version(GET_CFG_ITEM(Common, ProcName).c_str(),
                             GET_CFG_ITEM(Common, Version).c_str());
    
    AppManager app;
    if (0 != app.init())
    {
        sleep(5);
        printf("Main app manager init [%s].\n", PRT_RED_TEXT(FAILED));
        return -1;
    }
    printf("Main app manager init [%s].\n", PRT_RED_TEXT(OK));

    printf("Main start [%s].\n", PRT_RED_TEXT(OK));

    SET_DEBUG_LEVEL(WARN_LEVEL);
      
    while (1)
    {
        svc();
        
        struct timeval timeWait;
        timeWait.tv_sec = 0;
        timeWait.tv_usec = 100;

        select(0, (fd_set*)0, (fd_set*)0, (fd_set*)0, &timeWait);
    }
    
    ACE_Reactor::instance()->close();
    
    return 0;
}
