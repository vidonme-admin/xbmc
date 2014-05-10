/*
*      Copyright (C) 2005-2013 Team VidOn.me
*      http://www.vidon.me
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with VidOn.me; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#include "ZeroconfDNS.h"

#include <string>
#include <sstream>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <threads/SingleLock.h>
#include <utils/log.h>
#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/LocalizeStrings.h"

CZeroconfDNS::CZeroconfDNS()
  : CThread("DNSResultsProcess")
{
  m_service = NULL;
}

CZeroconfDNS::~CZeroconfDNS()
{
  doStop();
  StopThread();
}

bool CZeroconfDNS::IsZCdaemonRunning()
{
  uint32_t version;
  uint32_t size = sizeof(version);
  DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &version, &size);
  if(err != kDNSServiceErr_NoError)
  {
    CLog::Log(LOGERROR, "ZeroconfDNS: Zeroconf can't be started probably because Apple's Bonjour Service isn't installed. You can get it by either installing Itunes or Apple's Bonjour Print Service for Windows (http://support.apple.com/kb/DL999)");
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, g_localizeStrings.Get(34300), g_localizeStrings.Get(34301), 10000, true);
    return false;
  }
  CLog::Log(LOGDEBUG, "ZeroconfDNS:Bonjour version is %d.%d", version / 10000, version / 100 % 100);
  return true;
}

//methods to implement for concrete implementations
bool CZeroconfDNS::doPublishService(const std::string& fcr_identifier,
  const std::string& fcr_type,
  const std::string& fcr_name,
  unsigned int f_port,
  const std::vector<std::pair<std::string, std::string> >& txt)
{
  DNSServiceRef netService = NULL;
  TXTRecordRef txtRecord;
  DNSServiceErrorType err;
  TXTRecordCreate(&txtRecord, 0, NULL);

  if(m_service == NULL)
  {
    err = DNSServiceCreateConnection(&m_service);
    if (err != kDNSServiceErr_NoError)
    {
      CLog::Log(LOGERROR, "ZeroconfDNS: DNSServiceCreateConnection failed with error = %ld", (int) err);
      return false;
    }

    CLog::Log(LOGINFO, "ZeroconfDNS: DNSServiceCreateConnection successful!");
#if 0
    err = WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_service ), g_hWnd, BONJOUR_EVENT, FD_READ | FD_CLOSE );
    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfDNS: WSAAsyncSelect failed with error = %ld", (int) err);
#else
    m_thread_stop = false;
#if defined(TARGET_ANDROID)
    //temporarily removed
#else
    Create();
#endif
#endif
  }

  CLog::Log(LOGDEBUG, "ZeroconfDNS: identifier: %s type: %s name:%s port:%i", fcr_identifier.c_str(), fcr_type.c_str(), fcr_name.c_str(), f_port);

  //add txt records
  if(!txt.empty())
  {
    for(std::vector<std::pair<std::string, std::string> >::const_iterator it = txt.begin(); it != txt.end(); ++it)
    {
      CLog::Log(LOGDEBUG, "ZeroconfDNS: key:%s, value:%s",it->first.c_str(),it->second.c_str());
      uint8_t txtLen = (uint8_t)strlen(it->second.c_str());
      TXTRecordSetValue(&txtRecord, it->first.c_str(), txtLen, it->second.c_str());
    }
  }

  {
    CSingleLock lock(m_data_guard);
    netService = m_service;
    err = DNSServiceRegister(&netService, kDNSServiceFlagsShareConnection, 0, fcr_name.c_str(), fcr_type.c_str(), NULL, NULL, htons(f_port), TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord), registerCallback, NULL);
  }

  if (err != kDNSServiceErr_NoError)
  {
    // Something went wrong so lets clean up.
    if (netService)
      DNSServiceRefDeallocate(netService);

    CLog::Log(LOGERROR, "ZeroconfDNS: DNSServiceRegister returned (error = %ld)", (int) err);
  }
  else
  {
    CSingleLock lock(m_data_guard);
    m_services.insert(make_pair(fcr_identifier, netService));
  }

  TXTRecordDeallocate(&txtRecord);

  return err == kDNSServiceErr_NoError;
}

bool CZeroconfDNS::doRemoveService(const std::string& fcr_ident)
{
  CSingleLock lock(m_data_guard);
  tServiceMap::iterator it = m_services.find(fcr_ident);
  if(it != m_services.end())
  {
    DNSServiceRefDeallocate(it->second);
    m_services.erase(it);
    CLog::Log(LOGDEBUG, "ZeroconfDNS: Removed service %s", fcr_ident.c_str());
    return true;
  }
  else
    return false;
}

void CZeroconfDNS::doStop()
{
  {
    CSingleLock lock(m_data_guard);
    CLog::Log(LOGDEBUG, "ZeroconfDNS: Shutdown services");
    for(tServiceMap::iterator it = m_services.begin(); it != m_services.end(); ++it)
    {
      DNSServiceRefDeallocate(it->second);
      CLog::Log(LOGDEBUG, "ZeroconfDNS: Removed service %s", it->first.c_str());
    }
    m_services.clear();
  }
  {
    StopThread();
    CSingleLock lock(m_data_guard);
#if 0
    WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_service ), g_hWnd, BONJOUR_EVENT, 0 );
#endif
    DNSServiceRefDeallocate(m_service);
    m_service = NULL;
  }
}

void DNSSD_API CZeroconfDNS::registerCallback(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context)
{
  (void)sdref;    // Unused
  (void)flags;    // Unused
  (void)context;  // Unused

  if (errorCode == kDNSServiceErr_NoError)
  {
    if (flags & kDNSServiceFlagsAdd)
      CLog::Log(LOGDEBUG, "ZeroconfDNS: %s.%s%s now registered and active", name, regtype, domain);
    else
      CLog::Log(LOGDEBUG, "ZeroconfDNS: %s.%s%s registration removed", name, regtype, domain);
  }
  else if (errorCode == kDNSServiceErr_NameConflict)
    CLog::Log(LOGDEBUG, "ZeroconfDNS: %s.%s%s Name in use, please choose another", name, regtype, domain);
  else
    CLog::Log(LOGDEBUG, "ZeroconfDNS: %s.%s%s error code %d", name, regtype, domain, errorCode);
}

#if 0
void CZeroconfDNS::ProcessResults()
{
  CSingleLock lock(m_data_guard);
  DNSServiceErrorType err = DNSServiceProcessResult(m_service);
  if (err != kDNSServiceErr_NoError)
    CLog::Log(LOGERROR, "ZeroconfDNS: DNSServiceProcessResult returned (error = %ld)", (int) err);
}
#endif

void CZeroconfDNS::Process()
{
  CLog::Log(LOGINFO, "ZeroconfDNS: begin process result thread.");

  int dns_sd_fd = -1;

  {
    CSingleLock lock(m_data_guard);
    dns_sd_fd = DNSServiceRefSockFD(m_service);
  }

  int nfds = dns_sd_fd + 1;
  fd_set readfds;
  struct timeval tv;
  int result;

  bool stopNow = false; 
  {
    CSingleLock lock(m_data_guard);
    stopNow = m_thread_stop;
  }

  while (!stopNow)
  {
    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    FD_ZERO(&readfds);

    // 2. Add the fd for our client(s) to the fd_set
    FD_SET(dns_sd_fd , &readfds);

    // 3. Set up the timeout.
    tv.tv_sec  = 3;
    tv.tv_usec = 0;

    {
      CSingleLock lock(m_data_guard);
      result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
    }

    if (result > 0)
    {
      DNSServiceErrorType err = kDNSServiceErr_NoError;
      if (FD_ISSET(dns_sd_fd , &readfds))
      {
        {
          CSingleLock lock(m_data_guard);
          err = DNSServiceProcessResult(m_service);
        }

        if (err != kDNSServiceErr_NoError)
          CLog::Log(LOGERROR, "ZeroconfDNS: DNSServiceProcessResult returned (error = %ld)", (int) err);
        else
          break;
      }

      if (err)
      { 
        //stopNow = true; 
      }
    }
    else
    {
      if (errno != EINTR) stopNow = true;
    }

    Sleep(2000);

    {
      CSingleLock lock(m_data_guard);
      stopNow = m_thread_stop;
    }
  }
}

void CZeroconfDNS::StopThread(bool bWait)
{
  {
    CSingleLock lock(m_data_guard);
    m_thread_stop = true;
  }

  CThread::StopThread(bWait);
}
