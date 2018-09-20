#ifndef __APNS_MANAGER_H__
#define __APNS_MANAGER_H__

#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#include "service_thread.h"

#define MAX_MESSAGE_SIZE 409600

#define FMT_GROUP      "{\"aps\":{\"alert\":\"%s\n%s:%s\",\"badge\":1,\"sound\":\"default\"}}"
#define FMT_PERSON     "{\"aps\":{\"alert\":\"%s:%s\",\"badge\":1,\"sound\":\"default\"}}"

class ApnsManager : public ServiceThread
{
public:
    ApnsManager();
    virtual ~ApnsManager();

public:
    virtual int init(int id);
    virtual int on_init();
    int on_service(NLMessage*& msg);
    int on_apns(NLMessage*& msg);
    void token2bytes(const char *token, char *bytes);

    int init_ssl();
    void reset();
    void encode_apns(unsigned int& total_length, const char* payload, unsigned short payload_length, const char* device_token);
    void encode_enhanced_apns(unsigned int& total_length, unsigned int apns_id, const char* payload, unsigned short payload_length, const char* device_token);


private:    
    SSL_CTX                        *m_CTX;
    SSL                            *m_SSL;
    const SSL_METHOD               *m_Meth;
    X509                           *m_ServerCert;
    EVP_PKEY                       *m_Key;
    BIO                            *m_BIO;

    unsigned int                    m_ApnsMsgId;
    char                            m_MessageBuffer[MAX_MESSAGE_SIZE];
    char                            m_MsgContent[MAX_MESSAGE_SIZE];
};

#endif

