
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmsgtype.h"
#include "zmqttbase.h"
#include "zmqttconnect.h"
#include "zmqttpublish.h"
#include "zmqttsubscribe.h"
#include "zmqttunsubscribe.h"
#include "zmqttping.h"


MQTTMessage::MQTTMessage()
{
    memset(&m_FixedHdr, 0, sizeof(m_FixedHdr));
    m_RemainingLength = 0;
    m_Type = TYPE_MQTT;
    m_RemainingData = NULL;
};

MQTTMessage::~MQTTMessage()
{
    ZDELETE_A(m_RemainingData);
};


int MQTTMessage::encode_header(uchar *data, int length, int& bytes)
{
    data[0] = *((unsigned char *)&m_FixedHdr);
    MqttFunc::encode_variable_length(m_RemainingLength, data + 1, length, bytes);
    return 0;
};

void MQTTMessage::set_fix_hdr(uchar cmd)
{
    memcpy(&m_FixedHdr, &cmd, 1);
}

void MQTTMessage::set_base(const MQTTMessage *msg)
{
    m_FixedHdr = msg->m_FixedHdr;
    m_RemainingLength = msg->m_RemainingLength;
    m_RemainingData = msg->m_RemainingData;
}

MQTTMessage* MQTTMessage::decode()
{
    MQTTMessage *msg = NULL;
    switch (m_FixedHdr.m_MessageType)
    {
        case CONNECT:
        {
            ZNEW(msg, MQTTConnect);
            break;
        }
        case CONNACK:
        {
            ZNEW(msg, MQTTConnAck);
            break;
        }
        case PUBLISH:
        {
            ZNEW(msg, MQTTPublish);
            break;
        }
        case PUBACK:
        {
            ZNEW(msg, MQTTPubAck);
            break;
        }
        case PUBREC:
        {
            ZNEW(msg, MQTTPubRec);
            break;
        }
        case PUBREL:
        {
            ZNEW(msg, MQTTPubRel);
            break;
        }
        case PUBCOMP:
        {
            ZNEW(msg, MQTTPubComp);
            break;
        }
        case SUBSCRIB:
        {
            ZNEW(msg, MQTTSubscribe);
            break;
        }
        case SUBACK:
        {
            ZNEW(msg, MQTTSubAck);
            break;
        }
        case UNSUBSCRIB:
        {
            ZNEW(msg, MQTTUNSubscribe);
            break;
        }
        case UNSUBACK:
        {
            ZNEW(msg, MQTTUNSubAck);
            break;
        }
        case PINGREQ:
        {
            ZNEW(msg, MQTTPingReq);
            break;
        }
        case PINGRESP:
        {
            ZNEW(msg, MQTTPingRsp);
            break;
        }
        case DISCONNECT:
        {
            ZNEW(msg, MQTTDisConnect);
            break;
        }
        case Reserved1:
        case Reserved2:
        default:
        {
            break;
        }
    }
    
    if (NULL != msg)
    {
        msg->set_base(this);
        m_RemainingData = NULL;
        msg->decode_i();
    }
    return msg;
};

void MQTTMessage::decode_i()
{
    if ((NULL != m_RemainingData) && (m_RemainingLength > 0))
    {
        int skip_bytes = 0;
        if (CODE_OK != decode(m_RemainingData, m_RemainingLength, skip_bytes))
        {
            LOGWARN("decode failed.");
        }
    }
}

void MQTTMessage::dump()
{
    LOGINFO("mqtt message[%d,%d],[%d,%d,%d]",
             m_FixedHdr.m_MessageType, m_RemainingLength, m_FixedHdr.m_DupFlag,
             m_FixedHdr.m_QosLevel, m_FixedHdr.m_Retain);
    
    DEBUGBIN((const char*)m_RemainingData, m_RemainingLength, "message data:");
}
