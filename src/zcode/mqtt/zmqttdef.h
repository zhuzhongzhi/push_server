#ifndef __MQTT_DEFINE_H__
#define __MQTT_DEFINE_H__

#include "zbasedef.h"

enum TMqttType
{
    Reserved1 = 0,
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    PUBREC = 5,
    PUBREL = 6,
    PUBCOMP = 7,
    SUBSCRIB = 8,
    SUBACK = 9,
    UNSUBSCRIB = 10,
    UNSUBACK = 11,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14,
    Reserved2 = 15,
    MqttTypeSize
};

enum TCodeResult
{
    CODE_OK = 0,
    CODE_NOT_ENOUGH_BUFFER,
    CODE_NOT_COMPLETE,
    CODE_FAILED,
};

enum TCodeProcess
{
    PROCESS_OK = 0,
    PROCESS_NEED_CONNECT,
    PROCESS_SEND_AUTH_REQ,
    PROCESS_WAIT_AUTH_RSP,
    PROCESS_FAILED,
};

typedef enum {
    MT_TXT=0,               // 文本消息
    MT_ARM,                 // 语音消息
    MT_VIDEO                // 视频消息
} MessageType;

typedef enum {
    MS_RECV=0,              // 消息收取， 未读
    MT_READ,                // 消息收取， 已读
    MT_SEND,                // 消息发送， 未成功
    MT_DONE,                // 消息发送， 成功
    MT_DELETE               // 消息删除
} MessageState;

typedef enum {
    MD_IN=0,                // 收取的消息
    MD_OUT,                 // 发送的消息
} MessageDirection;

typedef enum {
    RT_PO=0,                // person
    RT_PS,                  // mult persons
    RT_GO,                  // group
} RecverType;


/**Connect return code**/
extern const uchar Connection_Accepted;
extern const uchar Unacceptable_Protocol_Version;
extern const uchar Identifier_Rejected;
extern const uchar Server_Unavailable;
extern const uchar Bad_UserName_OR_Password;
extern const uchar Not_Authorized;
extern const uchar Reserved; /**6-255 Reserved for future use**/

extern const STLString  DeviceType_IPHONE;
extern const STLString  DeviceType_IPAD;
extern const STLString  DeviceType_APHONE;
extern const STLString  DeviceType_APAD;
extern const STLString  DeviceType_WPHONE;
extern const STLString  DeviceType_WPAD;
extern const STLString  DeviceType_IPC;
extern const STLString  DeviceType_WPC;

#pragma pack(1)
struct TFXHdr
{
    uchar  m_Retain:1;
    uchar  m_QosLevel:2;
    uchar  m_DupFlag:1;
    uchar  m_MessageType:4;
};

struct TConnectFlags
{
    uchar m_Reserved:1;
    uchar m_CleanSession:1;
    uchar m_WillFlag:1;
    uchar m_WillQos:2;
    uchar m_WillRetain:1;
    uchar m_PasswordFlag:1;
    uchar m_UserNameFlag:1;
}; 
#pragma pack()


#endif