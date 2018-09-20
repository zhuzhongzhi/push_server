
#include "zmqttfunc.h"
#include "zlogger.h"
#include "zmqttping.h"

MQTTPingReq::MQTTPingReq()
{
    m_FixedHdr.m_MessageType = PINGREQ;
}

MQTTPingReq::~MQTTPingReq()
{
    
}

int MQTTPingReq::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;

    return CODE_OK;
}

int MQTTPingReq::decode(uchar *data, int length, int& skip_bytes)
{
    return CODE_OK;
}

MQTTPingRsp::MQTTPingRsp()
{
    m_FixedHdr.m_MessageType = PINGRESP;
}

MQTTPingRsp::~MQTTPingRsp()
{
    
}

int MQTTPingRsp::encode(uchar *data, int length, int& skip_bytes)
{    
    skip_bytes = 0;
    return CODE_OK;
}

int MQTTPingRsp::decode(uchar *data, int length, int& skip_bytes)
{
    return CODE_OK;
}


