
#ifndef _Z_BASE_DEF_H_
#define _Z_BASE_DEF_H_

#include <assert.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include <ctype.h>
#include <string.h>

#include <string>
#include <list>
#include <vector>
#include <map>

#include "ace/Event_Handler.h"
#include "zstring.h"

// macros
#define DEF_COPY_AND_ASSIGN(className)                                          \
            className(const className& );                                       \
            className& operator = (const className& );

#define SAFE_STATIC         static __thread 

#define ZMin(a, b)          ((a) > (b) ? (b) : (a))
#define ZMax(a, b)          ((a) > (b) ? (a) : (b))

#define     CLOSE_FD(fd)                                        \
                do                                              \
                {                                               \
                    if (-1 != fd)                               \
                    {                                           \
                        ::close(fd);                            \
                        fd = -1;                                \
                    }                                           \
                }while(0)

#define  FORMAT_CHECK(type, fmtIndex, firstCheck) __attribute__((format(type, fmtIndex, firstCheck)))

#define  INLINE inline
#define  ASSERT assert

#ifdef _DARWIN
    #define EPOLLIN  1
    #define EPOLLOUT  4
#endif

#define _RED_TEXT(STR)     "\033[47;31m"#STR"\033[0m"
#define _GREEN_TEXT(STR)   "\033[47;32m"#STR"\033[0m"
#define _TO_STRING(STR)    #STR

#ifdef DEBUG_WITH_COLOUR
    #define RED_TEXT(STR)     _RED_TEXT(STR)
    #define GREEN_TEXT(STR)   _GREEN_TEXT(STR)
#else
    #define RED_TEXT(STR)     #STR
    #define GREEN_TEXT(STR)   #STR
#endif
#define PRT_RED_TEXT(STR)     _RED_TEXT(STR)
#define PRT_GREEN_TEXT(STR)   _GREEN_TEXT(STR)

// memory macros define
#define ZADD_MEM(p)
#define ZDEL_MEM(p)

#define ZCHECK_NEW_PRT(p)                                                                    \
            {                                                                               \
                if (NULL == p)                                                              \
                {                                                                           \
                    printf("Failed to malloc memory, exit![%s:%d]\n", __FILE__, __LINE__);  \
                    abort();                                                                \
                }                                                                           \
                else                                                                        \
                {                                                                           \
                    ZADD_MEM(p);                                                             \
                }                                                                           \
            }

#define ZNEW(p, type)                                                                        \
            {                                                                               \
                try                                                                         \
                {                                                                           \
                    p = new type;                                                           \
                }                                                                           \
                catch(...)                                                                  \
                {                                                                           \
                    p = NULL;                                                               \
                }                                                                           \
                ZCHECK_NEW_PRT(p)                                                            \
            }

#define ZNEW_S(p, type, size)                                                                \
            {                                                                               \
                try                                                                         \
                {                                                                           \
                    p = new type[size];                                                     \
                }                                                                           \
                catch(...)                                                                  \
                {                                                                           \
                    p = NULL;                                                               \
                }                                                                           \
                ZCHECK_NEW_PRT(p)                                                           \
            }

#define ZNEW_T(p, basetype, type)                                                           \
            {                                                                               \
                try                                                                         \
                {                                                                           \
                    p = (basetype *)new type;                                               \
                }                                                                           \
                catch(...)                                                                  \
                {                                                                           \
                    p = NULL;                                                               \
                }                                                                           \
                ZCHECK_NEW_PRT(p)                                                           \
            }

#define ZDELETE(p)                                                                          \
            {                                                                               \
                if (NULL != p)                                                              \
                {                                                                           \
                    ZDEL_MEM(p);                                                            \
                    delete p;                                                               \
                    p = NULL;                                                               \
                }                                                                           \
            }
                    
#define ZDELETE_A(p)                                                                        \
            {                                                                               \
                if (NULL != p)                                                              \
                {                                                                           \
                    ZDEL_MEM(p);                                                            \
                    delete [] p;                                                            \
                    p = NULL;                                                               \
                }                                                                           \
            }

#define ZREF(N)   do\
            {\
                N->reference();\
            }while(0)

#define ZRENL(N)   do\
            {\
                N->release();\
                N = NULL;\
            } while(0)

#define ZREL(N)   do\
        {\
            if (N != NULL)\
            {\
                N->release();\
                if (N->m_RefNum <= 0)\
                {\
                    ZDELETE(N);\
                }\
            }\
        }while(0)
        

// const define
const int       MAX_PROC_LEN                    = 32;           //max precess name length
const int       MAX_VERSION_LEN                 = 32;           //max version length
const int       MAX_PATH_LEN                    = 256;          //max path length
const int       MAX_FILE_LEN                    = 256;          //max file name length
const int       MIN_BUFF_LEN                    = 128;          //min buffer length
const int       MID_BUFF_LEN                    = 256;          //middle buffer length
const int       MAX_BUFF_LEN                    = 512;          //max buffer length
const int       HUG_BUFF_LEN                    = 1024;         //huge buffer length
const int       BASE_YEAR                       = 1900;        
const int       BASE_MONTH                      = 1;


// typedef
typedef         std::string                     STLString;
typedef         long long                       UserIDType;
typedef         XString<MAX_PROC_LEN>           TProcName;
typedef         XString<MAX_VERSION_LEN>        TProcVer;
typedef         XString<MAX_PATH_LEN>           TPath;
typedef         XString<MAX_FILE_LEN>           TFileName;
typedef         XString<MIN_BUFF_LEN>           TMinBuff;
typedef         XString<MID_BUFF_LEN>           TMidBuff;
typedef         XString<MAX_BUFF_LEN>           TMaxBuff;
typedef         XString<HUG_BUFF_LEN>           THugBuff;

typedef         unsigned char                   uchar;
typedef         unsigned short                  ushort;
typedef         unsigned int                    uint;
typedef         unsigned long                   ulong;
typedef         unsigned long long              ulonglong;

typedef ACE_Event_Handler TQHandler;

#endif // _Z_BASE_DEF_H_
