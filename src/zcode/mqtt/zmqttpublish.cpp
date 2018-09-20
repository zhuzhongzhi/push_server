
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmqttpublish.h"

MQTTPublish::MQTTPublish()
{
    m_FixedHdr.m_MessageType = PUBLISH;
    m_FixedHdr.m_QosLevel = 1;
    m_RecvMessageID = 0;
    m_SendMessageID = 0;
    m_TimeStamp = time(NULL);
    m_SenderID = 0;
    m_ReceiverID = 0;
    m_MessageType = MT_TXT;
    m_ReceiverType = RT_PO;
    m_PublishData = NULL;
    m_PublishDataLength = 0;
    m_SessionFD = -1;
}

MQTTPublish::~MQTTPublish()
{
    m_PublishData = NULL;
    m_PublishDataLength = 0;
}

MQTTPublish *MQTTPublish::clone()
{
    MQTTPublish *msg = NULL;
    ZNEW(msg, MQTTPublish);
    msg->m_Type = m_Type;
    msg->m_RefNum = 1;
    msg->m_TimerID = -1;
    msg->m_FixedHdr = m_FixedHdr;
    msg->m_RemainingLength = m_RemainingLength;
    ZNEW_S(msg->m_RemainingData, uchar, msg->m_RemainingLength);
    memcpy(msg->m_RemainingData, m_RemainingData, msg->m_RemainingLength);
    msg->m_TopicName = m_TopicName;
    msg->m_RecvMessageID = m_RecvMessageID;
    msg->m_SendMessageID = m_SendMessageID;
    msg->m_TimeStamp = m_TimeStamp;
    msg->m_SenderID = m_SenderID;
    msg->m_SenderName = m_SenderName;
    msg->m_GroupName = m_GroupName;
    msg->m_ReceiverID = m_ReceiverID;
    msg->m_MessageType = m_MessageType;
    msg->m_ReceiverType = m_ReceiverType;
    msg->m_MultReceivers = m_MultReceivers;
    msg->m_PublishData = msg->m_RemainingData + (m_PublishData - m_RemainingData);
    msg->m_PublishDataLength = m_PublishDataLength;

    msg->m_SessionFD = m_SessionFD;
    return msg;
};

int MQTTPublish::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    if (!m_TopicName.empty())
    {        
        if (0 != MqttFunc::encode_string(m_TopicName, start, length_left))
        {
            LOGWARN("MQTTPublish Encode topic name failed!");
            return CODE_NOT_ENOUGH_BUFFER;
        }
        skip = m_TopicName.length() + sizeof(ushort);
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
    
    // encode message id
    MqttFunc::encode_short(m_RecvMessageID, start, length_left);
    skip = sizeof(ushort);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    // encode TimeStamp
    MqttFunc::encode_int(m_TimeStamp, start, length_left);
    skip = sizeof(unsigned int);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    // encode message type
    MqttFunc::encode_short(m_MessageType, start, length_left);
    skip = sizeof(unsigned short);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    // encode receiver type
    MqttFunc::encode_short(m_ReceiverType, start, length_left);
    skip = sizeof(unsigned short);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    // encode sender id
    MqttFunc::encode_long(m_SenderID, start, length_left);
    skip = sizeof(unsigned long long);
    start += skip;
    length_left -= skip;
    skip_bytes += skip;

    if (RT_PS == m_ReceiverType)
    {
        if (!m_MultReceivers.empty())
        {        
            if (0 != MqttFunc::encode_string(m_MultReceivers, start, length_left))
            {
                LOGWARN("MQTTPublish Encode m_MultReceivers failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_MultReceivers.length() + sizeof(unsigned short);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(unsigned short);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
    }
    else
    {
        MqttFunc::encode_long(m_ReceiverID, start, length_left);
        skip = sizeof(unsigned long long);
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }

    if (RT_GO == m_ReceiverType)
    {
        if (!m_GroupName.empty())
        {        
            if (0 != MqttFunc::encode_string(m_GroupName, start, length_left))
            {
                LOGWARN("MQTTPublish Encode m_GroupName failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = m_MultReceivers.length() + sizeof(unsigned short);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
        else
        {
            MqttFunc::encode_short(0, start, length_left);
            skip = sizeof(unsigned short);
            start += skip;
            length_left -= skip;
            skip_bytes += skip;
        }
    }
    
    if (length_left < m_PublishDataLength)
    {
        LOGWARN("MQTTPublish Encode Publish Data failed! Data length is %d, left length is %d.", 
                m_PublishDataLength, length_left);
        return CODE_NOT_ENOUGH_BUFFER;
    }
    memcpy(start, m_PublishData, m_PublishDataLength);
    skip_bytes += m_PublishDataLength;
    return CODE_OK;
}

int MQTTPublish::decode(uchar *data, int length, int& skip_bytes)
{    
    LOGINFO("MQTTPublish::decode data length is %d, RemainingLength is %d.",
            length, m_RemainingLength);
    length = m_RemainingLength;

    uchar *start = data;
    int length_left = length;

    // decode Topic Name
    if (0 != MqttFunc::decode_string(start, length_left, m_TopicName))
    {
        LOGWARN("MQTTPublish decode topic Name failed!");
        return CODE_FAILED;
    }    
    skip_bytes = m_TopicName.length() + sizeof(ushort);
    start = data + skip_bytes;
    if (length < skip_bytes)
    {
        LOGWARN("MQTTPublish decode message TopicName failed, length is %d, TopicName length is %d.",
            length, skip_bytes);
        return CODE_FAILED;
    }
    
    length_left = length - skip_bytes;

    if (m_FixedHdr.m_QosLevel > 0)
    {
        // decode message id
        if (0 != MqttFunc::decode_short(start, length_left, m_RecvMessageID))
        {
            LOGWARN("MQTTPublish decode message id failed.");
            return CODE_FAILED;
        }

        skip_bytes += sizeof(ushort);
        start = data + skip_bytes;
    }
    else
    {
        LOGINFO("MQTTPublish qos_level is 0, message id not present .");
    }

    if (length < skip_bytes)
    {
        LOGWARN("MQTTPublish decode message playload failed.");
        return CODE_FAILED;
    }
    
    // decode timestamp
    if (0 != MqttFunc::decode_int(start, length_left, (uint&)m_TimeStamp))
    {
        LOGWARN("MQTTPublish decode timestamp failed.");
        return CODE_FAILED;
    }

    skip_bytes += sizeof(long long);
    start = data + skip_bytes;    

    // decode msg_type
    if (0 != MqttFunc::decode_short(start, length_left, m_MessageType))
    {
        LOGWARN("MQTTPublish decode msg_type failed.");
        return CODE_FAILED;
    }

    skip_bytes += sizeof(unsigned short);
    start = data + skip_bytes; 

    // decode receiver_type
    if (0 != MqttFunc::decode_short(start, length_left, m_ReceiverType))
    {
        LOGWARN("MQTTPublish decode receiver_type failed.");
        return CODE_FAILED;
    }

    skip_bytes += sizeof(unsigned short);
    start = data + skip_bytes;
    
    // decode sender id
    if (0 != MqttFunc::decode_long(start, length_left, m_SenderID))
    {
        LOGWARN("MQTTPublish decode sender failed.");
        return CODE_FAILED;
    }

    skip_bytes += sizeof(long long);
    start = data + skip_bytes; 

    if (RT_PS == m_ReceiverType)
    {
        // decode mult receivers
        if (0 != MqttFunc::decode_string(start, length_left, m_MultReceivers))
        {
            LOGWARN("MQTTPublish decode topic Name failed!");
            return CODE_FAILED;
        }
        
        skip_bytes = m_MultReceivers.length() + sizeof(unsigned short);
        start = data + skip_bytes;
    }
    else
    {
        // decode reciver
        if (0 != MqttFunc::decode_long(start, length_left, m_ReceiverID))
        {
            LOGWARN("MQTTPublish decode reciver failed.");
            return CODE_FAILED;
        }

        skip_bytes += sizeof(long long);
        start = data + skip_bytes;
    }

    if (RT_GO == m_ReceiverType)
    {
        // decode groupname
        if (0 != MqttFunc::decode_string(start, length_left, m_GroupName))
        {
            LOGWARN("MQTTPublish decode topic Name failed!");
            return CODE_FAILED;
        }
        
        skip_bytes = m_MultReceivers.length() + sizeof(unsigned short);
        start = data + skip_bytes;
    }
    
    // decode content
    m_PublishDataLength = length_left = length - skip_bytes;
    m_PublishData = (uchar*)start; 
    skip_bytes += length_left;
    
    LOGINFO("Recv Publish Message, TopicName is %s, Message ID is %d, Publish Data Length is %d, Publish Data is %s.",
           m_TopicName.c_str(), m_RecvMessageID, m_PublishDataLength, m_PublishData);
    return CODE_OK;
}

void MQTTPublish::replace_message_id(ushort mid)
{
    if (NULL == m_RemainingData)
    {
        return;
    }
    
    m_SendMessageID = mid;
    
    uchar *start = (uchar *)m_RemainingData;
    int skip = 0;
    
    if (!m_TopicName.empty())
    {
        skip = m_TopicName.length() + sizeof(ushort);
    }
    else
    {
        skip = sizeof(unsigned short);
    }
    
    start += skip;
    MqttFunc::encode_short(mid, start, m_RemainingLength - skip);
}

void MQTTPublish::replace_time_stamp()
{
    if (NULL == m_RemainingData)
    {
        return;
    }
    
    uchar *start = (uchar *)m_RemainingData;
    int skip = 0;
    
    if (!m_TopicName.empty())
    {
        skip = m_TopicName.length() + sizeof(ushort);
    }
    else
    {
        skip = sizeof(ushort);
    }
    skip += 2; // skip message id
    start += skip;
    MqttFunc::encode_int(m_TimeStamp, start, m_RemainingLength - skip);
}

MQTTPubAck::MQTTPubAck()
{
    m_FixedHdr.m_MessageType = PUBACK;
    m_MessageID = 0;
}

MQTTPubAck::~MQTTPubAck()
{
}

int MQTTPubAck::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(ushort);
    skip_bytes += skip;
    return CODE_OK;
}

int MQTTPubAck::decode(uchar *data, int length, int& skip_bytes)
{
    // decode message id
    if (0 != MqttFunc::decode_short(data, length, m_MessageID))
    {
        LOGWARN("MQTT_PubAck decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    return CODE_OK;
}

MQTTPubRec::MQTTPubRec()
{
    m_FixedHdr.m_MessageType = PUBREC;
    m_MessageID = 0;
}

MQTTPubRec::~MQTTPubRec()
{
}

int MQTTPubRec::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(ushort);
    skip_bytes += skip;
    return CODE_OK;
}

int MQTTPubRec::decode(uchar *data, int length, int& skip_bytes)
{
    // decode message id
    if (0 != MqttFunc::decode_short(data, length, m_MessageID))
    {
        LOGWARN("MQTT_PubRec decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    return CODE_OK;
}

MQTTPubRel::MQTTPubRel()
{
    m_FixedHdr.m_MessageType = PUBREL;
    m_MessageID = 0;
}

MQTTPubRel::~MQTTPubRel()
{
}

int MQTTPubRel::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(ushort);
    skip_bytes += skip;
    return CODE_OK;
}

int MQTTPubRel::decode(uchar *data, int length, int& skip_bytes)
{
    // decode message id
    if (0 != MqttFunc::decode_short(data, length, m_MessageID))
    {
        LOGWARN("MQTT_PubRel decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    return CODE_OK;
}

MQTTPubComp::MQTTPubComp()
{
    m_FixedHdr.m_MessageType = PUBCOMP;
    m_MessageID = 0;
}

MQTTPubComp::~MQTTPubComp()
{
}

int MQTTPubComp::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;

    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(ushort);
    skip_bytes += skip;
    return CODE_OK;
}

int MQTTPubComp::decode(uchar *data, int length, int& skip_bytes)
{
    // decode message id
    if (0 != MqttFunc::decode_short(data, length, m_MessageID))
    {
        LOGWARN("MQTT_PubComp decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    return CODE_OK;
}

