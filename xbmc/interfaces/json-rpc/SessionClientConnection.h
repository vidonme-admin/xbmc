#if !defined(__PROXY_SESSION_CLIENT__H__)
#define __PROXY_SESSION_CLIENT__H__

#include "utils/Variant.h"
#include "boost/enable_shared_from_this.hpp"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"

#include "SResult.h"
#include "ProxyJSONRPC.h"

using namespace JSONRPC;

enum RemoteModuleResult
{
    rmrUnknown = kUnknown,
    rmrOk = kOk,
    rmrError = RemoteModuleError + 1,
    rmrInactiveRemoteModule,
    rmrCancelWaitingForResponse,
    rmrTimeoutWaitingForResponse,
    rmrDisconnectWaitingForResponse,
    rmrMaximum,
};

typedef unsigned int SimpleActionID;
class CSimpleAction;
typedef boost::shared_ptr<CSimpleAction> SimpleActionPtr;
typedef std::map<SimpleActionID, SimpleActionPtr> SimpleActionList;


class CSessionClientConnection;
typedef boost::shared_ptr<CSessionClientConnection> SessionClientConnectionPtr;


typedef std::function<void(const CSimpleAction& simpleAction)> SimpleActionEvent;

class CSimpleAction
    : public boost::enable_shared_from_this<CSimpleAction>
{
public:
    CSimpleAction();
    SimpleActionID simpleActionId( void ) const { return m_simpleActionId; }
private:
    SimpleActionID m_simpleActionId;
    unsigned int m_tickCount;
public:
    void SetSessionClient(const SessionClientConnectionPtr& sessionClient);
    bool IsTimeout( const unsigned int tickCount ) const;
    bool IsBusy( void ) const { return m_bIsBusy; }
    SResult sResult( void ) const { return m_sResult; }
    void MethodSend(
        const CStdString& method, 
        const CVariant& params);
    void OnResponse(
        const SResult sResult,
        const CVariant& response);
    void DoCancel( void );
    const CVariant& response( void ) const { return m_response; }
private:
    SessionClientConnectionPtr m_sessionClient;
    volatile bool m_bIsBusy;
    SResult m_sResult;
    CVariant m_response;
public:
    void SetEvent(const SimpleActionEvent& event);
private:
    CCriticalSection m_mutexEvent;
    SimpleActionEvent m_event; 
};

class IClientConnectionEvent
{
public:

    void ClientConnection_OnNofity(
        const SessionClientID sessionClientId,
        const NotifyKind kind,
        const CStdString& strEvent,
        const CStdString& arg1,
        const CStdString& arg2,
        const CStdString& arg3);
    
};

class CSessionClientConnection
    : public boost::enable_shared_from_this<CSessionClientConnection>
    , public CThread
{
#pragma region sync action

public:
    SResult SessionClientConnect( 
        const SessionClientID &sessionClientId );
    SResult SessionClientDisconnect( 
        const SessionClientID &sessionClientId );
private:
    SResult MethodCall(
        const CStdString& method, 
        const CVariant& params, 
        OUT CVariant& result);

#pragma endregion sync action

#pragma region async action

public:
    void MethodSend( 
        const SimpleActionPtr& simpleAction,
        const CStdString& method, 
        const CVariant& params);
    void OnResponse( 
        const SimpleActionID simpleActionId,
        const SResult sResult,
        const CVariant& response);
    void OnNotification( 
        const CVariant& notification);
private:
    void AddSimpleAction(const SimpleActionPtr& simpleAction);
    SimpleActionPtr TakeSimpleAction( const SimpleActionID simpleActionId );
    CCriticalSection m_mutex;
    SimpleActionList m_simpleActionList;

#pragma endregion async action

public:

    CSessionClientConnection( void );
    //Copying a CCriticalSection is not allowed, so copy everything but that
    //when adding a member variable, make sure to copy it in CJsonClient::Copy
    CSessionClientConnection(const CSessionClientConnection& client);
    CSessionClientConnection& operator=(const CSessionClientConnection& client);
    virtual ~CSessionClientConnection() { }

    bool ContentBlue(const CStdString& host, const int port);
    bool ContentTCP(const CStdString& host, const int port);
    void Stop( void );

    /*** OVERRIDE ***/
    virtual SResult Send(const char *data, unsigned int size);
    virtual void PushBuffer(const char *buffer, int length);
    virtual bool Closing() const { return false; }
private:
    virtual void Connect();
    virtual void Disconnect();
    void ClearSimpleActionForDisconnect();
    void ClearSimpleActionForTimeout();

    SOCKET              m_socket;
    sockaddr_storage    m_cliaddr;
    socklen_t           m_addrlen;
    CCriticalSection    m_critSection;
    volatile bool       m_connected;

    void HandleIncomingData(const CStdString& data);
protected:
    void Copy(const CSessionClientConnection& client);
private:
    int m_beginBrackets, m_endBrackets;
    char m_beginChar, m_endChar;
    std::string m_buffer;

protected:
    /*** OVERRIDE FROM CThread ***/
    void Process();
};



#endif
