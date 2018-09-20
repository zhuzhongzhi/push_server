#ifndef __SIGNAL_HANDLE_H__
#define __SIGNAL_HANDLE_H__

class Signal_Handle
{
public:
    Signal_Handle();
    virtual ~Signal_Handle();

public:
    static void sig_routine();
    static void sig_hup(int dunno);
    static void sig_int(int dunno);
    static void sig_quit(int dunno);
    static void sig_term(int dunno);
    static void sig_usr1(int dunno);
    static void sig_usr2(int dunno);
    static void sig_pipe(int dunno);
};

#endif
