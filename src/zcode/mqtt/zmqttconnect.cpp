
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmqttconnect.h"

MQTTConnect::MQTTConnect()
{
    m_FixedHdr.m_MessageType = CONNECT;
    m_ProtocolName = "MQIsdp";
    m_ConnectFlags.m_UserNameFlag = 1;
    m_ConnectFlags.m_Reserved = 0;
    m_ConnectFlags.m_WillFlag = 1;
    m_ConnectFlags.m_WillRetain = 0;
    m_ConnectFlags.m_WillQos = 1;
    m_KeepAlivetimer = 10;
}

MQTTConnect::~MQTTConnect()
{
}

int MQTTConnect::encode(uchar *data, int length, int& skip_bytes)
{
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    if (!m_ProtocolName.empty())
    {        
        if (0 != MqttFunc::encode_string(m_ProtocolName, start, length_left))
        {
            LOGWARN("Connect Encode protocol name failed!");
            return CODE_NOT_ENOUGH_BUFFER;
        }
        skip = m_ProtocolName.length() + sizeof(ushort);
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }
    else
    {
        MqttFunc::encode_short(0, start, length_left);
        skip = sizeof(ushort);
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }

    start[0] = m_ProtocolVersionNumber;
    skip = 1;
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    start[0] = *((uchar *)&m_ConnectFlags);
    skip = 1;
    start += skip;
    length_left -= skip;
    skip_bytes += skip;
    
    MqttFunc::encode_short(m_KeepAlivetimer, start, length_left);
    skip = sizeof(ushort);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;
    
    if (!m_ClientIdentifier.empty())
    {        
        if (0 != MqttFunc::encode_string(m_ClientIdentifier, start, length_left))
        {
            LOGWARN("Connect Encode client identifier failed!");
            return CODE_NOT_ENOUGH_BUFFER;
        }
        skip = m_ClientIdentifier.length() + sizeof(ushort);
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }
    else
    {
        MqttFunc::encode_short(0, start, length_left);
        skip = sizeof(ushort);
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }

    if (m_ConnectFlags.m_WillFlag)
    {
        if (!m_WillTopic.empty())
        {        
            if (0 != MqttFunc::encode_string(m_WillTopic, start, length_left))
            {
                LOGWARN("Connect Encode will topic failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_WillTopic.length() + sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }        
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }

        if (!m_WillMessage.empty())
        {        
            if (0 != MqttFunc::encode_string(m_WillMessage, start, length_left))
            {
                LOGWARN("Connect Encode will message failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_WillMessage.length() + sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
    }

    
    if (m_ConnectFlags.m_UserNameFlag)
    {
        if (!m_UserName.empty())
        {        
            if (0 != MqttFunc::encode_string(m_UserName, start, length_left))
            {
                LOGWARN("Connect Encode user name failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_UserName.length() + sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
    }

    
    if (m_ConnectFlags.m_PasswordFlag)
    {
        if (!m_Password.empty())
        {        
            if (0 != MqttFunc::encode_string(m_Password, start, length_left))
            {
                LOGWARN("Connect Encode password failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_Password.length() + sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(ushort);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
    }

    return CODE_OK;
}

int MQTTConnect::decode(uchar *data, int length, int& skip_bytes)
{
    uchar *start = data;
    int length_left = length;

    // decode Protocol Name
    m_ProtocolName.clear();
    if (0 != MqttFunc::decode_string(start, length_left, m_ProtocolName))
    {
        LOGWARN("decode Protocol Name failed!");
        return CODE_FAILED;
    }    
    skip_bytes = m_ProtocolName.length() + sizeof(ushort);
    start = data + skip_bytes;
    //length_left = length - skip_bytes;
    LOGINFO("Protocol Name is %s.", m_ProtocolName.c_str());

    // decode Protocol Version Number or Protocol Level
    m_ProtocolVersionNumber = *start;
    skip_bytes += sizeof(uchar);
    start = data + skip_bytes;
    //length_left = length - skip_bytes;
    LOGINFO("ProtocolVersionNumber is %d.", m_ProtocolVersionNumber);

    // decode Connect Flags
    memcpy(&m_ConnectFlags, start, 1);
    skip_bytes += 1;
    start = data + skip_bytes;
    length_left = length - skip_bytes;
    LOGINFO("user_name_flag is %d, password_flag is %d, will_retain is %d, will_qos is %d, "
        "will_flag is %d, clean_session is %d", 
        m_ConnectFlags.m_UserNameFlag,
        m_ConnectFlags.m_PasswordFlag,
        m_ConnectFlags.m_WillRetain,
        m_ConnectFlags.m_WillQos,
        m_ConnectFlags.m_WillFlag,
        m_ConnectFlags.m_CleanSession);
    
    // decode Keep Alive timer
    if (0 != MqttFunc::decode_short(start, length_left, m_KeepAlivetimer))
    {
        LOGWARN("decode Keep Alive timer failed.");
        return CODE_FAILED;
    }
    LOGINFO("KeepAlivetimer is %d.", m_KeepAlivetimer);
    
    skip_bytes += sizeof(ushort);
    start = data + skip_bytes;
    length_left = length - skip_bytes;
    
    // decode Client Identifier
    if (0 != MqttFunc::decode_string(start, length_left, m_ClientIdentifier))
    {
        LOGWARN(" decode Client Identifier failed!");
        return CODE_FAILED;
    }    
    skip_bytes += m_ClientIdentifier.length() + sizeof(ushort);
    start = data + skip_bytes;
    length_left = length - skip_bytes;

    // If the Will Flag is set
    if (m_ConnectFlags.m_WillFlag)
    {
        // decode Will Topic
        if (0 != MqttFunc::decode_string(start, length_left, m_WillTopic))
        {
            LOGWARN(" decode Will Topic failed!");
            return CODE_FAILED;
        }    
        skip_bytes += m_WillTopic.length() + sizeof(ushort);
        start = data + skip_bytes;
        length_left = length - skip_bytes;

        // decode Will Message
        if (0 != MqttFunc::decode_string(start, length_left, m_WillMessage))
        {
            LOGWARN(" decode Will Message failed!");
            return CODE_FAILED;
        }    
        skip_bytes += m_WillMessage.length() + sizeof(ushort);
        start = data + skip_bytes;
        length_left = length - skip_bytes;
    }

    // If the User Name flag is set
    if (m_ConnectFlags.m_UserNameFlag)
    {
        // decode User Name
        if (0 != MqttFunc::decode_string(start, length_left, m_UserName))
        {
            LOGWARN(" decode User Name failed!");
            return CODE_FAILED;
        }    
        skip_bytes += m_UserName.length() + sizeof(ushort);
        start = data + skip_bytes;
        length_left = length - skip_bytes;
    }

    // If the Password flag is set
    if (m_ConnectFlags.m_PasswordFlag)
    {
        // decode Password
        if (0 != MqttFunc::decode_string(start, length_left, m_Password))
        {
            LOGWARN(" decode Password failed!");
            return CODE_FAILED;
        }    
        skip_bytes += m_Password.length() + sizeof(ushort);
        start = data + skip_bytes;
        length_left = length - skip_bytes;
    }
    
    return CODE_OK;
}


MQTTConnAck::MQTTConnAck()
{
    m_FixedHdr.m_MessageType = CONNACK;
    m_TopicNameCompressionResponse = 0;
    m_ConnectReturnCode = Connection_Accepted;
}

MQTTConnAck::~MQTTConnAck()
{
}

int MQTTConnAck::encode(uchar *data, int length, int& skip_bytes)
{
    skip_bytes = 0;
    uchar *start = data;
    start[0] = m_TopicNameCompressionResponse;
    start[1] = m_ConnectReturnCode;
    skip_bytes += 2;
    return CODE_OK;
}

int MQTTConnAck::decode(uchar *data, int length, int& skip_bytes)
{ 
    if (length < 2)
    {
        LOGWARN("MQTTConnAck decode length is %d, need more data.", length);
        return CODE_NOT_COMPLETE;
    }
    
    m_TopicNameCompressionResponse = data[0];
    m_ConnectReturnCode = data[1];
    skip_bytes = 2;
    return CODE_OK;
}

MQTTDisConnect::MQTTDisConnect()
{
    m_FixedHdr.m_MessageType = DISCONNECT;
}

MQTTDisConnect::~MQTTDisConnect()
{
}

    
int MQTTDisConnect::encode(uchar *data, int length, int& skip_bytes)
{
    skip_bytes = 0;
    return CODE_OK;
}

int MQTTDisConnect::decode(uchar *data, int length, int& skip_bytes)
{
    return CODE_OK;
}


