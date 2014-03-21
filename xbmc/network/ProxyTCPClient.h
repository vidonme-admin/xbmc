

#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "system.h"
#include <vector>
#include <sys/socket.h>

#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "websocket/WebSocket.h"
#include "utils/StdString.h"

namespace JSONRPC
{
    class IProxyClient
    {
    public:
        virtual SOCKET GetSocket( void ) const = 0;             
        virtual void Send(const char *data, unsigned int size) = 0;
        virtual void PushBuffer(const char *buffer, int length = 0) = 0;
        virtual bool connected( void ) const = 0;
        virtual void Disconnect() = 0;
        virtual bool Closing() const = 0;
        virtual const char* host() const = 0;
        virtual int port( void ) const = 0;
    };

    class IProxyClientEvent
    {
    public:
        virtual void OnDisconnect() = 0;
        virtual void OnPushBuffer(const CStdString& data) = 0;
    };

    class CProxyTCPClientPool : public CThread
    {
    public:
        static bool StartPool( void );
        static void StopPool(bool bWait);
        static void AddClient(IProxyClient* const client);
        static void RemoveClient(IProxyClient* const client);
    protected:
        void Process();
    private:
        CProxyTCPClientPool();
        bool Initialize();
        void Deinitialize();


    std::vector<IProxyClient*> m_connections;
    static CProxyTCPClientPool *s_inst;

  };

  class CProxyTCPClient: public IProxyClient
  {
  public:
      CProxyTCPClient( void );
      //Copying a CCriticalSection is not allowed, so copy everything but that
      //when adding a member variable, make sure to copy it in CJsonClient::Copy
      CProxyTCPClient(const CProxyTCPClient& client);
      CProxyTCPClient& operator=(const CProxyTCPClient& client);
      virtual ~CProxyTCPClient() { Deinitialize(); };
      IProxyClientEvent* clientEvent(void) const {return m_clientEvent;}
      void SetClientEvent(IProxyClientEvent*const clientEvent){m_clientEvent = clientEvent;}

      void Initialize();
      void Deinitialize();
      bool ContentBlue(const CStdString& host, const int port);
      bool ContentTCP(const CStdString& host, const int port);

      /*** OVERRIDE ***/
      virtual void Send(const char *data, unsigned int size);
      virtual void PushBuffer(const char *buffer, int length);
      virtual void Connect();
      virtual void Disconnect();
      virtual SOCKET GetSocket( void ) const {return m_socket;}
      virtual bool connected( void ) const {return m_connected;};
      virtual bool Closing() const { return false; }
      virtual const char* host() const{return m_host.c_str();};
      virtual int port( void ) const{return m_port;}
  private:
      SOCKET           m_socket;
      sockaddr_storage m_cliaddr;
      socklen_t        m_addrlen;
      CCriticalSection m_critSection;
      bool m_connected;

      CStdString m_host;
      int m_port;
  protected:
      void Copy(const CProxyTCPClient& client);
  private:
      int m_beginBrackets, m_endBrackets;
      char m_beginChar, m_endChar;
      std::string m_buffer;
      IProxyClientEvent* m_clientEvent;
  };

  class CProxyWebSocketClient : public CProxyTCPClient
  {
  public:
      CProxyWebSocketClient(CWebSocket *websocket);
      CProxyWebSocketClient(const CProxyWebSocketClient& client);
      CProxyWebSocketClient(CWebSocket *websocket, const CProxyTCPClient& client);
      CProxyWebSocketClient& operator=(const CProxyWebSocketClient& client);
      ~CProxyWebSocketClient();

      virtual void Send(const char *data, unsigned int size);
      virtual void PushBuffer(const char *buffer, int length);
      virtual void Connect();
      virtual void Disconnect();

      virtual bool Closing() const { return m_websocket != NULL && m_websocket->GetState() == WebSocketStateClosed; }

  private:
      CWebSocket *m_websocket;
  };
}

#endif
