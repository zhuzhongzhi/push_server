#ifndef __UN_SUBSCRIBE_H__
#define __UN_SUBSCRIBE_H__

#include "zbasedef.h"
#include "zmqttdef.h"
#include "zmqttbase.h"


class MQTTUNSubscribe : public MQTTMessage
{
public:
    MQTTUNSubscribe();
    virtual ~MQTTUNSubscribe();
    
    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);

public:
    /**Variable header**/
    ushort                m_MessageID;

    /**Payload**/
    std::list<STLString>  m_Topics;
};

class MQTTUNSubAck : public MQTTMessage
{
public:
    MQTTUNSubAck();
    virtual ~MQTTUNSubAck();
    
    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);

public:
    /**Variable header**/
    unsigned short        m_MessageID;

    /**Payload**/
    /*There is no payload.*/
};


#endif
