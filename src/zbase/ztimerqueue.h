#ifndef __TIMER_QUEUE_H__
#define __TIMER_QUEUE_H__

#include "ace/Timer_Wheel.h"
#include "ace/Timer_Queue_Adapters.h"

typedef ACE_Timer_Wheel   Timer_Queue;
typedef ACE_Timer_Wheel_Iterator  Timer_Queue_Iterator;

typedef ACE_Thread_Timer_Queue_Adapter<ACE_Timer_Wheel> ActiveTimer; 
extern ActiveTimer   g_Timer;

#define START_THREAD_TIMER  g_Timer.activate()

#define SET_THREAD_TIMER(FUTURE, INTERVAL, ID)   \
 do\
 {\
    ACE_Time_Value first (0, FUTURE);\
    ACE_Time_Value tv (0, INTERVAL);\
    ID = g_Timer.schedule (this,\
                           NULL,\
                           ACE_OS::gettimeofday () + first, tv);\
  } while(0);

#define SET_QUEUE_TIMER(FUTURE, INTERVAL, HANDLE, ID, ARG)   \
 do\
 {\
    ACE_Time_Value first (FUTURE);\
    ACE_Time_Value tv (INTERVAL);\
    ID = g_Timer.schedule (HANDLE,\
                           (void*)ARG,\
                           ACE_OS::gettimeofday () + first, tv);\
  } while(0);
    
#endif
