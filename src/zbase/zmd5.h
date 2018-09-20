#ifndef __MD5__INCLUDE__H__
#define __MD5__INCLUDE__H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define _LENGTH_MD5_    16

typedef unsigned long   md5_UINT4;

typedef struct
{
    md5_UINT4   state[4];
    md5_UINT4   bitcount[2];
    unsigned char   buffer[64];
} MD5_context;

void    MD5_init(MD5_context *_md5_context);
void    MD5_update(MD5_context *_md5_context,unsigned char *_buffer,int _lenth);
void    MD5_final(MD5_context *_md5_context,unsigned char *_digest);
int MD5_hmac(char *_input,int _length,char *_out_md5_hmac);
char*   _md5_public_KEY_string_(void);

#define MD5_BEGIN(_out_MD5)                     \
    {                               \
        char        *_A_str;                \
        unsigned char   _A_dig[64];             \
        char        _t_str[4];              \
        int     _n=0;                   \
        MD5_context _A_ctx;                 \
                                    \
        if(sizeof(_out_MD5)>4)                  \
            memset(_out_MD5,0,sizeof(_out_MD5));        \
        _A_str=(char *)_out_MD5;                \
        memset(_A_dig,0,sizeof(_A_dig));            \
        memset(&_A_ctx,0,sizeof(MD5_context));          \
        MD5_init(&_A_ctx);
#define MD5_string(_str_UPDATE)                     \
        if(strlen(_str_UPDATE)>0)               \
            MD5_update(&_A_ctx,(unsigned char*)_str_UPDATE, \
                strlen(_str_UPDATE));
#define MD5_binary(_str_UPDATE,_str_LENGTH)             \
        if(_str_LENGTH>0)                   \
            MD5_update(&_A_ctx,(unsigned char*)_str_UPDATE, \
                _str_LENGTH);
#define MD5_int(_int_UPDATE)                        \
        {                           \
            char    _int_STRING[32];            \
                                    \
            memset(_int_STRING,0,sizeof(_int_STRING));  \
            sprintf(_int_STRING,"%d%X",         \
                _int_UPDATE,_int_UPDATE);       \
            MD5_string(_int_STRING);            \
        }

#define MD5_smallint    MD5_int
#define MD5_short   MD5_int
#define MD5_char    MD5_int

#define MD5_uint    MD5_int
#define MD5_ushort  MD5_int
#define MD5_uchar   MD5_int

#define MD5_long(_long_UPDATE)                      \
        {                           \
            char    _long_STRING[64];           \
                                    \
            memset(_long_STRING,0,sizeof(_long_STRING));    \
            sprintf(_long_STRING,"%ld%08lX",        \
                _long_UPDATE,_long_UPDATE);     \
            MD5_string(_long_STRING);           \
        }
#define MD5_ulong   MD5_long

#define MD5_END                             \
        MD5_final(&_A_ctx,_A_dig);              \
        for(_n=0;_n<_LENGTH_MD5_;_n++)              \
        {                           \
            sprintf(_t_str,"%02X",(unsigned char)_A_dig[_n]);\
            strcat(_A_str,_t_str);              \
        }                           \
    }
#define MD5_begin(_out_MD5)                     \
    {                               \
        char        *_A_str;                \
        unsigned char   _A_dig[64];             \
        MD5_context _A_ctx;                 \
                                    \
        if(sizeof(_out_MD5)>4)                  \
            memset(_out_MD5,0,sizeof(_out_MD5));        \
        _A_str=(char *)_out_MD5;                \
        memset(_A_dig,0,sizeof(_A_dig));            \
        memset(&_A_ctx,0,sizeof(MD5_context));          \
        MD5_init(&_A_ctx);
#define MD5_end                             \
        MD5_final(&_A_ctx,_A_dig);              \
        memcpy(_A_str,_A_dig,_LENGTH_MD5_);         \
    }

#define public_KEY  _md5_public_KEY_string_()

#endif

