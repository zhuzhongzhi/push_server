#ifndef __SESSION_H__
#define __SESSION_H__

#include <list>
#include <map>
#include "ace/Basic_Types.h"
#include "ace/Event_Handler.h"
#include "ace/Time_Value.h"
#include "service_process.h"
#include "zbasedef.h"

const int SESSION_STATUS_INIT = 0;
const int SESSION_STATUS_WAIT_AUTH_RSP = 1;
const int SESSION_STATUS_WORK = 2;
const int SESSION_STATUS_END = 99;

const unsigned short DEFAULT_KEEPLIVE = 1800; // 30 * 60 30


class ServiceThread;
class NLMessage;
typedef std::map<unsigned short, NLMessage*>::value_type  wait_value_type;

struct mqtt_packet
{
    uint8_t command;
    uint8_t have_remaining;
    uint8_t remaining_count;
    uint16_t mid;
    uint32_t remaining_mult;
    uint32_t remaining_length;
    uint32_t packet_length;
    uint32_t to_process;
    uint32_t pos;
    uint8_t *payload;
};

#define SENDING_HEADER 0
#define SENDING_REMAINING 1
struct mqtt_sending
{
    char                      m_MsgHeader[8];         
    int                       m_MsgHeaderLength;      
    int                       m_MsgSendState;         
    int                       m_MsgSendBytes;         
    NLMessage                *m_CurrentMsg;           
};
class Session : public TQHandler
{
public:
    friend class ServiceProcess;
    enum
    {
        SESSION_RECV_BUFFER_SIZE = 409600,
        SESSION_SEND_BUFFER_SIZE = 409600,
        LOWER_LEVEL_BUFFER_SIZE = 256,
        PUBLISH_TIMEOUT_TIME = 10,
        HEAR_TIMEOUT = 90,
        HEAR_SIZE = 8,
    };
public:
    Session(ACE_HANDLE fd, ServiceThread* thread);
    virtual ~Session();

public:
    int init();
    void close();
    void clear();
    void set(ACE_HANDLE fd, ServiceThread* thread);
    
    void clear_packet();
    void clear_sending();
    int new_sending();
    int set_sending(NLMessage* msg);
    void end_sending();
    void try_sending();
    int handle_input();
    int handle_output();
    int send_data();

    int handle_error();
    int handle_close();

    void set_timer(int future, int interval = 0);
    void cancle_timer();

    int handle_timeout(const ACE_Time_Value &current_time, const void *act);
    
    int register_input();
    int unregister_input();
    int register_output();
    int unregister_output();
    int register_all();
    int unregister_all();

    void set_status(int status);
    int get_status();
    ACE_HANDLE get_peer_fd();
    UserIDType get_user_id();
    void set_user_id(UserIDType user_id);

    void set_keep_alive_timer(unsigned short keep_alive_timer);
    void update_last_message_time();
    int process_ack(unsigned short message_id);
    int process_resend();
    void put_down_message(NLMessage* msg);
    void put_down_message(std::list<NLMessage*>& msgs);
    ServiceThread* get_thread();
    bool valid();
    void set_delay_close(bool delay_close);
    
    bool is_kick_off();
    void set_is_kick_off();

    void set_user_name(const std::string& user_name);
    const std::string& get_user_name();
    void set_password(const std::string& password);
    const std::string& get_password();

    void set_device_type(const std::string& device_type);
    const std::string& get_device_type();
    void set_device_token(const std::string& device_token);
    const std::string& get_device_token();
    void set_device_int_type(int type);
    int get_device_int_type();

    void query_offline_message();
    unsigned short get_message_id();
    void set_same_device();
    bool get_same_device();
    bool is_heart_beat_timeout();   

private:
    UserIDType                m_UserID;
    ACE_HANDLE                m_PeerFd;
    int                       m_Status;
    ServiceThread*           m_Thread;
    std::list<NLMessage*>    m_DownMessage;
    std::map<unsigned short, NLMessage*>    m_WaitResponseMessage;
    unsigned short            m_KeepAlivetimer;
    time_t                    m_LastMessageTime;
    long                      m_TimeID; 
    bool                      m_DelayClose;

    ACE_Atomic_Op<ACE_Thread_Mutex, int>    m_IsKickOff;
    bool                      m_SameDevice;               

    
    std::string               m_UserName;             
    std::string               m_Password;             
    std::string               m_DeviceType;           
    int                       m_DeviceIntType;        
    std::string               m_DeviceToken;         
    bool                      m_QueryOfflineMessage;
    unsigned short            m_MessageID;

    mqtt_packet               m_MqttPacket;       
    mqtt_sending              m_MqttSending;      
};

#endif
