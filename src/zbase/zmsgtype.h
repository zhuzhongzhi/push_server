
#ifndef __MESSAGE_TYPE_H__
#define __MESSAGE_TYPE_H__

// inner message type
enum MSG_TYPE
{
	TYPE_CONNECT,
    TYPE_LOGGER,
    TYPE_MQTT,
	TYPE_CONTROL,
	TYPE_SERVICE,
};


enum SERVICE_TYPE
{
    ST_AUTH_REQ,
    ST_AUTH_RES,
    ST_KICK_OFF,
    ST_CHAR_RECORD,
    ST_GROUP_QUERY,
    ST_OFFLINE,
    ST_ONLINE_REQ,
    ST_ONLINE_RES,
    ST_RESERVE
};

#endif
