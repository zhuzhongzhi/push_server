
#include "zmsgtype.h"
#include "znlmessage.h"

NLMessage::NLMessage()
{
    m_Type = 0;
    m_RefNum = 1;
    m_TimerID = -1;
};
NLMessage::~NLMessage()
{
    m_RefNum = 0;
};
    
int NLMessage::get_type()
{
    return m_Type;
}
void NLMessage::set_type(int t)
{
    m_Type = t;
}

ConnectMsg::ConnectMsg()
{
    m_PeerFd = -1;
    m_Type = TYPE_CONNECT;
}
    
ConnectMsg::~ConnectMsg()
{
};

LogMsg::LogMsg(int size)
{
    m_Type = TYPE_LOGGER;
    m_BufferSize = 0;
    m_ArraySize = size;
    if (m_ArraySize <= 0)
    {
        m_ArraySize = DEFAULT_BUFFER_SIZE;
    }
    m_Buffer = new char[m_ArraySize];
}

LogMsg::~LogMsg()
{
    if (NULL != m_Buffer)
    {
        delete[] m_Buffer;
        m_Buffer = NULL;
    }
}
    
void LogMsg::clear()
{
    m_BufferSize = 0;
    m_Buffer[0] = '\0';
}


