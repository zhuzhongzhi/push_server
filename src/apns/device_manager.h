#ifndef __DEVICE_MANGER_H__
#define __DEVICE_MANGER_H__

#include "ace/Event_Handler.h"
#include "ace/Singleton.h"
#include "ace/Thread_Mutex.h"
#include <list>
#include <map>
#include "zbasedef.h"
/**
apple device manager
*/
#define MAX_DEVICE_ID 65535
#define TOKEN_SIZE 32
class ServiceThread;

class DeviceNode
{
public:
    DeviceNode();
    virtual ~DeviceNode();

    
public:
    char    m_IPhone[TOKEN_SIZE];
    char    m_IPad[TOKEN_SIZE];
    char    m_IMac[TOKEN_SIZE];

    UserIDType   m_UserID;
    DeviceNode  *m_Next;
};

class DeviceManager
{
public:
    DeviceManager();
    virtual ~DeviceManager();

public:
    int init();
    void get_device(UserIDType use_id, DeviceNode& devices);
    void add_device(UserIDType use_id, const STLString& device_id, int type);
    void token2bytes(const char *token, char *bytes);
private:
    DeviceNode*                   m_UserDeviceMap[MAX_DEVICE_ID];
    ACE_Thread_Mutex              m_UserDeviceMutex[MAX_DEVICE_ID];
};


typedef ACE_Singleton<DeviceManager, ACE_Recursive_Thread_Mutex> DeviceMgrIns;


#endif

