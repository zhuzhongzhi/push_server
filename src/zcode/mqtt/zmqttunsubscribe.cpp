
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmqttunsubscribe.h"

MQTTUNSubscribe::MQTTUNSubscribe()
{
    m_FixedHdr.m_MessageType = UNSUBSCRIB;
    m_MessageID = 0;
}

MQTTUNSubscribe::~MQTTUNSubscribe()
{
}

int MQTTUNSubscribe::encode(uchar *data, int length, int& skip_bytes)
{
    skip_bytes = 0;
    uchar *start = data;
    int length_left = length;
    int skip = 0;
    
    MqttFunc::encode_short(m_MessageID, start, length_left);
    skip = sizeof(ushort);
    skip_bytes += skip;

    std::list<STLString>::iterator iter;
    for (iter = m_Topics.begin(); iter != m_Topics.end(); ++iter)
    {
        STLString& topic = *iter;
        if (!topic.empty())
        {        
            if (0 != MqttFunc::encode_string(topic, start, length_left))
            {
                LOGWARN("MQTT_UNSubscribe Encode topic name failed!");
                return CODE_NOT_ENOUGH_BUFFER;
            }
            skip = topic.length() + sizeof(ushort);
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

int MQTTUNSubscribe::decode(uchar *data, int length, int& skip_bytes)
{
    uchar *start = data;
    int length_left = length;
    
    // decode message id
    if (0 != MqttFunc::decode_short(start, length_left, m_MessageID))
    {
        LOGWARN("MQTT_UNSubscribe decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    start = data + skip_bytes;
    length_left = length - skip_bytes;
    
    STLString topic;
    do
    {
        topic.clear();
        
        // decode topic
        if (0 != MqttFunc::decode_string(start, length_left, topic))
        {
            LOGWARN("MQTT_UNSubscribe decode topic failed!");
            return CODE_FAILED;
        }    
        skip_bytes = topic.length() + sizeof(ushort);
        start = data + skip_bytes;
        length_left = length - skip_bytes;

        m_Topics.push_back(topic);
    }while (length_left > 3);

    if (0 != length_left)
    {
        DEBUGBIN((const char*)data, length, "MQTT_UNSubscribe decode left length is %d.", length_left);
    }
    return CODE_OK;
}

MQTTUNSubAck::MQTTUNSubAck()
{
    m_FixedHdr.m_MessageType = UNSUBACK;
    m_MessageID = 0;
}

MQTTUNSubAck::~MQTTUNSubAck()
{
}

int MQTTUNSubAck::encode(uchar *data, int length, int& skip_bytes)
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

int MQTTUNSubAck::decode(uchar *data, int length, int& skip_bytes)
{
    uchar *start = data;
    int length_left = length;
    
    // decode message id
    if (0 != MqttFunc::decode_short(start, length_left, m_MessageID))
    {
        LOGWARN("MQTT_UNSubAck decode message id failed.");
        return CODE_FAILED;
    }
    skip_bytes += sizeof(ushort);
    return CODE_OK;
}


