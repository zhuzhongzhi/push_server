#ifndef __PUBLISH_MESSAGE_H__
#define __PUBLISH_MESSAGE_H__
#include <time.h>
#include <string>
#include "zmqttdef.h"
#include "zmqttbase.h"

//Publish message
/***
PUBLISH messages can be sent either from a publisher to the server, or from the server to a subscriber. The action of 
the recipient when it receives a message depends on the QoS level of the message:
QoS 0
    Make the message available to any interested parties.
QoS 1
    Log the message to persistent storage, make it available to any interested parties, and return a PUBACK message 
to the sender.
QoS 2
    Log the message to persistent storage, do not make it available to interested parties yet, and return a PUBREC 
message to the sender. 

**/
class MQTTPublish : public MQTTMessage
{
public:
    MQTTPublish();
    virtual ~MQTTPublish();

    MQTTPublish *clone();
    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
    void replace_message_id(ushort mid);
    void replace_time_stamp();

    virtual time_t message_time() const
    {
        return m_TimeStamp;
    }
public:
    /**Variable header**/
    STLString             m_TopicName;
    ushort                m_RecvMessageID;
    ushort                m_SendMessageID;
    
    /**Payload**/
    time_t                m_TimeStamp;
    UserIDType            m_SenderID;
    UserIDType            m_ReceiverID;
    ushort                m_MessageType;
    ushort                m_ReceiverType;
    STLString             m_MultReceivers;
    uchar*                m_PublishData;
    int                   m_PublishDataLength;

    int                   m_SessionFD;
    STLString             m_SenderName;
    STLString             m_GroupName;
};

//Publish acknowledgment
/**
When the client receives the PUBACK message, it discards the original message, because it is also received (and logged
) by the server.
**/
class MQTTPubAck : public MQTTMessage
{
public:
    MQTTPubAck();
    virtual ~MQTTPubAck();

    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
public:
    /**Variable header**/
    ushort               m_MessageID;

    /**Payload**/
    /*There is no payload.*/
};

//Assured publish received
/**
When it receives a PUBREC message, the recipient sends a PUBREL message to the sender with the same Message ID as the 
PUBREC message.
**/
class MQTTPubRec : public MQTTMessage
{
public:
    MQTTPubRec();
    virtual ~MQTTPubRec();

    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
public:
    /**Variable header**/
    ushort              m_MessageID;

    /**Payload**/
    /*There is no payload.*/
};

//Assured Publish Release
/**
When the server receives a PUBREL message from a publisher, the server makes the original message available to 
interested subscribers, and sends a PUBCOMP message with the same Message ID to the publisher. When a subscriber 
receives a PUBREL message from the server, the subscriber makes the message available to the subscribing application 
and sends a PUBCOMP message to the server.
**/
class MQTTPubRel : public MQTTMessage
{
public:
    MQTTPubRel();
    virtual ~MQTTPubRel();

    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);
    
public:
    /**Variable header**/
    ushort             m_MessageID;

    /**Payload**/
    /*There is no payload.*/
};

//Assured publish complete
/**
When the client receives a PUBCOMP message, it discards the original message because it has been delivered, exactly 
once, to the server.
**/
class MQTTPubComp : public MQTTMessage
{
public:
    MQTTPubComp();
    virtual ~MQTTPubComp();

    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);
public:
    /**Variable header**/
    ushort             m_MessageID;

    /**Payload**/
    /*There is no payload.*/
};


#endif
