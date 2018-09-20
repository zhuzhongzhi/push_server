#ifndef _DARWIN
#include <sys/epoll.h>
#endif
#include <time.h>
#include "ace/OS_NS_sys_socket.h"

#include "zdispatch.h"
#include "zlogger.h"
#include "zmqttconnect.h"
#include "zmqttfunc.h"
#include "zmqttping.h"
#include "zmqttsubscribe.h"
#include "zmqttunsubscribe.h"
#include "zmqttpublish.h"
#include "zmqttbase.h"
#include "znlmessage.h"
#include "zmqttpublish.h"
#include "zservicemessage.h"
#include "service_process.h"
#include "service_thread.h"
#include "session_manger.h"
#include "session.h"

Session::Session(ACE_HANDLE fd, ServiceThread* thread)
{
    m_UserID = -1;
    m_PeerFd = fd;
    m_Thread = thread;
    m_Status = SESSION_STATUS_INIT;
    m_KeepAlivetimer = DEFAULT_KEEPLIVE;
    m_LastMessageTime = 0;
    m_TimeID = -1;
    m_DelayClose = false;
    m_IsKickOff = 0;
    m_SameDevice = false;
    m_UserName.clear();
    m_Password.clear();
    m_DeviceType.clear();
    m_DeviceIntType = 0;
    m_DeviceToken.clear();
    m_QueryOfflineMessage = false;
    m_MessageID = 0;
    memset(&m_MqttPacket, 0, sizeof(m_MqttPacket));
    memset(&m_MqttSending, 0, sizeof(m_MqttSending));
}

Session::~Session()
{
    m_Thread = NULL;
}

int Session::init()
{
    clear();
    return 0;
}

void Session::close()
{
    if (is_kick_off() && !get_same_device())
    {
        // send a disconnect message
        MQTTDisConnect* disconnect = NULL;
        ZNEW(disconnect, MQTTDisConnect);
        disconnect->m_RemainingLength = 0;
        put_down_message(disconnect);
        disconnect = NULL;

        LOGWARN("User[%d:%lld] Service Porcess send disconnect.",
                get_peer_fd(), get_user_id());

        try_sending();
    }
    handle_close();    
}

void Session::clear()
{
    m_UserID = -1;
    m_PeerFd = -1;
    m_Thread = NULL;
    m_Status = SESSION_STATUS_INIT;
    m_KeepAlivetimer = DEFAULT_KEEPLIVE;
    m_LastMessageTime = 0;
    m_TimeID = -1;
    m_DelayClose = false;
    m_IsKickOff = 0;
    m_SameDevice = false;
    m_UserName.clear();
    m_Password.clear();
    m_DeviceType.clear();
    m_DeviceIntType = 0;
    m_DeviceToken.clear();
    m_QueryOfflineMessage = false;
    m_MessageID = 0;
    memset(&m_MqttPacket, 0, sizeof(m_MqttPacket));
    memset(&m_MqttSending, 0, sizeof(m_MqttSending));
}

void Session::set(ACE_HANDLE fd, ServiceThread* thread)
{
    m_UserID = -1;
    m_PeerFd = fd;
    m_Thread = thread;
    m_Status = SESSION_STATUS_INIT;
}

void Session::clear_packet()
{
    m_MqttPacket.command = 0;
    m_MqttPacket.have_remaining = 0;
    m_MqttPacket.remaining_count = 0;
    m_MqttPacket.remaining_mult = 1;
    m_MqttPacket.remaining_length = 0;
    ZDELETE_A(m_MqttPacket.payload);
    m_MqttPacket.payload = NULL;
    m_MqttPacket.to_process = 0;
    m_MqttPacket.pos = 0;
}

void Session::clear_sending()
{
    m_MqttSending.m_MsgHeaderLength = 0;
    m_MqttSending.m_MsgSendState = 0;
    m_MqttSending.m_MsgSendBytes = 0;
    ZDELETE(m_MqttSending.m_CurrentMsg);
}

int Session::new_sending()
{
    NLMessage* msg = m_DownMessage.front();
    if (NULL == msg)
    {
        m_DownMessage.pop_front();
        LOGINFO("SESSION[%d:%lld] down message is NULL.", get_peer_fd(), get_user_id());
        return 1;
    }

    return set_sending(msg);
}

int Session::set_sending(NLMessage* msg)
{
    MQTTMessage* base = (MQTTMessage*)msg;
    if (PUBLISH == base->m_FixedHdr.m_MessageType)
    {
        MQTTPublish* pub_msg = (MQTTPublish*)base ;
        pub_msg->replace_message_id(get_message_id());
        LOGWARN("SESSION[%d:%lld] send message[%lld--->%lld, %d:%d].",
                  get_peer_fd(), get_user_id(),
                  pub_msg->m_SenderID, pub_msg->m_ReceiverID,
                  pub_msg->m_SendMessageID, pub_msg->m_RecvMessageID);
    }

    // encode header
    m_MqttSending.m_MsgHeader[0] = *((unsigned char *)&(base->m_FixedHdr));
    int skip_bytes = 0;
    if (-1 == MqttFunc::encode_variable_length(base->m_RemainingLength,
                                (unsigned char*)(m_MqttSending.m_MsgHeader + 1),
                                               6, 
                                               skip_bytes))
    {
        LOGWARN("MQTTMessage Remaining Length is %d, not enough buffer.",
                base->m_RemainingLength);
        clear_sending();
        return -1;
    }

    m_MqttSending.m_MsgHeaderLength  = skip_bytes + 1;

    return 0;
}

void Session::end_sending()
{
    if ((m_DownMessage.empty()) && (NULL == m_MqttSending.m_CurrentMsg))
    {
        unregister_output();
    }
    else
    {
        register_output();
    }
}

void Session::try_sending()
{
    if (NULL == m_MqttSending.m_CurrentMsg)
    {
        //try send data
        if (0 == new_sending())
        {
            send_data();
        }
    }

    end_sending();
}

int Session::handle_input()
{
    uint8_t byte;
    ssize_t read_length;
    int rc = 0;
    if (!m_MqttPacket.command)
    {
        read_length = recv(m_PeerFd, &byte, 1, 0);
        if (read_length == 1)
        {
            m_MqttPacket.command = byte;
        }
        else
        {
            if (read_length == 0)
            {
                /* EOF */
                handle_close();
                return -1;
            }

            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //wait next read
                return 0;
            }
            else
            {
                handle_close();
                return -1;
            }
        }
    }

    if (!m_MqttPacket.have_remaining)
    {
        /* Read remaining
         * Algorithm for decoding taken from pseudo code at
         * http://publib.boulder.ibm.com/infocenter/wmbhelp/v6r0m0/topic/com.ibm.etools.mft.doc/ac10870_.htm
         */
        do
        {
            read_length = recv(m_PeerFd, &byte, 1, 0);
            if (read_length == 1)
            {
                m_MqttPacket.remaining_count++;
                /* Max 4 bytes length for remaining length as defined by protocol.
                 * Anything more likely means a broken/malicious client.
                 */
                if (m_MqttPacket.remaining_count > 4)
                {
                    /* error message */
                    handle_close();
                    return -1;
                }

                m_MqttPacket.remaining_length += (byte & 127)
                                * m_MqttPacket.remaining_mult;
                m_MqttPacket.remaining_mult *= 128;
            }
            else
            {
                if (read_length == 0)
                {
                    /* EOF */
                    handle_close();
                    return -1;
                }
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    //wait next read
                    return 0;
                }
                else
                {
                    handle_close();
                    return -1;
                }
            }
        }
        while ((byte & 128) != 0);

        if (m_MqttPacket.remaining_length > 0)
        {
            ZNEW_S(m_MqttPacket.payload, uint8_t, m_MqttPacket.remaining_length);
            if (!m_MqttPacket.payload)
            {
                handle_close();
                return -1;
            }
            m_MqttPacket.to_process = m_MqttPacket.remaining_length;
        }
        m_MqttPacket.have_remaining = 1;
    }

    while (m_MqttPacket.to_process > 0)
    {
        read_length = recv(m_PeerFd,
                           &(m_MqttPacket.payload[m_MqttPacket.pos]),
                           m_MqttPacket.to_process, 0);
        if (read_length > 0)
        {
            m_MqttPacket.to_process -= read_length;
            m_MqttPacket.pos += read_length;
        }
        else
        {
            if (read_length == 0)
            {
                /* EOF */
                handle_close();
                return -1;
            }
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //wait next read
                return 0;
            }
            else
            {
                handle_close();
                return -1;
            }
        }
    }

    MQTTMessage *mqtt_msg = NULL;
    switch ((m_MqttPacket.command) & 0xF0)
    {
        case PINGREQ:
        {
            ZNEW(mqtt_msg, MQTTPingReq);
            break;
        }
        case PINGRESP:
        {
            ZNEW(mqtt_msg, MQTTPingRsp);
            break;
        }
        case PUBACK:
        {
            ZNEW(mqtt_msg, MQTTPubAck);
            break;
        }
        case PUBCOMP:
        {
            ZNEW(mqtt_msg, MQTTPubComp);
            break;
        }
        case PUBLISH:
        {
            ZNEW(mqtt_msg, MQTTPublish);
            break;
        }
        case PUBREC:
        {
            ZNEW(mqtt_msg, MQTTPubRec);
            break;
        }
        case PUBREL:
        {
            ZNEW(mqtt_msg, MQTTPubRel);
            break;
        }
        case CONNECT:
        {
            ZNEW(mqtt_msg, MQTTConnect);
            break;
        }
        case CONNACK:
        {
            ZNEW(mqtt_msg, MQTTConnAck);
            break;
        }
        case DISCONNECT:
        {
            ZNEW(mqtt_msg, MQTTDisConnect);
            break;
        }
        case SUBSCRIB:
        {
            ZNEW(mqtt_msg, MQTTSubscribe);
            break;
        }
        case SUBACK:
        {
            ZNEW(mqtt_msg, MQTTSubAck);
            break;
        }
        case UNSUBSCRIB:
        {
            ZNEW(mqtt_msg, MQTTUNSubscribe);
            break;
        }
        case UNSUBACK:
        {
            ZNEW(mqtt_msg, MQTTUNSubAck);
            break;
        }
        default:
        {
            clear_packet();
            return 0;
        }
    }

    mqtt_msg->m_RemainingLength = m_MqttPacket.remaining_length;
    memcpy(&(mqtt_msg->m_FixedHdr), &(m_MqttPacket.command), 1);
    mqtt_msg->m_RemainingData = m_MqttPacket.payload;
    m_MqttPacket.payload = NULL;
    clear_packet();

    int skip_bytes = 0;
    mqtt_msg->decode_i();
    if (SESSION_STATUS_WAIT_AUTH_RSP == get_status())
    {
        if (CONNECT != mqtt_msg->m_FixedHdr.m_MessageType)
        {
            LOGWARN("Session status is %d, and message type is %d, discard it.",
                    get_status(), mqtt_msg->m_FixedHdr.m_MessageType);
            return 0;
        }
    }

    update_last_message_time();

    ServiceProcess::process_recv(mqtt_msg, (Session*)this);

    return 0;
}

int Session::process_ack(unsigned short message_id)
{
    std::map<unsigned short, NLMessage*>::iterator iter;
    iter = m_WaitResponseMessage.find(message_id);
    if (iter != m_WaitResponseMessage.end())
    {        
        NLMessage* msg = iter->second;
        MQTTPublish* publish_msg = (MQTTPublish*)msg;
        LOGWARN("SESSION[%d:%lld] process_ack delete a wait response message, id is %u, old id is %u.",
                get_peer_fd(), 
                get_user_id(),
                publish_msg->m_SendMessageID, 
                publish_msg->m_RecvMessageID);
        ZDELETE(msg);
        m_WaitResponseMessage.erase(iter);
    }
    return 0;
}

int Session::process_resend()
{
    LOGINFO("process_resend.");
    if (!m_WaitResponseMessage.empty())
    {
        NLMessage* msg = NULL;
        MQTTPublish* publish_msg = NULL;
        time_t current_time = m_Thread->get_current_time();
        std::map<unsigned short, NLMessage*>::iterator iter, iter_del;
        for (iter = m_WaitResponseMessage.begin(); iter != m_WaitResponseMessage.end(); )
        {
            msg = iter->second;
            if (NULL != msg)
            {
                publish_msg = (MQTTPublish*)msg;
                if ((current_time - publish_msg->m_TimeStamp) > PUBLISH_TIMEOUT_TIME) 
                {
                    m_WaitResponseMessage.erase(iter++);
                    if (0 == set_sending(msg))
                    {
                        if (1 != send_data())
                        {
                            return 0;
                        }
                    }
                    else
                    {
                        ZDELETE(msg);
                    }
                }
                else
                {
                    ++iter;
                }
            }
            else
            {
                ++iter;
            }
        }
    }
    
    return 0;
}


int Session::handle_output()
{
    if ((NULL != m_MqttSending.m_CurrentMsg) && (send_data() < 0))
    {
        LOGWARN("Session[%d:%lld] handle_output send_data failed!", m_PeerFd, m_UserID);
        handle_close();
        return 0;
    }

    if (m_DownMessage.empty())
    {
        process_resend();
    }
    
    ServiceThread* thread = get_thread();
    if (NULL == thread)
    {
        LOGWARN("SESSION[%d:%lld] service thread is NULL!",
                get_peer_fd(), get_user_id());
        handle_close();
        return 0;
    }

    time_t current_time = get_thread()->get_current_time();
    if ((current_time - m_LastMessageTime) > Session::HEAR_TIMEOUT)
    {
        LOGWARN("SESSION[%d:%lld] heartbeat timeout!",
                get_peer_fd(), 
                get_user_id());
        handle_close();
        return 0;
    }

    if (m_DownMessage.empty())
    {
        LOGINFO("SESSION[%d:%lld] no down message.",
                get_peer_fd(), 
                get_user_id());
    }
    else
    {
        int rc = 0;
        do
        {
            rc = new_sending();
            if (0 == rc)
            {
                rc = send_data();
                if (1 != rc)
                {
                    break;
                }
            }
        }
        while (!(m_DownMessage.empty()));
    }

    end_sending();
    return 0;
}

int Session::send_data()
{
    int bytes = 0;
    if (SENDING_HEADER == m_MqttSending.m_MsgSendState)
    {
        bytes = ACE_OS::send(m_PeerFd,
                             m_MqttSending.m_MsgHeader + m_MqttSending.m_MsgSendBytes,
                             m_MqttSending.m_MsgHeaderLength - m_MqttSending.m_MsgSendBytes);
        if (bytes > 0)
        {
            m_MqttSending.m_MsgSendBytes += bytes;
            if (m_MqttSending.m_MsgSendBytes >= m_MqttSending.m_MsgHeaderLength)
            {
                m_MqttSending.m_MsgSendState = SENDING_REMAINING;
                m_MqttSending.m_MsgSendBytes = 0;
            }
            else
            {
                //wait for next write
                return 0;
            }
        }
        else
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                //wait next write
                return 0;
            }
            else
            {
                handle_close();
                return -1;
            }
        }
    }

    if (SENDING_REMAINING == m_MqttSending.m_MsgSendState)
    {
        MQTTMessage * mqtt_msg = (MQTTMessage *)(m_MqttSending.m_CurrentMsg);
        if ((NULL != mqtt_msg->m_RemainingData)
         && (mqtt_msg->m_RemainingLength > 0))
        {
            bytes = ACE_OS::send(m_PeerFd,
                                 (const char*)(mqtt_msg->m_RemainingData + m_MqttSending.m_MsgSendBytes),
                                 mqtt_msg->m_RemainingLength - m_MqttSending.m_MsgSendBytes);
            if (bytes > 0)
            {
                m_MqttSending.m_MsgSendBytes += bytes;
                if (m_MqttSending.m_MsgSendBytes >= mqtt_msg->m_RemainingLength)
                {
                    if (PUBLISH == mqtt_msg->m_FixedHdr.m_MessageType)
                    {
                        MQTTPublish* publish_msg = (MQTTPublish*)(m_MqttSending.m_CurrentMsg);
                        std::map<unsigned short, NLMessage*>::iterator iter = m_WaitResponseMessage.find(publish_msg->m_SendMessageID);
                        if (iter != m_WaitResponseMessage.end())
                        {
                            NLMessage* old_msg = iter->second;
                            ZDELETE(old_msg);
                            m_WaitResponseMessage.erase(iter);
                        }
                        publish_msg->m_TimeStamp = time(NULL);
                        m_WaitResponseMessage.insert(wait_value_type(publish_msg->m_SendMessageID,
                                                                     m_MqttSending.m_CurrentMsg));
                        m_MqttSending.m_CurrentMsg = NULL;
                    }

                    clear_sending();
                    return 1;
                }
                else
                {
                    //wait for next write
                    return 0;
                }
            }
            else
            {
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    //wait next write
                    return 0;
                }
                else
                {
                    handle_close();
                    return -1;
                }
            }
        }
        else
        {
            //no data need write
            clear_sending();
            return 1;
        }
    }

    return 0;
}

int Session::handle_error()
{
    handle_close();
    return 0;
}

int Session::handle_close()
{
    clear_packet();
    if (ACE_INVALID_HANDLE == m_PeerFd)
    {
        LOGWARN("handle_close Session already closed.");
        return 0;
    }
    
    if (m_DelayClose)
    {
        LOGWARN("Session[%d:%lld] user delay close.", m_PeerFd, m_UserID);
        return 0;
    }

    if (NULL == m_MqttSending.m_CurrentMsg)
    {
        send_data();
    }
    
    LOGINFO("Session[%d:%lld] tcp connections close.", m_PeerFd, m_UserID);
    
    cancle_timer();

    unregister_all();
    ACE_OS::closesocket(m_PeerFd);

    std::list<NLMessage*>::iterator  iter = m_DownMessage.begin();
    for ( ; iter != m_DownMessage.end(); ++iter)
    {        
        NLMessage* nl_msg = *iter;
        MQTTMessage* mqtt_msg = (MQTTMessage*)nl_msg;
        if (PUBLISH == mqtt_msg->m_FixedHdr.m_MessageType)
        {
            OfflineMsg* offline = NULL;
            ZNEW(offline, OfflineMsg);
            offline->m_PublishMsg = (MQTTPublish*)mqtt_msg;
            if (0 != Dispatch::instance()->post_message(offline, T_DB))
            {
                LOGWARN("handle_close m_DownMessage Publish Message send offline message failed.");
                ZDELETE(offline);
            }
            else
            {
                LOGINFO("handle_close m_DownMessage Publish Message send offline message Success.");
            }    
        }
        else
        {
            ZDELETE(nl_msg);
        }
    }
    m_DownMessage.clear();

    std::map<unsigned short, NLMessage*>::iterator iter_resend;
    for (iter_resend = m_WaitResponseMessage.begin() ; iter_resend != m_WaitResponseMessage.end(); ++iter_resend)
    {        
        NLMessage* nl_msg = iter_resend->second;
        MQTTMessage* mqtt_msg = (MQTTMessage*)nl_msg;
        if (PUBLISH == mqtt_msg->m_FixedHdr.m_MessageType)
        {
            OfflineMsg* offline = NULL;
            ZNEW(offline, OfflineMsg);
            offline->m_PublishMsg = (MQTTPublish*)mqtt_msg;
            if (0 != Dispatch::instance()->post_message(offline, T_DB))
            {
                LOGWARN("handle_close m_WaitResponseMessage Publish Message send offline message failed.");
                ZDELETE(offline);
            }
            else
            {
                LOGINFO("handle_close m_WaitResponseMessage Publish Message send offline message Success.");
            }    
        }
        else
        {
            ZDELETE(nl_msg);
        }
    }
    m_WaitResponseMessage.clear();
    
    SessionMgrIns::instance()->release_session(m_PeerFd, m_UserID);

    clear();
    return 0;
}

void Session::set_timer(int future, int interval)
{
    if (-1 == m_TimeID)
    {
        m_TimeID = m_Thread->set_timer(this, future, interval, (void*)m_Thread);
        LOGINFO("User[%d:%lld] set timer, timer id is %ld. ", m_PeerFd, m_UserID, m_TimeID);
    }
    else
    {
        LOGWARN("User[%d:%lld] have timer, id is %ld, can not set timer again!", m_PeerFd, m_UserID, m_TimeID);
    }
}

void Session::cancle_timer()
{
    if (-1 != m_TimeID)
    {
        LOGINFO("User[%d:%lld] cancle timer, timer id is %ld. ", m_PeerFd, m_UserID, m_TimeID);
        m_Thread->cancle_timer(m_TimeID);
        m_TimeID = -1;
    }
    else
    {
        LOGWARN("User[%d:%lld] timer not exist, not need cancle timer!", m_PeerFd, m_UserID);
    }
}

int Session::handle_timeout(const ACE_Time_Value &current_time, const void *act)
{
    m_TimeID = -1;
    
    if (SESSION_STATUS_WAIT_AUTH_RSP == m_Status)
    {
        LOGWARN("User[%d:$lld] Auth time out!", m_PeerFd, m_UserID);
        handle_close();
        return 0;
    }
    else
    {
        LOGWARN("User[%d:$lld] Status is %d and time out!", m_PeerFd, m_UserID, m_Status);
    }
    
    return 0;
}

int Session::register_input()
{
    if (! m_Thread->event_register(m_PeerFd, EPOLLIN))
    {
        LOGWARN("register Session[%d:%lld] event[EPOLLIN] to FDDriver failed!", m_PeerFd, m_UserID);       
        return -1;
    }

    return 0;
}

int Session::unregister_input()
{
    if (! m_Thread->event_unregister(m_PeerFd, EPOLLIN))
    {
        LOGWARN("unregister Session[%d:%lld] event[EPOLLIN] to FDDriver failed!", m_PeerFd, m_UserID);      
        return -1;
    }
    
    return 0;
}

int Session::register_output()
{
    if (! m_Thread->event_register(m_PeerFd, EPOLLOUT))
    {
        LOGWARN("register Session[%d:%lld] event[EPOLLOUT] to FDDriver failed!", m_PeerFd, m_UserID);       
        return -1;
    }

    return 0;
}

int Session::unregister_output()
{
    if (! m_Thread->event_unregister(m_PeerFd, EPOLLOUT))
    {
        LOGWARN("unregister Session[%d:%lld] event[EPOLLOUT] to FDDriver failed!", m_PeerFd, m_UserID);      
        return -1;
    }

    return 0;
}

int Session::register_all()
{
    if (! m_Thread->event_register(m_PeerFd, EPOLLIN|EPOLLOUT))
    {
        LOGWARN("register Session[%d:%lld] event[EPOLLIN|EPOLLOUT] to FDDriver failed!", m_PeerFd, m_UserID);       
        return -1;
    }

    return 0;
}

int Session::unregister_all()
{
    if (! m_Thread->event_unregister(m_PeerFd, EPOLLIN|EPOLLOUT))
    {
        LOGWARN("unregister Session[%d:%lld] event[EPOLLIN|EPOLLOUT] to FDDriver failed!", m_PeerFd, m_UserID);      
        return -1;
    }

    return 0;
}

void Session::set_status(int status)
{
    m_Status = status;
}

int Session::get_status()
{
    return m_Status;
}

ACE_HANDLE Session::get_peer_fd()
{
    return m_PeerFd;
}

UserIDType Session::get_user_id()
{
    return m_UserID;
}

void Session::set_user_id(UserIDType user_id)
{
    m_UserID = user_id;
}

void Session::set_keep_alive_timer(unsigned short keep_alive_timer)
{
    m_KeepAlivetimer = keep_alive_timer;
}

void Session::update_last_message_time()
{    
    m_LastMessageTime = m_Thread->get_current_time();
}

void Session::put_down_message(NLMessage* msg)
{
    m_DownMessage.push_back(msg);
}

void Session::put_down_message(std::list<NLMessage*>& msgs)
{
    m_DownMessage.insert(m_DownMessage.end(), msgs.begin(), msgs.end());
}


ServiceThread* Session::get_thread()
{
    return m_Thread;
}

bool Session::valid()
{
    return (NULL != m_Thread) && (ACE_INVALID_HANDLE != m_PeerFd) && (!is_kick_off());
}


void Session::set_delay_close(bool delay_close)
{
    m_DelayClose = delay_close;
}


bool Session::is_kick_off()
{
    return (m_IsKickOff > 0);
}

void Session::set_is_kick_off()
{
    m_IsKickOff++;
}

void Session::set_user_name(const std::string& user_name)
{
    m_UserName = user_name;
}
const std::string& Session::get_user_name()
{
    return m_UserName;
}

void Session::set_password(const std::string& password)
{
    m_Password = password;    
}

const std::string& Session::get_password()
{
    return m_Password;
}

void Session::set_device_type(const std::string& device_type)
{
    m_DeviceType = device_type;
}

const std::string& Session::get_device_type()
{
    return m_DeviceType;
}

void Session::set_device_token(const std::string& device_token)
{
    m_DeviceToken = device_token;
}

const std::string& Session::get_device_token()
{
    return m_DeviceToken;
}

void Session::set_device_int_type(int type)
{
    m_DeviceIntType = type;
}

int Session::get_device_int_type()
{
    return m_DeviceIntType;
}

void Session::query_offline_message()
{    
    if (m_QueryOfflineMessage)
    {
        return ;
    }

    m_QueryOfflineMessage = true;
    OnlineReq* online_req = NULL;
    ZNEW(online_req, OnlineReq);
    online_req->m_UserID = get_user_id();
    online_req->m_DeviceFD = get_peer_fd();
    online_req->m_ThreadIndex = get_thread()->get_index();
    
    if (0 != Dispatch::instance()->post_message_by_user(online_req, T_DB, online_req->m_UserID))
    {
        LOGWARN("Service Process send online request failed.");
        ZDELETE(online_req);
    }
    else
    {
        LOGWARN("Service Process send online request success, wait for online response.");
    }
}


unsigned short Session::get_message_id()
{
    return m_MessageID++;
}

void Session::set_same_device()
{
    m_SameDevice = true;
}
bool Session::get_same_device()
{
    return m_SameDevice;
}

bool Session::is_heart_beat_timeout()
{
    time_t current_time = m_Thread->get_current_time();
    if ((current_time - m_LastMessageTime) > Session::HEAR_TIMEOUT)
    {
        LOGWARN("SESSION[%d:%lld] heartbeat timeout!", get_peer_fd(), get_user_id());
        return true;
    }
    else
    {
        return false;
    }
}




