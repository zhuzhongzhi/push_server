
#include <signal.h>
#include "zlogger.h"
#include "zsignalhandle.h"

Signal_Handle::Signal_Handle()
{
}

Signal_Handle::~Signal_Handle()
{
}

void Signal_Handle::sig_routine()
{
    signal(SIGHUP, sig_hup);
    signal(SIGINT, sig_int);
    signal(SIGTERM, sig_term);
    signal(SIGQUIT, sig_quit);
    signal(SIGUSR1, sig_usr1);
    signal(SIGUSR2, sig_usr2);
    signal(SIGPIPE, sig_pipe);
}

void Signal_Handle::sig_hup(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGHUP!");
}

void Signal_Handle::sig_int(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGINT!");
}

void Signal_Handle::sig_quit(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGQUIT!");
}

void Signal_Handle::sig_term(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGTERM!");
}

void Signal_Handle::sig_usr1(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGUSR1!");
    SET_DEBUG_LEVEL(DEBUG_LEVEL);
    LOGINFO("Recv USR1 signal open trace level to debug.");
}

void Signal_Handle::sig_usr2(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGUSR2!");
    SET_DEBUG_LEVEL(WARN_LEVEL);
    LOGINFO("Recv USR2 signal open trace level to warning.");
}

void Signal_Handle::sig_pipe(int dunno)
{
    LOGINFO("Signal_Handle got a signal SIGPIPE!");
}


