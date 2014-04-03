#include "SessionClientConnection.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "threads/SingleLock.h"
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/Stopwatch.h"
#include "JSONRPCUtils.h"

#include "guilib/GUIMessage.h"
#include "ApplicationMessenger.h"


#ifdef HAVE_LIBBLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

/* The defines BDADDR_ANY and BDADDR_LOCAL are broken so use our own structs */
static const bdaddr_t bt_bdaddr_any   = {{0, 0, 0, 0, 0, 0}};
static const bdaddr_t bt_bdaddr_local = {{0, 0, 0, 0xff, 0xff, 0xff}};

#endif


#define RECEIVEBUFFER 1024


/************************************************************************/
/*  NextSimpleActionID                                                  */
/************************************************************************/

class NextSimpleActionID
{
public:
	NextSimpleActionID( void );
	SimpleActionID Get( void );
private:
	CCriticalSection m_mutex;
	SimpleActionID m_simpleActionId;
};

const SimpleActionID kMinSimpleActionID = 50;
NextSimpleActionID::NextSimpleActionID( void )
	: m_simpleActionId(kMinSimpleActionID)
{

}

SimpleActionID NextSimpleActionID::Get( void )
{
	CSingleLock _lock(m_mutex);
	if (m_simpleActionId < kMinSimpleActionID )
		m_simpleActionId = kMinSimpleActionID;
	return m_simpleActionId++;
}

NextSimpleActionID nextSimpleActionId;

/************************************************************************/
/*  CSimpleAction                                                       */
/************************************************************************/

CSimpleAction::CSimpleAction()
	: m_bIsBusy(false)
	, m_sResult(rmrUnknown)
	, m_event(NULL)
{
	m_simpleActionId = nextSimpleActionId.Get();
	m_tickCount = XbmcThreads::SystemClockMillis();
}

bool CSimpleAction::IsTimeout( const unsigned int tickCount ) const
{
	const int kTimeout = 15000;
	return ( tickCount - m_tickCount > kTimeout);
}

void CSimpleAction::SetSessionClient(const SessionClientConnectionPtr& sessionClient)
{
	m_sessionClient = sessionClient;
}

void CSimpleAction::DoCancel( void )
{
	m_sessionClient->OnResponse(simpleActionId(), rmrCancelWaitingForResponse, CVariant::VariantTypeConstNull);
}

void CSimpleAction::MethodSend( 
	const CStdString& method, 
	const CVariant& params )
{
	assert(!m_bIsBusy);
	m_bIsBusy = true;

	m_sessionClient->MethodSend(shared_from_this(), method, params);
}

void CSimpleAction::SetEvent(const SimpleActionEvent& event)
{
	CSingleLock _lock(m_mutexEvent);
	m_event = event;
}

void CSimpleAction::OnResponse(const SResult sResult, const CVariant& response)
{
	assert(m_bIsBusy);
	m_sResult = sResult;
	m_response = response;
	m_bIsBusy = false;

	CSingleLock _lock(m_mutexEvent);
	if (m_event) m_event(*this);
}

void CSessionClientConnection::AddSimpleAction(const SimpleActionPtr& simpleAction)
{
	const SimpleActionID simpleActionId = simpleAction->simpleActionId();
	CSingleLock _lock(m_mutex);
#if !defined(NDEBUG)
	const SimpleActionList::iterator found = m_simpleActionList.find(simpleActionId);
	assert(found == m_simpleActionList.end());
#endif
	m_simpleActionList.insert(std::make_pair(simpleActionId, simpleAction));
}

SimpleActionPtr CSessionClientConnection::TakeSimpleAction(const SimpleActionID simpleActionId)
{
	CSingleLock _lock(m_mutex);
	const SimpleActionList::iterator found = m_simpleActionList.find(simpleActionId);
	assert( found != m_simpleActionList.end() );
	if (found == m_simpleActionList.end())
		return SimpleActionPtr();
	// 1. 在请求集里, 获得请求;
	SimpleActionPtr simpleAction = found->second;

	// 2. 清除请求
	m_simpleActionList.erase(found);

	return simpleAction;
}


void CSessionClientConnection::MethodSend( 
	const SimpleActionPtr& simpleAction,
	const CStdString& method, const CVariant& params)
{
	const SimpleActionID simpleActionId = simpleAction->simpleActionId();
	// 1. 请求已经建立

	// 2. 请求加入队列
	AddSimpleAction(simpleAction);

	// 发送请求;
	CVariant request;
	request["jsonrpc"] = "2.0";
	request["id"] = simpleActionId;
	request["method"] = method;
	request["params"] = params;
	const CStdString request_string = CJSONVariantWriter::Write(request, g_advancedSettings.m_jsonOutputCompact);
	const SResult sResult = Send(request_string.c_str(), request_string.length());

	if (sResult != rmrOk) 
	{
		// 发送失败处理;
		OnResponse(simpleActionId, sResult, CVariant::VariantTypeNull);
	}
}

void CSessionClientConnection::OnResponse( 
	const SimpleActionID simpleActionId,
	const SResult sResult,
	const CVariant& response)
{
	// 1. 在请求集里, 获得请求;
	// 2. 清除请求
	const SimpleActionPtr simpleAction = TakeSimpleAction(simpleActionId);

	if (simpleAction)
	{
		// 3. 设置回复信息;
		simpleAction->OnResponse(sResult, response);
	}
}

void CSessionClientConnection::OnNotification( 
	const CVariant& notification)
{
	CStdString method = notification["method"].asString();
	if (method.Equals("callback"))
	{
		const SessionClientID sessionClientId = (SessionClientID)notification["params"]["sessionClientId"].asInteger();
		const SessionActionID sessionActionId = (SessionActionID)notification["params"]["sessionActionId"].asInteger();
		const SessionActionEvent event = TranslateSessionActionEvent(notification["params"]["event"].asString());
		const CStdString arg1 = notification["params"]["arg1"].asString();
		const CStdString arg2 = notification["params"]["arg2"].asString();
		const CStdString arg3 = notification["params"]["arg3"].asString();

		//m_JSONRPCEvent->OnCallback(sessionClientId, sessionActionId, event, arg1, arg2, arg3 );
		return;
	}

	if (method.Equals("notify"))
	{
		const SessionClientID sessionClientId = (SessionClientID)notification["params"]["sessionClientId"].asInteger();
		const NotifyKind kind = TranslateNotifyKind(notification["params"]["kind"].asString());
		const CStdString strEvent = notification["params"]["event"].asString();
		const CStdString arg1 = notification["params"]["arg1"].asString();
		const CStdString arg2 = notification["params"]["arg2"].asString();
		const CStdString arg3 = notification["params"]["arg3"].asString();

		std::vector<CStdString> vecParams;
		vecParams.push_back(strEvent);
		vecParams.push_back(arg1);
		vecParams.push_back(arg2);
		vecParams.push_back(arg3);

		CGUIMessage msg(TMSG_SESSION_CLIENT, 0, 0);
		msg.SetStringParams(vecParams);

		CApplicationMessenger::Get().SendGUIMessage(msg, VDM_WINDOW_HOME, true);

		return;
	}
	std::string typeName; // response["method"].asString();

	const std::string sender = notification["params"]["sender"].asString();
	const CVariant& data = notification["params"]["data"];
	int sPos = method.find('.');
	if (method.npos != sPos)
	{
		typeName = method.substr(0, sPos-1);
		method.erase( method.begin() + sPos );
	}

	//m_JSONRPCEvent->OnNotification(typeName, method, sender, data);
	return;
}

SResult CSessionClientConnection::MethodCall(
	const CStdString& method, 
	const CVariant& params, 
	OUT CVariant& result)
{
	SimpleActionPtr simpleAction(new CSimpleAction);
	simpleAction->SetSessionClient(shared_from_this());
	simpleAction->MethodSend(method, params);
	while (simpleAction->IsBusy()) Sleep(20);
	if ( simpleAction->sResult() != rmrOk )
		return simpleAction->sResult();

	const CVariant& response = simpleAction->response();
	if (response.isMember("error"))
	{
		using JSONRPC::JSONRPC_STATUS;
		JSONRPC_STATUS status = (JSONRPC_STATUS)response["error"]["code"].asInteger();
		if (status == JSONRPC::UserError)
		{
			return (SResult)response["error"]["data"]["SResult"]["code"].asInteger();
		}
		return JSONRPC::ToJsonRpcResult(status);
	}
	else if (!response.isMember("result"))
	{
		return JSONRPC::jsrError;
	}

	result = response.isMember("result");
	return rmrOk;
}



SResult CSessionClientConnection::SessionClientConnect( 
	const SessionClientID &sessionClientId )
{
	CVariant params = CVariant::VariantTypeObject;
	params["sessionClientId"] = sessionClientId;
	CVariant vResult;
	const SResult sResult = MethodCall("VidOnMe.SessionClientConnect", params, vResult);
	if ( sResult != rmrOk ) return sResult;

	//TODO:
	return rmrOk;
}

SResult CSessionClientConnection::SessionClientDisconnect( 
	const SessionClientID &sessionClientId )
{
	CVariant params = CVariant::VariantTypeObject;
	params["sessionClientId"] = sessionClientId;
	CVariant vResult;
	const SResult sResult = MethodCall("VidOnMe.SessionClientDisconnect", params, vResult);
	if ( sResult != rmrOk ) return sResult;

	//TODO:
	return rmrOk;
}

CSessionClientConnection::CSessionClientConnection( void )
	: CThread("CSessionConnection")
{

	m_socket = INVALID_SOCKET;
	m_connected = false;
	m_beginBrackets = 0;
	m_endBrackets = 0;
	m_beginChar = 0;
	m_endChar = 0;

	m_addrlen = sizeof(m_cliaddr);
}

CSessionClientConnection::CSessionClientConnection(const CSessionClientConnection& client)
	: CThread("CSessionClientConnection")
{
	Copy(client);
}

CSessionClientConnection& CSessionClientConnection::operator=(const CSessionClientConnection& client)
{
	Copy(client);
	return *this;
}

bool CSessionClientConnection::ContentBlue(const CStdString& host, const int port)
{
#ifdef _WIN32

	SOCKET fd = socket (AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);
	if(fd == INVALID_SOCKET)
	{
		CLog::Log(LOGINFO, "JSONRPC Client: Unable to get bluetooth socket");
		return false;
	}
	SOCKADDR_BTH sa  = {};
	sa.addressFamily = AF_BTH;
	//sa.btAddr        = dfd;
	sa.port          = BT_PORT_ANY;

	if(connect(fd, (SOCKADDR*)&sa, sizeof(sa)) < 0)
	{
		CLog::Log(LOGINFO, "JSONRPC Client: Unable to connect to bluetooth socket");
		closesocket(fd);
		return false;
	}

	ULONG optval = TRUE;
	if(setsockopt(fd, SOL_RFCOMM, SO_BTH_AUTHENTICATE, (const char*)&optval, sizeof(optval)) == SOCKET_ERROR)
	{
		CLog::Log(LOGERROR, "JSONRPC Client: Failed to force authentication for bluetooth socket");
		closesocket(fd);
		return false;
	}

	int len = sizeof(sa);
	if(getsockname(fd, (SOCKADDR*)&sa, &len) < 0)
		CLog::Log(LOGERROR, "JSONRPC Client: Failed to get bluetooth port");

	m_socket = fd;
	Connect();
	return true;
#endif

#ifdef HAVE_LIBBLUETOOTH

	SOCKET fd = socket (AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
	if(fd == INVALID_SOCKET)
	{
		CLog::Log(LOGINFO, "JSONRPC Server: Unable to get bluetooth socket");
		return false;
	}
	struct sockaddr_rc sa  = {0};
	sa.rc_family  = AF_BLUETOOTH;
	sa.rc_bdaddr  = bt_bdaddr_any;
	sa.rc_channel = 0;

	if(bind(fd, (struct sockaddr*)&sa, sizeof(sa)) < 0)
	{
		CLog::Log(LOGINFO, "JSONRPC Server: Unable to bind to bluetooth socket");
		closesocket(fd);
		return false;
	}

	socklen_t len = sizeof(sa);
	if(getsockname(fd, (struct sockaddr*)&sa, &len) < 0)
		CLog::Log(LOGERROR, "JSONRPC Server: Failed to get bluetooth port");

	if (listen(fd, 10) < 0)
	{
		CLog::Log(LOGERROR, "JSONRPC Server: Failed to listen to bluetooth port %d", sa.rc_channel);
		closesocket(fd);
		return false;
	}

	m_servers.push_back(fd);

	return true;
#endif
	return false;
}

bool CSessionClientConnection::ContentTCP(const CStdString& host, const int port)
{
	struct sockaddr_in cliaddr;
	memset(&cliaddr, 0, sizeof(cliaddr));

	cliaddr.sin_family = AF_INET;
	inet_pton(AF_INET, host.c_str(), &cliaddr.sin_addr.s_addr);
	cliaddr.sin_port = htons(port);

	SOCKET fd = socket(PF_INET, SOCK_STREAM, 0);

	if (fd == INVALID_SOCKET)
	{
		CLog::Log(LOGERROR, "JSONRPC Client: Failed to create serversocket");
		return false;
	}

	if (connect(fd, (struct sockaddr*)&cliaddr, sizeof cliaddr) < 0)
	{
		CLog::Log(LOGERROR, "JSONRPC Client: Failed to connect serversocket");
		closesocket(fd);
		return false;
	}

	m_socket = fd;
	Connect();
	return true;
}

SResult CSessionClientConnection::Send(const char *data, unsigned int size)
{
	CSingleLock lock (m_critSection);

	unsigned int sent = 0;
	do
	{
		int tmp = send(m_socket, data, size - sent, 0);
		sent += tmp;
	} while (sent < size);

	return rmrOk;
}

void CSessionClientConnection::PushBuffer(const char *buffer, int length)
{
	for (int i = 0; i < length; i++)
	{
		char c = buffer[i];

		if (m_beginChar == 0 && c == '{')
		{
			m_beginChar = '{';
			m_endChar = '}';
		}
		else if (m_beginChar == 0 && c == '[')
		{
			m_beginChar = '[';
			m_endChar = ']';
		}

		if (m_beginChar != 0)
		{
			m_buffer.push_back(c);
			if (c == m_beginChar)
				m_beginBrackets++;
			else if (c == m_endChar)
				m_endBrackets++;
			if (m_beginBrackets > 0 && m_endBrackets > 0 && m_beginBrackets == m_endBrackets)
			{
				HandleIncomingData(m_buffer);
				m_beginChar = m_beginBrackets = m_endBrackets = 0;
				m_buffer.clear();
			}
		}
	}
}

void CSessionClientConnection::HandleIncomingData(const CStdString& data)
{

	CVariant varData = CJSONVariantParser::Parse((unsigned char*) data.c_str(), data.length());

	if (varData.isMember("method") && !varData.isMember("id"))
	{
		OnNotification(varData);
	}
	else if (varData.isMember("result") || varData.isMember("error"))
	{
		// 等待对象;
		const SimpleActionID simpleActionId = 
			(SimpleActionID)varData["id"].asInteger();
		OnResponse(simpleActionId, rmrOk, varData);
	}
	else
	{
		CLog::Log(LOGINFO, "%s: no handler json-rpc message(%s)", __FUNCTION__, data.c_str() );
	}
}


void CSessionClientConnection::Connect()
{
	m_connected = true;
	CThread::Create(false);
}

void CSessionClientConnection::Disconnect()
{
	m_connected = false;

	if (m_socket > 0)
	{
		CSingleLock lock (m_critSection);
		shutdown(m_socket, SHUT_RDWR);
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}




void CSessionClientConnection::Copy(const CSessionClientConnection& client)
{
	m_socket            = client.m_socket;
	m_cliaddr           = client.m_cliaddr;
	m_addrlen           = client.m_addrlen;
	m_connected         = client.m_connected;
	m_beginBrackets     = client.m_beginBrackets;
	m_endBrackets       = client.m_endBrackets;
	m_beginChar         = client.m_beginChar;
	m_endChar           = client.m_endChar;
	m_buffer            = client.m_buffer;
}

void CSessionClientConnection::Stop( void )
{
	CThread::StopThread(true);
}

void CSessionClientConnection::ClearSimpleActionForTimeout( void )
{
	const unsigned int currTickCount = XbmcThreads::SystemClockMillis();

	CSingleLock _lock(m_mutex);

	while (true)
	{
		SimpleActionList::iterator found = m_simpleActionList.end();
		for (SimpleActionList::iterator itor = m_simpleActionList.begin();
			itor != m_simpleActionList.end(); itor++)
		{
			if ( itor->second->IsTimeout(currTickCount) )
			{
				found = itor;
				break;
			}
		}

		if ( found == m_simpleActionList.end() ) break;

		found->second->OnResponse(rmrTimeoutWaitingForResponse, CVariant::VariantTypeNull);
		m_simpleActionList.erase(found);
	}
}

void CSessionClientConnection::ClearSimpleActionForDisconnect( void )
{
	CSingleLock lock (m_mutex);
	for (SimpleActionList::iterator itor = m_simpleActionList.begin();
		itor != m_simpleActionList.end(); ++itor )
	{
		itor->second->OnResponse(rmrDisconnectWaitingForResponse, CVariant::VariantTypeNull);
	}
	m_simpleActionList.clear();

	//OnSessionClientNotify(sessionClientId, kind, event, arg1, arg2, arg3);
}

void CSessionClientConnection::Process()
{
	CStopWatch slowTimer;
	slowTimer.StartZero();

	m_bStop = false;
	while (!m_bStop)
	{
		if ( slowTimer.GetElapsedMilliseconds() > 500 )
		{
			slowTimer.Reset();
			ClearSimpleActionForTimeout();
		}

		fd_set          rfds;
		struct timeval  to     = {1, 0};
		FD_ZERO(&rfds);
		FD_SET(m_socket, &rfds);
		const int res = select(m_socket + 1, &rfds, NULL, NULL, &to);
		if (res < 0) break;

		if (res > 0)
		{
			char buffer[RECEIVEBUFFER] = {};
			const int  nread = recv(m_socket, (char*)&buffer, RECEIVEBUFFER, 0);
			bool close = true;
			if (nread > 0)
			{
				PushBuffer(buffer, nread);
				close = Closing();
			}

			if (close) break;
		}
	}

	Disconnect();
	ClearSimpleActionForDisconnect();
}


