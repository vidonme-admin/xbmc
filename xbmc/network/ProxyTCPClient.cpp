
#if defined(__VIDONME_MEDIACENTER__)

#include "ProxyTCPClient.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/Variant.h"
#include "threads/SingleLock.h"
#include "websocket/WebSocketManager.h"

#ifdef HAVE_LIBBLUETOOTH
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

 /* The defines BDADDR_ANY and BDADDR_LOCAL are broken so use our own structs */
static const bdaddr_t bt_bdaddr_any   = {{0, 0, 0, 0, 0, 0}};
static const bdaddr_t bt_bdaddr_local = {{0, 0, 0, 0xff, 0xff, 0xff}};

#endif

using namespace JSONRPC;
//using namespace std; On VS2010, bind conflicts with std::bind

#define RECEIVEBUFFER 1024

CProxyTCPClientPool *CProxyTCPClientPool::s_inst = NULL;

bool CProxyTCPClientPool::StartPool( void )
{
  StopPool(true);

  s_inst = new CProxyTCPClientPool();
  if (s_inst->Initialize())
  {
    s_inst->Create();
    return true;
  }
  else
    return false;
}

void CProxyTCPClientPool::StopPool(bool bWait)
{
  if (s_inst)
  {
    s_inst->StopThread(bWait);
    if (bWait)
    {
      delete s_inst;
      s_inst = NULL;
    }
  }
}

CProxyTCPClientPool::CProxyTCPClientPool( ) : CThread("CJsonClientPool")
{

}

void CProxyTCPClientPool::Process()
{
    m_bStop = false;

    while (!m_bStop)
    {
        SOCKET          max_fd = 0;
        fd_set          rfds;
        struct timeval  to     = {1, 0};
        FD_ZERO(&rfds);

        bool hasConnection = false;
        
        for (unsigned int i = 0; i < m_connections.size(); i++)
        {
            if ( !m_connections[i]->connected() ) continue;

            hasConnection = true;

            FD_SET(m_connections[i]->GetSocket(), &rfds);
            if ((intptr_t)m_connections[i]->GetSocket() > (intptr_t)max_fd)
                max_fd = m_connections[i]->GetSocket();
        }

        if ( ! hasConnection )
        {
            Sleep(200);
            continue;            
        }

        int res = select((intptr_t)max_fd+1, &rfds, NULL, NULL, &to);
        if (res < 0)
        {
            CLog::Log(LOGERROR, "JSONRPC Server: Select failed");
            Sleep(1000);
            Initialize();
        }
        else if (res > 0)
        {
            for (int i = m_connections.size() - 1; i >= 0; i--)
            {
                if ( !m_connections[i]->connected() ) continue;

                int socket = m_connections[i]->GetSocket();
                if (FD_ISSET(socket, &rfds))
                {
                    char buffer[RECEIVEBUFFER] = {};
                    int  nread = 0;
                    nread = recv(socket, (char*)&buffer, RECEIVEBUFFER, 0);
                    bool close = false;
                    if (nread > 0)
                    {
                        m_connections[i]->PushBuffer(buffer, nread);
                        close = m_connections[i]->Closing();
                    }
                    else
                        close = true;

                    if (close)
                    {
                        CLog::Log(LOGINFO, "JSONRPC Client: Disconnection detected");
                        m_connections[i]->Disconnect();
                        //delete m_connections[i];
                        //m_connections.erase(m_connections.begin() + i);
                    }

                }
            }
        }
    }
    Deinitialize();
}

bool CProxyTCPClientPool::Initialize()
{
  Deinitialize();

  CLog::Log(LOGINFO, "JSONRPC ClientPool: Successfully initialized");
  return true;
}

void CProxyTCPClientPool::Deinitialize()
{
  for (unsigned int i = 0; i < m_connections.size(); i++)
  {
    m_connections[i]->Disconnect();
    //delete m_connections[i];
  }
  //m_connections.clear();
}

void CProxyTCPClientPool::AddClient(IProxyClient* const client)
{
    if (s_inst)
    {
        std::vector<IProxyClient*>& connections = s_inst->m_connections;
        connections.push_back(client);
    }
    
}
void CProxyTCPClientPool::RemoveClient(IProxyClient* const client)
{
    if (s_inst)
    {
        std::vector<IProxyClient*>& connections = s_inst->m_connections;
        std::vector<IProxyClient*>::iterator found = std::find(connections.begin(), connections.end(), client);
        if (connections.end()!=found)
            connections.erase(found);
    }
}

CProxyTCPClient::CProxyTCPClient( void )
{
  m_clientEvent = NULL;
  m_socket = INVALID_SOCKET;
  m_connected = false;
  m_beginBrackets = 0;
  m_endBrackets = 0;
  m_beginChar = 0;
  m_endChar = 0;

  m_host = "";
  m_port = 0;

  m_addrlen = sizeof(m_cliaddr);
}

CProxyTCPClient::CProxyTCPClient(const CProxyTCPClient& client)
{
  Copy(client);
}

CProxyTCPClient& CProxyTCPClient::operator=(const CProxyTCPClient& client)
{
  Copy(client);
  return *this;
}

void CProxyTCPClient::Initialize()
{
    Deinitialize();

    CProxyTCPClientPool::AddClient(this);
    CLog::Log(LOGINFO, "JSONRPC ClientPool: Successfully initialized");
}

void CProxyTCPClient::Deinitialize()
{
    CProxyTCPClientPool::RemoveClient(this);
}

bool CProxyTCPClient::ContentBlue(const CStdString& host, const int port)
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
    m_host = host;
    m_port = port;
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

bool CProxyTCPClient::ContentTCP(const CStdString& host, const int port)
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
    m_host = host;
    m_port = port;
    Connect();
    return true;
}

void CProxyTCPClient::Send(const char *data, unsigned int size)
{
  unsigned int sent = 0;
  do
  {
    CSingleLock lock (m_critSection);
    sent += send(m_socket, data, size - sent, 0);
  } while (sent < size);
}

void CProxyTCPClient::PushBuffer(const char *buffer, int length)
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
        if (m_clientEvent)
          m_clientEvent->OnPushBuffer(m_buffer);
        m_beginChar = m_beginBrackets = m_endBrackets = 0;
        m_buffer.clear();
      }
    }
  }
}

void CProxyTCPClient::Connect()
{
    m_connected = true;
}

void CProxyTCPClient::Disconnect()
{
    m_connected = false;
    if (m_clientEvent)
        m_clientEvent->OnDisconnect();

    if (m_socket > 0)
    {
        CSingleLock lock (m_critSection);
        shutdown(m_socket, SHUT_RDWR);
        closesocket(m_socket);
        m_socket = INVALID_SOCKET;
    }
}

void CProxyTCPClient::Copy(const CProxyTCPClient& client)
{
  m_clientEvent       = client.m_clientEvent;
  m_socket            = client.m_socket;
  m_cliaddr           = client.m_cliaddr;
  m_addrlen           = client.m_addrlen;
  m_connected         = client.m_connected;
  m_beginBrackets     = client.m_beginBrackets;
  m_endBrackets       = client.m_endBrackets;
  m_beginChar         = client.m_beginChar;
  m_endChar           = client.m_endChar;
  m_buffer            = client.m_buffer;

  m_host              = client.m_host;
  m_port              = client.m_port;
}

CProxyWebSocketClient::CProxyWebSocketClient(CWebSocket *websocket)
{
  m_websocket = websocket;
}

CProxyWebSocketClient::CProxyWebSocketClient(const CProxyWebSocketClient& client):CProxyTCPClient(client)
{
  *this = client;
}

CProxyWebSocketClient::CProxyWebSocketClient(CWebSocket *websocket, const CProxyTCPClient& client)
{
  //Copy(client);

  m_websocket = websocket;
}

CProxyWebSocketClient::~CProxyWebSocketClient()
{
  delete m_websocket;
}

CProxyWebSocketClient& CProxyWebSocketClient::operator=(const CProxyWebSocketClient& client)
{
  Copy(client);

  m_websocket = client.m_websocket;

  return *this;
}

void CProxyWebSocketClient::Send(const char *data, unsigned int size)
{
  const CWebSocketMessage *msg = m_websocket->Send(WebSocketTextFrame, data, size);
  if (msg == NULL || !msg->IsComplete())
    return;

  std::vector<const CWebSocketFrame *> frames = msg->GetFrames();
  for (unsigned int index = 0; index < frames.size(); index++)
    CProxyTCPClient::Send(frames.at(index)->GetFrameData(), (unsigned int)frames.at(index)->GetFrameLength());
}

void CProxyWebSocketClient::PushBuffer(const char *buffer, int length)
{
  bool send;
  const CWebSocketMessage *msg;
  if ((msg = m_websocket->Handle(buffer, length, send)) != NULL && msg->IsComplete())
  {
    assert(!send);

    std::vector<const CWebSocketFrame *> frames = msg->GetFrames();
    if (send)
    {
      for (unsigned int index = 0; index < frames.size(); index++)
        Send(frames.at(index)->GetFrameData(), (unsigned int)frames.at(index)->GetFrameLength());
    }
    else
    {
      for (unsigned int index = 0; index < frames.size(); index++)
        CProxyTCPClient::PushBuffer(frames.at(index)->GetApplicationData(), (int)frames.at(index)->GetLength());
    }

    delete msg;
  }

  if (m_websocket->GetState() == WebSocketStateClosed)
    Disconnect();
}

void CProxyWebSocketClient::Connect()
{
    CProxyTCPClient::Connect();
    char xxStr[] =    
        "GET /chat HTTP/1.1\r\n"
        "Host: server.example.com\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Origin: http://example.com\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "Sec-WebSocket-Protocol: chat, superchat\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n";

    CProxyTCPClient::Send(xxStr, sizeof(xxStr)-1);


/*
HTTP/1.1 101 Switching Protocols
Upgrade: websocket
Connection: Upgrade
Sec-WebSocket-Accept: s3pPLMBiTxaQ9kYGzzhZRbK+xOo=
Sec-WebSocket-Protocol: chat
*/
}

void CProxyWebSocketClient::Disconnect()
{
  if (GetSocket() > 0)
  {
    if (m_websocket->GetState() != WebSocketStateClosed && m_websocket->GetState() != WebSocketStateNotConnected)
    {
      const CWebSocketFrame *closeFrame = m_websocket->Close();
      if (closeFrame)
        Send(closeFrame->GetFrameData(), (unsigned int)closeFrame->GetFrameLength());
    }

    if (m_websocket->GetState() == WebSocketStateClosed)
      CProxyTCPClient::Disconnect();
  }
}

#endif