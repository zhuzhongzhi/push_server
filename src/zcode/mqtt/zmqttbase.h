#ifndef __MQTT_BASE_H__
#define __MQTT_BASE_H__

#include "zbasedef.h"
#include "znlmessage.h"
#include "zmqttdef.h"

class MQTTMessage : public NLMessage
{
public:
    MQTTMessage();
    virtual ~MQTTMessage();

    int encode_header(uchar *data, int length, int& bytes);
    
    void set_fix_hdr(uchar cmd);
    MQTTMessage* decode();
    void dump();
    
    void set_base(const MQTTMessage *msg);
    virtual int encode(uchar *data, int length, int& skip_bytes){ return 0;};
    virtual int decode(uchar *data, int length, int& skip_bytes){ return 0;};
    
    void decode_i();

public:    
    /**Fixed header**/
    TFXHdr          m_FixedHdr;
    int             m_RemainingLength;
    uchar          *m_RemainingData;
};

#endif // __MQTT_BASE_H__
