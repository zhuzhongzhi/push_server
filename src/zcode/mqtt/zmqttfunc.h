#ifndef __PACKET_TOOLS_H__
#define __PACKET_TOOLS_H__

#include <string>
#if _MSC_VER
#include "winsock2.h"
#define snprintf _snprintf
#else
#include <netinet/in.h>
#endif
#include "zbasedef.h"

class MqttFunc
{
public:
    MqttFunc(){};
    virtual ~MqttFunc(){};
    
public:
    static int decode_variable_length(uchar* data, int length, int& bytes);
    static int encode_variable_length(int variable_length, uchar* data, int length, int& skip_bytes);

    static int decode_short(uchar* data, int length, ushort& out_short);    
    static int encode_short(ushort in_short, uchar* data, int length);
    
    static int decode_int(uchar* data, int length, uint& out_int);
    static int encode_int(uint in_int, uchar* data, int length);
    
    static int decode_long(uchar* data, int length, long long& out_long);
    static int encode_long(long long in_long, uchar* data, int length);

    static int decode_string(uchar* data, int length, std::string& out_string);    
    static int encode_string(const STLString& in_string, uchar* data, int length);
};
#endif
