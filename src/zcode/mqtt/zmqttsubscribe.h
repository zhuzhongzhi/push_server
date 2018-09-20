#ifndef __SUBSCRIBE_MESSAGE_H__
#define __SUBSCRIBE_MESSAGE_H__

#include <list>
#include <vector>
#include "zmqttdef.h"
#include "zmqttbase.h"

struct TSubTopic
{
    STLString   m_TopicName;
    char        m_Qos;
};

class MQTTSubscribe : public MQTTMessage
{
public:
    MQTTSubscribe();
    virtual ~MQTTSubscribe();

    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
public:
    /**Variable header**/
    unsigned short         m_MessageID;

    /**Payload**/
    std::list<TSubTopic>   m_SubTopics;
};

class MQTTSubAck : public MQTTMessage
{
public:
    MQTTSubAck();
    virtual ~MQTTSubAck();

    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
public:
    /**Variable header**/
    unsigned short                m_MessageID;

    /**Payload**/
    std::vector<uchar>            m_GrantedQos;
};

#endif