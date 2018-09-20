
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmqttsubscribe.h"

MQTTSubscribe::MQTTSubscribe()
{
    m_FixedHdr.m_MessageType = SUBSCRIB;
    m_MessageID = 0;
}

MQTTSubscribe::~MQTTSubscribe()
{
}

int MQTTSubscribe::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;
    
    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(unsigned short);
    skip_bytes += skip;

    std::list<TSubTopic>::iterator iter;
    for (iter = m_SubTopics.begin(); iter != m_SubTopics.end(); ++iter)
    {        
        TSubTopic& tpoic_qos = *iter;
        if (length_left < (tpoic_qos.m_TopicName.length() + sizeof(unsigned short) + 1))
        {
            LOGWARN("MQTTPublish Encode topic name and qos failed!");
            return CODE_NOT_ENOUGH_BUFFER;
        }
        
        if (!tpoic_qos.m_TopicName.empty())
        {        
            if (0 != MqttFunc::encode_string(tpoic_qos.m_TopicName, start, length_left))
            {
                LOGWARN("MQTTPublish Encode topic name failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = tpoic_qos.m_TopicName.length() + sizeof(unsigned short);
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
        *start = tpoic_qos.m_Qos;
        skip = 1;
        start += skip;
        length_left -= skip;
        skip_bytes += skip;
    }
    
    return CODE_OK;
}

int MQTTSubscribe::decode(uchar *data, int length, int& skip_bytes)
{
    LOGINFO("MQTT_Subscribe::decode data length is %d, RemainingLength is %d.", length, m_RemainingLength);
    length = m_RemainingLength;
    uchar *start = data;
    int length_left = length;
    
    // decode message id
    if (0 != MqttFunc::decode_short(start, length_left, m_MessageID))
    {
        LOGWARN("MQTT_Subscribe decode message id failed.");
        return CODE_FAILED;
    }  
    
    skip_bytes += sizeof(unsigned short);
    start = data + skip_bytes;
    length_left = length - skip_bytes;

    TSubTopic  sub_topic;
    do
    {
        sub_topic.m_TopicName.clear();
        
        // decode topic Name
        if (0 != MqttFunc::decode_string(start, length_left, sub_topic.m_TopicName))
        {
            LOGWARN("MQTT_Subscribe decode topic Name failed!");
            return CODE_FAILED;
        }
        
        skip_bytes += sub_topic.m_TopicName.length() + sizeof(unsigned short);
        start = data + skip_bytes;
        length_left = length - skip_bytes;

        // decode requested qos
        sub_topic.m_Qos = *start;
        skip_bytes += 1;
        start = data + skip_bytes;
        length_left = length - skip_bytes;
        LOGINFO("MQTT_Subscribe::decode Toppic is %s, Qos is %d, length_left is %d.", 
                sub_topic.m_TopicName.c_str(),
                sub_topic.m_Qos,
                length_left);
        m_SubTopics.push_back(sub_topic);
    } while (length_left > 3);

    if (0 != length_left)
    {
        DEBUGBIN((const char*)data, length, "MQTT_Subscribe decode left length is %d.", length_left);
    }

    LOGINFO("MQTT_Subscribe::decode skip_bytes is %d, m_TopicQosPairs size is %d.", skip_bytes, m_SubTopics.size());
    
    return CODE_OK;
}

MQTTSubAck::MQTTSubAck()
{
    m_FixedHdr.m_MessageType = SUBACK;
    m_MessageID = 0;
}

MQTTSubAck::~MQTTSubAck()
{
}

int MQTTSubAck::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;
    
    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(unsigned short);
    skip_bytes += skip;
    length_left -= skip;
    start += skip;
    
    if (length_left < m_GrantedQos.size())
    {
        LOGWARN("MQTT_SubAck encode failed, left length is %d, Qos size is %d, not enough buffer.",
                length_left, m_GrantedQos.size());
        return CODE_NOT_ENOUGH_BUFFER;
    }

    skip = m_GrantedQos.size();
    for (int i = 0; i < skip; ++i)
    {
        start[i] = m_GrantedQos[i];
    }
    skip_bytes += skip;
    
    return CODE_OK;
}

int MQTTSubAck::decode(uchar *data, int length, int& skip_bytes)
{    
    uchar *start = data;
    int length_left = length;
    
    // decode message id
    if (0 != MqttFunc::decode_short(start, length_left, m_MessageID))
    {
        LOGWARN("MQTT_Subscribe decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(unsigned short);
    start = data + skip_bytes;
    length_left = length - skip_bytes;

    for (int i = 0; i < length_left; ++i)
    {
        m_GrantedQos.push_back(start[i]);
    }

    skip_bytes += length_left;
    
    return CODE_OK;
}


