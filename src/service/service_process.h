#ifndef __SERVICE_PROCESS_H__
#define __SERVICE_PROCESS_H__

#include "zmqttdef.h"

class MQTTMessage;
class Session;
class MQTTPublish;
class ServiceThread;



class ReceiverNode
{
public:
    ReceiverNode(){};
    virtual ~ReceiverNode(){};

public:
    UserIDType         m_ReceiverUserID;
    MQTTPublish       *m_PublishMsg;
    ReceiverNode      *m_Next;
};


typedef int (*PROCESS_FUNC)(MQTTMessage*& mqtt_msg, Session* session);

class ServiceProcess
{
public:
    ServiceProcess();
    virtual ~ServiceProcess();

public:
    static int process_recv(MQTTMessage*& mqtt_msg, Session* session);
    static int process_send_to_receiver(ReceiverNode* current_receiver, ServiceThread* thread, UserIDType sender_id);
    static int process_send_to_one_receiver(MQTTPublish*& publishMsg, Session* receiver, ServiceThread* thread, UserIDType sender_id);
    
protected:
    static int process_reserved1(MQTTMessage*& mqtt_msg, Session* session);
    static int process_connect(MQTTMessage*& mqtt_msg, Session* session);
    static int process_connack(MQTTMessage*& mqtt_msg, Session* session);
    static int process_publish(MQTTMessage*& mqtt_msg, Session* session);
    static int process_puback(MQTTMessage*& mqtt_msg, Session* session);
    static int process_pubrec(MQTTMessage*& mqtt_msg, Session* session);
    static int process_pubrel(MQTTMessage*& mqtt_msg, Session* session);
    static int process_pubcomp(MQTTMessage*& mqtt_msg, Session* session);
    static int process_subscribe(MQTTMessage*& mqtt_msg, Session* session);
    static int process_suback(MQTTMessage*& mqtt_msg, Session* session);
    static int process_unsubscribe(MQTTMessage*& mqtt_msg, Session* session);
    static int process_unsuback(MQTTMessage*& mqtt_msg, Session* session);
    static int process_pingreq(MQTTMessage*& mqtt_msg, Session* session);
    static int process_pingrsp(MQTTMessage*& mqtt_msg, Session* session);
    static int process_disconnect(MQTTMessage*& mqtt_msg, Session* session);
    static int process_reserved2(MQTTMessage*& mqtt_msg, Session* session);

private:
    static void auth_ok(Session* session);
    static void auth_failed(Session* session, const unsigned char reason);
    static int get_device_type(const STLString& device);

private:
    static PROCESS_FUNC  m_ProcessFuctions[MqttTypeSize];
};
#endif
