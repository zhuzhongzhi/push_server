
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "zlogger.h"
#include "zmqttfunc.h"

int MqttFunc::decode_variable_length(uchar* data, int length, int& bytes)
{
    int variable_length = 0;
    int multiplier = 1;
    bytes = -1;
    do 
    {
        ++bytes;
        if (bytes >= length)
        {
            LOGWARN("Bytes is %d, length is %d, need more data.", bytes, length);
            return -1;
        }
        variable_length += (data[bytes] & 127) * multiplier;
        multiplier *= 128;      
    } while ((data[bytes] & 128) != 0);

    bytes ++;
    
    return variable_length;
}

int MqttFunc::encode_variable_length(int variable_length, uchar* data, int length, int& skip_bytes)
{
    skip_bytes = 0;
    uchar byte;
    do
    {
        if (skip_bytes >= length)
        {
            // buffer
            LOGWARN("skip_bytes is %d, length is %d, not enough buffer.", skip_bytes, length);
            return -1;
        }
        
        byte = variable_length % 128;
        variable_length = variable_length / 128;
        /* If there are more digits to encode, set the top bit of this digit */
        if(variable_length > 0)
        {
            byte = byte | 0x80;
        }
        data[skip_bytes] = byte;
        skip_bytes++;
    } while(variable_length > 0 && skip_bytes < 5);

    return 0;
}


int MqttFunc::decode_short(uchar* data, int length, ushort& out_short)
{
    if (length < sizeof(ushort))
    {
        LOGWARN("length is %d < sizeof(ushort), need more data.", length);
        return -1;
    }
    out_short = ((ushort)(data[0]) << 8) + (ushort)(data[1]);
    return 0;
}

int MqttFunc::encode_short(ushort in_short, uchar* data, int length)
{
    data[0] = (uchar)(in_short >> 8);
    data[1] = (uchar)(in_short & 0x00FF);
    return 0;
}

int MqttFunc::decode_int(uchar* data, int length, uint& out_int)
{
    if (length < sizeof(uint))
    {
        LOGWARN("length is %d < sizeof(uint), need more data.",
        		length);

        return -1;
    }
    out_int = ((uint)(data[0]) << 24)
            + ((uint)(data[1]) << 16)
            + ((uint)(data[2]) << 8)
            + (uint)(data[3]);
    return 0;
}

int MqttFunc::encode_int(uint in_int, uchar* data, int length)
{
    data[0] = (uchar)(in_int >> 24);
    data[1] = (uchar)(in_int >> 16);
    data[2] = (uchar)(in_int >> 8);
    data[3] = (uchar)(in_int & 0x00FF);
    return 0;
}

int MqttFunc::decode_long(uchar* data, int length, long long& out_long)
{
    if (length < sizeof(long long))
    {
        LOGWARN("length is %d < sizeof(long long), need more data.",
                length);
        return -1;
    }
    out_long = ((ulonglong)(data[0]) << 56)
             + ((ulonglong)(data[1]) << 48)
             + ((ulonglong)(data[2]) << 40)
             + ((ulonglong)(data[3]) << 32)
             + ((ulonglong)(data[4]) << 24)
             + ((ulonglong)(data[5]) << 16)
             + ((ulonglong)(data[6]) << 8)
             + (ulonglong)(data[7]);
    return 0;
}

int MqttFunc::encode_long(long long in_long, uchar* data, int length)
{
    data[0] = (uchar)(in_long >> 56);
    data[1] = (uchar)(in_long >> 48);
    data[2] = (uchar)(in_long >> 40);
    data[3] = (uchar)(in_long >> 32);
    data[4] = (uchar)(in_long >> 24);
	data[5] = (uchar)(in_long >> 16);
	data[6] = (uchar)(in_long >> 8);
	data[7] = (uchar)(in_long & 0x00FF);
    return 0;
}

int MqttFunc::decode_string(uchar* data, int length, STLString& out_string)
{
    ushort string_length = 0;
    if (0 != decode_short(data, length, string_length))
    {
        LOGWARN("decode short failed, need more data.");
        return -1;
    }
    out_string.clear();
    out_string.append((const char*)(data + sizeof(ushort)), string_length);

    return 0;
}

int MqttFunc::encode_string(const STLString& in_string, uchar* data, int length)
{
    ushort string_length = in_string.length();
    if (length <  (int)(string_length + sizeof(ushort)))
    {
        LOGWARN("decode string failed, length is %d, string length is %d, not enough buffer.",
                length, in_string.length());
        return -1;
    }
    
    encode_short(string_length, data, length);
    memcpy(data + sizeof(ushort), in_string.c_str(), string_length);
    return 0;
}




