
#include <string.h>
#include "zbase64.h"

const char CBase64::EncodeTable[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

const char CBase64::DecodeTable[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    62, // '+'
    0, 0, 0,
    63, // '/'
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
    0, 0, 0, 0, 0, 0, 0,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
    13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
    0, 0, 0, 0, 0, 0,
    26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
    39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
};

int CBase64::encode(const char *src, int src_length, 
                    char *dst, int &dst_length)
{
    unsigned char Tmp[4] = {0};
    int LineLength=0;
    int pos = 0;
    int cycle = (src_length / 3);
    for (int i = 0; i < cycle; ++i)
    {
        memcpy(Tmp + 1, src + 3*i, 3);        
        dst[pos] = EncodeTable[Tmp[1] >> 2];
        dst[pos+1] = EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        dst[pos+2] = EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        dst[pos+3] = EncodeTable[Tmp[3] & 0x3F];
        pos += 4;
        if (LineLength += 4, LineLength == 76) 
        {
            memcpy(dst + pos, "\r\n", 2); 
            pos += 2;
            LineLength = 0;
        }
    }
    
    int Mod = src_length % 3;
    if (1 == Mod)
    {
        Tmp[1] = src[src_length - 1];
        dst[pos] = EncodeTable[(Tmp[1] & 0xFC) >> 2];
        dst[pos + 1] = EncodeTable[((Tmp[1] & 0x03) << 4)];
        dst[pos + 2] = '=';
        dst[pos + 3] = '=';
        pos += 4;
    }
    else if(2 == Mod)
    {
        Tmp[1] = src[src_length - 2];
        Tmp[2] = src[src_length - 1];
        dst[pos] = EncodeTable[(Tmp[1] & 0xFC) >> 2];
        dst[pos + 1] = EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        dst[pos + 2] = EncodeTable[((Tmp[2] & 0x0F) << 2)];
        dst[pos + 3] = '=';
        pos += 4;
    }

    dst_length = pos;
    return 0;
}

int CBase64::decode(const char *src, int src_length, 
                      char *dst, int &dst_length)
{
    int nValue;
    int i= 0;
    int pos = 0;
    while (i < src_length)
    {
        if ((src[i] != '\r') && (src[i] != '\n'))
        {
            nValue = DecodeTable[src[i]] << 18;
            nValue += DecodeTable[src[i + 1]] << 12;
            dst[pos] = (nValue & 0x00FF0000) >> 16;
            pos++;
            
            if (src[i + 2] != '=')
            {
                nValue += DecodeTable[src[i + 2]] << 6;
                dst[pos] =(nValue & 0x0000FF00) >> 8;
                pos++;
                
                if (src[i + 3] != '=')
                {
                    nValue += DecodeTable[src[i + 3]];
                    dst[pos] = nValue & 0x000000FF;
                    pos++;
                }
            }
            i += 4;
        }
        else
        {
            pos++;
            i++;
        }
    }

    dst_length = pos;
    return 0;
}


