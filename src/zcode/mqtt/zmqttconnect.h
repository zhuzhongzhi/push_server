#ifndef __CONNECT_MESSAGE_H__
#define __CONNECT_MESSAGE_H__

#include "zbasedef.h"
#include "zmqttdef.h"
#include "zmqttbase.h"

class MQTTConnect : public MQTTMessage
{
public:
    MQTTConnect();
    virtual ~MQTTConnect();
    
    virtual int encode(uchar *data, int length, int& skip_bytes);
    virtual int decode(uchar *data, int length, int& skip_bytes);

public:
    /**Variable header**/
    STLString       m_ProtocolName;
    uchar           m_ProtocolVersionNumber;
    TConnectFlags   m_ConnectFlags;
    ushort          m_KeepAlivetimer;

    /**Payload**/
    STLString       m_ClientIdentifier;
    STLString       m_WillTopic;
    STLString       m_WillMessage;
    STLString       m_UserName;
    STLString       m_Password;
};

class MQTTConnAck : public MQTTMessage
{
public:
    MQTTConnAck();
    virtual ~MQTTConnAck();

    
    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);

public:
    /**Variable header**/
    uchar   m_TopicNameCompressionResponse; //Reserved values. Not used.
    uchar   m_ConnectReturnCode;

    /**Payload**/
    /*There is no payload.*/
};


class MQTTDisConnect : public MQTTMessage
{
public:
    MQTTDisConnect();
    virtual ~MQTTDisConnect();

    
    virtual int encode(uchar *data, int length, int& skip_bytes);    
    virtual int decode(uchar *data, int length, int& skip_bytes);

public:
    /**Variable header**/
    /*There is no variable header.*/

    /**Payload**/
    /*There is no payload.*/
};

#endif
