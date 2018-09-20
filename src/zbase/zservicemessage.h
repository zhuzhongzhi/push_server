//
//  zservicemessage.h
//  pusher
//
//  Created by zzz on 16/4/15.
//
//

#ifndef zservicemessage_h
#define zservicemessage_h

#include "znlmessage.h"
#include "zmsgtype.h"

class ServiceMsg : public NLMessage
{
public:
    ServiceMsg()
    {
        m_Type = TYPE_SERVICE;
        m_ServiceType = 0;
        m_ThreadIndex = -1;
    };
    virtual ~ServiceMsg(){};
    
public:
    int m_ServiceType;
    int m_ThreadIndex;
};

class AuthMsgReq : public ServiceMsg
{
public:
    AuthMsgReq()
    {
        m_ServiceType = ST_AUTH_REQ;
        m_UserID = 0;
        m_DeviceFD = -1;
    };
    virtual ~AuthMsgReq(){};
    
public:
    UserIDType       m_UserID;
    int              m_DeviceFD;
    STLString        m_UserName;
    STLString        m_Password;
}; 

class AuthMsgRsp : public ServiceMsg
{
public:
    AuthMsgRsp()
    {
        m_ServiceType = ST_AUTH_RES;
        m_UserID = 0;
        m_DeviceFD = -1;
        m_AuthResult = -1;
    };
    virtual ~AuthMsgRsp(){};
    
public:
    UserIDType       m_UserID;
    int              m_DeviceFD;
    int              m_AuthResult;
};

class KickOff : public ServiceMsg
{
public:
    KickOff()
    {
        m_ServiceType = ST_KICK_OFF;
        m_UserFD = -1;
    };
    virtual ~KickOff(){};
    
public:
    int           m_UserFD;
};

class ChatRecord : public ServiceMsg
{
public:
    ChatRecord()
    {
        m_ServiceType = ST_CHAR_RECORD;
        m_RecordMsg = NULL;
    };
    virtual ~ChatRecord()
    {
        ZDELETE(m_RecordMsg);
    };
    
public:
    NLMessage       *m_RecordMsg;
};

class OnlineReq : public ServiceMsg
{
public:
    OnlineReq()
    {
        m_ServiceType = ST_ONLINE_REQ;
        m_UserID = 0;
        m_DeviceFD = -1;
    };
    virtual ~OnlineReq(){};
    
public:
    UserIDType       m_UserID;
    int              m_DeviceFD;
};

class OnlineRsp : public ServiceMsg
{
public:
    OnlineRsp()
    {
        m_ServiceType = ST_ONLINE_RES;
        m_UserID = 0;
        m_DeviceFD = -1;
    };
    virtual ~OnlineRsp(){};
    
public:
    UserIDType            m_UserID;
    int                   m_DeviceFD;
    std::list<NLMessage*> m_Messages;
};


class GroupQuery : public ServiceMsg
{
public:
    GroupQuery()
    {
        m_ServiceType = ST_GROUP_QUERY;
        m_UserID = 0;
        m_DeviceFD = -1;
        m_GroupID = 0;
        m_PublishMsg = NULL;
    };
    virtual ~GroupQuery()
    {
        ZDELETE(m_PublishMsg);
    };
    
public:
    UserIDType              m_UserID;
    int                     m_DeviceFD;
    UserIDType              m_GroupID;
    NLMessage              *m_PublishMsg;
    std::list<UserIDType>   m_UserIDs;
};


class OfflineMsg : public ServiceMsg
{
public:
    OfflineMsg()
    {
        m_ServiceType = ST_OFFLINE;
        m_PublishMsg = NULL;
    };
    virtual ~OfflineMsg()
    {
        ZDELETE(m_PublishMsg);
    };
    
public:
    NLMessage       *m_PublishMsg;
};


#endif /* zservicemessage_h */
