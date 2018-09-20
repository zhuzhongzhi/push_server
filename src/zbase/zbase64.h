#ifndef __BASE_64_H__
#define __BASE_64_H__

class CBase64
{   
public:
    static int encode(const char *src, int src_length, char *dst, int &dst_length);
    static int decode(const char *src, int src_length, char *dst, int &dst_length);

private:
    static const char EncodeTable[];
    static const char DecodeTable[];    
};
#endif
