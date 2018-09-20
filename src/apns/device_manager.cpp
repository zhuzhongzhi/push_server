#include "zlogger.h"
#include "device_manager.h"

DeviceNode::DeviceNode()
{
    m_IPhone[0] = '\0';
    m_IPad[0] = '\0';
    m_IMac[0] = '\0';
}

DeviceNode::~DeviceNode()
{
}


DeviceManager::DeviceManager()
{
    for (int i = 0; i < MAX_DEVICE_ID; ++i)
    {
        m_UserDeviceMap[i] = NULL;
    }
}

DeviceManager::~DeviceManager()
{
    for (int i = 0; i < MAX_DEVICE_ID; ++i)
    {
        DeviceNode* device = m_UserDeviceMap[i];
        do
        {
            DeviceNode* del_dev = device;
            device = device->m_Next;
            ZDELETE(del_dev);
        }
        while(NULL != device);
        m_UserDeviceMap[i] = NULL;
    }
}


int DeviceManager::init()
{
    
    return 0;
}

void DeviceManager::get_device(UserIDType use_id, DeviceNode& devices)
{
    int slot_id = use_id % MAX_DEVICE_ID;
    LOGWARN("Device Manager User[%lld] slot id[%d] get device.", use_id, slot_id);

    ACE_Guard<ACE_Thread_Mutex>  guard(m_UserDeviceMutex[slot_id]);
    DeviceNode* device = m_UserDeviceMap[slot_id];
    if (NULL == device)
    {
    	LOGWARN("Device Manager User[%lld] slot id[%d] not device id.", use_id, slot_id);
    }
    else
    {
        do
        {
            if (device->m_UserID == use_id)
            {
                break;
            }
            else
            {
                device = device->m_Next;
            }
        }
        while(NULL != device);

        if (NULL != device)
        {
            if ('\0' != device->m_IPhone[0])
            {
                memcpy(devices.m_IPhone, device->m_IPhone, TOKEN_SIZE);
            }

            if ('\0' != device->m_IPad[0])
            {
                memcpy(devices.m_IPad, device->m_IPad, TOKEN_SIZE);
            }

            if ('\0' != device->m_IMac[0])
            {
                memcpy(devices.m_IMac, device->m_IMac, TOKEN_SIZE);
            }
        }
    }
}

void DeviceManager::add_device(UserIDType use_id, const STLString& device_id, int type)
{
    int slot_id = use_id % MAX_DEVICE_ID;
    LOGWARN("Device Manager User[%lld] slot id[%d] add device.", use_id, slot_id);

    ACE_Guard<ACE_Thread_Mutex>  guard(m_UserDeviceMutex[slot_id]);
    DeviceNode* device = m_UserDeviceMap[slot_id];
    if (NULL != device)
    {
        do
        {
            if (device->m_UserID == use_id)
            {
                break;
            }
            else
            {
                device = device->m_Next;
            }
        }
        while(NULL != device);
    }

    if (NULL == device)
    {
        ZNEW(device, DeviceNode);
        device->m_Next = m_UserDeviceMap[slot_id];
        m_UserDeviceMap[slot_id] = device;
    }
    
    if (0 == type)
    {
        memset(device->m_IPhone, 0, TOKEN_SIZE);
        token2bytes(device_id.c_str(), device->m_IPhone);
    }
    else if (1 == type)
    {
        memset(device->m_IPad, 0, TOKEN_SIZE);
        token2bytes(device_id.c_str(), device->m_IPad);
    }
    else if (2 == type)
    {
        memset(device->m_IMac, 0, TOKEN_SIZE);
        token2bytes(device_id.c_str(), device->m_IMac);
    }
}

void DeviceManager::token2bytes(const char *token, char *bytes)
{
    int val;
    while (*token) 
    {
        sscanf(token, "%2x", &val);
        *(bytes++) = (char)val;
        token += 2;
        while (*token == ' ') 
        {
            // skip space
            ++token;
        }
    }
}


