
#include "zconfiger.h"
#include "zlogger.h"
#include "zdispatch.h"
#include "zmsgtype.h"
#include "zmqttpublish.h"
#include "zservicemessage.h"
#include "db_info.h"
#include "device_manager.h"
#include "apns_manager.h"

ApnsManager::ApnsManager()
{
    m_CTX = NULL;
    m_SSL = NULL;
    m_Meth = NULL;
    m_ServerCert = NULL;
    m_Key = NULL;
    m_BIO = NULL;
    m_MessageBuffer[0] = '\0';
    m_MsgContent[0] = '\0';
}

ApnsManager::~ApnsManager()
{
    reset();
}

void ApnsManager::reset()
{
    if(m_SSL)
    {
        SSL_shutdown(m_SSL);
        SSL_free(m_SSL);
        m_SSL = NULL;
    }
    
    if(m_CTX)
    {
        SSL_CTX_free(m_CTX);
        m_CTX = NULL;
    }
}

int ApnsManager::init_ssl()
{   
    STLString apns_host = GET_CFG_ITEM(APNS, Host);
    STLString apns_cert_file = GET_CFG_ITEM(APNS, CertFile);
    
    m_CTX = SSL_CTX_new(SSLv23_client_method());
    if (!SSL_CTX_use_certificate_chain_file(m_CTX, apns_cert_file.c_str())) 
    {
        LOGWARN("loading certificate from file[%s] failed.", apns_cert_file.c_str());
        reset();
        return -1;
    }
    
    if (!SSL_CTX_use_PrivateKey_file(m_CTX, apns_cert_file.c_str(), SSL_FILETYPE_PEM))
    {
        LOGWARN("loading private key from file[%s] failed.", apns_cert_file.c_str());
        reset();
        return -1;
    }
    
    char host_port[1024] = {0};
    strcpy(host_port, apns_host.c_str());
    m_BIO = BIO_new_connect(host_port);
    if (NULL == m_BIO) 
    {
        LOGWARN("Creating connection BIO failed, Host is %s.", apns_host.c_str());
        reset();
        return -1;
    }
    
    if (BIO_do_connect(m_BIO) <= 0) 
    {
        LOGWARN("Connection to Server[%s] failed.", apns_host.c_str());
        reset();
        return -1;
    }
    
    m_SSL = SSL_new(m_CTX);
    if (NULL == m_SSL) 
    {
        LOGWARN("SSL_new filed");
        reset();
        return -1;
    }

    SSL_set_bio(m_SSL, m_BIO, m_BIO);
    int result = SSL_connect(m_SSL);
    if (result <= 0) 
    {
        LOGWARN("SSL_connect failed, result is %d\n", result);
        reset();
        return -1;
    }

    LOGWARN("SSL_connect init ok.");
    return 0;
}

int ApnsManager::init(int id)
{
    support(TYPE_SERVICE, std::bind(&ApnsManager::on_service, this, std::placeholders::_1));
    
    set_index(id);
    
    return TQThread::init();
}

int ApnsManager::on_init()
{
    //connect to apple apns
    init_ssl();
    return ServiceThread::on_init();
}

int ApnsManager::on_service(NLMessage*& msg)
{
    switch (msg->m_Type)
    {
        case ST_OFFLINE:
        {
            on_apns(msg);
            break;
        }
        default:
        {
            delete msg;
            msg = NULL;
            break;
        }
    }

    return 0;
}

void ApnsManager::encode_apns(unsigned int& total_length, const char* payload, unsigned short payload_length, const char* device_token)
{
	//Simple Notification Format
	unsigned char command = 0;
	char *pointer = m_MessageBuffer;
	memcpy(pointer, &command, sizeof(unsigned char));
	pointer += 1;

	unsigned short item_length = htons(32);
	memcpy(pointer, &item_length,  2);
	pointer += 2;
	memcpy(pointer, device_token,  32);
	pointer += 32;

	unsigned short h_length = htons(payload_length);
	memcpy(pointer, &h_length,  2);
	pointer += 2;
	memcpy(pointer, payload,  payload_length);
	pointer += payload_length;

	total_length = (unsigned int)(pointer - m_MessageBuffer);
}

void ApnsManager::encode_enhanced_apns(unsigned int& total_length, unsigned int apns_id, const char* payload, unsigned short payload_length, const char* device_token)
{
	//Enhanced Notification Format
	unsigned char command = 1;
	char *pointer = m_MessageBuffer;
	memcpy(pointer, &command, sizeof(unsigned char));
	pointer += 1;

	memcpy(pointer, &apns_id, 4);
	pointer += 4;

	unsigned int expire = htonl(time(NULL)+86400);
	memcpy(pointer, &expire, 4);
	pointer += 4;

	unsigned short item_length = htons(32);
	memcpy(pointer, &item_length,  2);
	pointer += 2;
	memcpy(pointer, device_token,  32);
	pointer += 32;

	unsigned short h_length = htons(payload_length);
	memcpy(pointer, &h_length,  2);
	pointer += 2;
	memcpy(pointer, payload,  payload_length);
	pointer += payload_length;

	total_length = (unsigned int)(pointer - m_MessageBuffer);
}

int ApnsManager::on_apns(NLMessage*& msg)
{
    OfflineMsg* offline = (OfflineMsg*)msg;
    MQTTPublish* publish_msg = (MQTTPublish*)(offline->m_PublishMsg);
    DEBUGBIN((const char*)(publish_msg->m_PublishData), publish_msg->m_PublishDataLength, 
        "APNS message [Time is %u, Sender is %lld, Receiver is %lld, TopocName is %s, message id is %u]",
        publish_msg->m_TimeStamp,
        publish_msg->m_SenderID, 
        publish_msg->m_ReceiverID,
        publish_msg->m_TopicName.c_str(), 
        publish_msg->m_SendMessageID);

    DeviceNode devices;
    DeviceMgrIns::instance()->get_device(publish_msg->m_ReceiverID, devices);
    if (('\0' == devices.m_IPhone[0]) 
     && ('\0' == devices.m_IPad[0]) 
     && ('\0' == devices.m_IMac[0]))
    {
        LOGWARN("device not apple device, not need to apns.");

        ZDELETE(offline);
        msg = NULL;
        return 0;
    }

    /*
    int payload_length = MAX_MESSAGE_SIZE;
    CBase64::encode(publish_msg->m_PublishData, 
                    publish_msg->m_PublishDataLength,
                    m_Payload, payload_length);
    m_Payload[payload_length] = '\0';
    */
    int length = 0;
    memset(m_MsgContent, 0, sizeof(m_MsgContent));
    if (RT_GO == publish_msg->m_ReceiverType)
    {
        length = snprintf(m_MsgContent, sizeof(m_MsgContent),
                          FMT_GROUP,
                          publish_msg->m_GroupName.c_str(),
                          publish_msg->m_SenderName.c_str(),
                          publish_msg->m_TopicName.c_str());
    }
    else
    {
        length = snprintf(m_MsgContent, sizeof(m_MsgContent),
                          FMT_PERSON,
                          publish_msg->m_SenderName.c_str(),
                          publish_msg->m_TopicName.c_str());
    }
	LOGWARN("APNS msg content:[%s].", m_MsgContent);

    
    for (int i = 0; i < 3; ++i)
    {
        const char * device_token = NULL;
        if (0 == i)
        {
            device_token = devices.m_IPhone;
        }
        else if (1 == i)
        {
            device_token = devices.m_IPad;
        }
        else if (2 == i)
        {
            device_token = devices.m_IMac;
        }
        
        if ('\0' == device_token[0])
        {
            LOGWARN("on apns get a empty.");
            continue;
        }
        
        if (!m_SSL)
        {
            reset();
            if (0 != init_ssl())
            {
                //
                LOGWARN("on apns re init ssl connectio to apns failed.");
                return -1;
            }
        }

        unsigned int total_length = 0;
        encode_enhanced_apns(total_length, m_ApnsMsgId, m_MsgContent, length, device_token);
        ++m_ApnsMsgId;

        //
        int bytes =0; 
        int send_bytes = 0;

        DEBUGBIN(m_MessageBuffer, length, "APNS Message:");

        while (send_bytes < total_length)
        {
            bytes = SSL_write(m_SSL, m_MessageBuffer + send_bytes, total_length); 
            if (bytes > 0)
            {
            	LOGWARN("SSL Write to [%d] Bytes!",  bytes);
                send_bytes += bytes;
            }
            else
            {
                int sys_errno = errno;
                LOGWARN("SSL Write to APNS error[%d]!",  sys_errno);
            
                // send failed, check error no
                if ((EINTR == sys_errno) || (EAGAIN == sys_errno) || (0 == sys_errno) || (EWOULDBLOCK == sys_errno))
                {
                    continue;
                }
                else
                {
                    reset();
                    break;
                }          
            }
       }

        LOGWARN("APNS Send finish a message!");
    }

    ZDELETE(offline);
    msg = NULL;
    return 0;
}

// change deviceToken string to binary bytes
void ApnsManager::token2bytes(const char *token, char *bytes)
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




