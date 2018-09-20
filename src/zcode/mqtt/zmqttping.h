#ifndef __PING_MESSAGE_H__
#define __PING_MESSAGE_H__

#include "zmqttdef.h"
#include "zmqttbase.h"


class MQTTPingReq : public MQTTMessage
{
public:
    MQTTPingReq();
    virtual ~MQTTPingReq();

    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);
public:
    /**Variable header**/
    /*There is no variable header.*/

    /**Payload**/
    /*There is no payload.*/
};

class MQTTPingRsp : public MQTTMessage
{
public:
    MQTTPingRsp();
    virtual ~MQTTPingRsp();
    
    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);
public:
    /**Variable header**/
    /*There is no variable header.*/

    /**Payload**/
    /*There is no payload.*/
};

#endif
