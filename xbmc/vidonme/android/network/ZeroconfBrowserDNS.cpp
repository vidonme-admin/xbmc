/*
*      Copyright (C) 2012-2013 Team VidOn.me
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

#include "ZeroconfBrowserDNS.h"
#include <utils/log.h>
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <threads/SingleLock.h>
#include "guilib/GUIWindowManager.h"
#include "guilib/GUIMessage.h"
#include "GUIUserMessages.h"
#include "network/DNSNameCache.h"


CZeroconfBrowserDNS::CZeroconfBrowserDNS()
  : CThread("DNSBrowserResultsProcess")
{
  m_browser = NULL;
}

CZeroconfBrowserDNS::~CZeroconfBrowserDNS()
{
  StopThread();

  CSingleLock lock(m_data_guard);
  //make sure there are no browsers anymore
  for(tBrowserMap::iterator it = m_service_browsers.begin(); it != m_service_browsers.end(); ++it )
    doRemoveServiceType(it->first);

#if 0
  WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_browser ), g_hWnd, BONJOUR_BROWSER_EVENT, 0 );
#endif

  DNSServiceRefDeallocate(m_browser);
  m_browser = NULL;
}

void DNSSD_API CZeroconfBrowserDNS::BrowserCallback(DNSServiceRef browser,
  DNSServiceFlags flags,
  uint32_t interfaceIndex,
  DNSServiceErrorType errorCode,
  const char *serviceName,
  const char *regtype,
  const char *replyDomain,
  void *context)
{

  if (errorCode == kDNSServiceErr_NoError)
  {
    //get our instance
    CZeroconfBrowserDNS* p_this = reinterpret_cast<CZeroconfBrowserDNS*>(context);
    //store the service
    ZeroconfService s(serviceName, regtype, replyDomain);

    if (flags & kDNSServiceFlagsAdd)
    {
      CLog::Log(LOGDEBUG, "ZeroconfBrowserDNS::BrowserCallback found service named: %s, type: %s, domain: %s", s.GetName().c_str(), s.GetType().c_str(), s.GetDomain().c_str());
      p_this->addDiscoveredService(browser, s);
    }
    else
    {
      CLog::Log(LOGDEBUG, "ZeroconfBrowserDNS::BrowserCallback service named: %s, type: %s, domain: %s disappeared", s.GetName().c_str(), s.GetType().c_str(), s.GetDomain().c_str());
      p_this->removeDiscoveredService(browser, s);
    }
    if(! (flags & kDNSServiceFlagsMoreComing) )
    {
      CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
      message.SetStringParam("zeroconf://");
      g_windowManager.SendThreadMessage(message);
      CLog::Log(LOGDEBUG, "ZeroconfBrowserDNS::BrowserCallback sent gui update for path zeroconf://");
    }
  }
  else
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserDNS::BrowserCallback returned (error = %ld)\n", (int)errorCode);
  }
}

#if defined(__VIDONME_MEDIACENTER__)
void DNSSD_API AddrinfoCallback(DNSServiceRef sdref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address, uint32_t ttl, void *context)
{
  if (errorCode)
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserDNS: AddrinfoCallback failed with error = %ld", (int) errorCode);
    return;
  }

  CLog::Log(LOGINFO, "ZeroconfBrowserDNS: AddrinfoCallback begin");

  DNSServiceErrorType err;
  CStdString strIP;
  CZeroconfBrowser::ZeroconfService* service = (CZeroconfBrowser::ZeroconfService*) context;
  char addr[256] = "";

  CLog::Log(LOGINFO, "ZeroconfBrowserDNS: AddrinfoCallback before parse IP, family: %d", address->sa_family);
  if (address && address->sa_family == AF_INET)
  {

    CLog::Log(LOGINFO, "ZeroconfBrowserDNS: AddrinfoCallback begin");
    const unsigned char *b = (const unsigned char *) &((struct sockaddr_in *)address)->sin_addr;
    snprintf(addr, sizeof(addr), "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
    CLog::Log(LOGINFO, "ZeroconfBrowserDNS: AddrinfoCallback result, ip: %s", addr);

  }
  strIP = addr;
  service->SetIP(strIP);
}
#endif

void DNSSD_API CZeroconfBrowserDNS::ResolveCallback(DNSServiceRef                       sdRef,
  DNSServiceFlags                     flags,
  uint32_t                            interfaceIndex,
  DNSServiceErrorType                 errorCode,
  const char                          *fullname,
  const char                          *hosttarget,
  uint16_t                            port,        /* In network byte order */
  uint16_t                            txtLen,
  const unsigned char                 *txtRecord,
  void                                *context
  )
{

  if (errorCode)
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserDNS: ResolveCallback failed with error = %ld", (int) errorCode);
    return;
  }

  DNSServiceErrorType err;
  CZeroconfBrowser::ZeroconfService::tTxtRecordMap recordMap; 
  CStdString strIP;
  CZeroconfBrowser::ZeroconfService* service = (CZeroconfBrowser::ZeroconfService*) context;

#if defined(__VIDONME_MEDIACENTER__)
  {
    DNSServiceErrorType err;
    DNSServiceRef sdRef = NULL;

    CLog::Log(LOGINFO, "ZeroconfBrowserDNS: DNSServiceGetAddrInfo begin");
    err = DNSServiceGetAddrInfo(&sdRef, kDNSServiceFlagsReturnIntermediates, kDNSServiceInterfaceIndexAny,kDNSServiceProtocol_IPv4, hosttarget, AddrinfoCallback, service);
    if( err != kDNSServiceErr_NoError )
    {
      if (sdRef)
        DNSServiceRefDeallocate(sdRef);

      CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceGetAddrInfo returned (error = %ld)", (int) err);
      return;
    }
    err = DNSServiceProcessResult(sdRef);

    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfBrowserDNS::doGetAddrInfoService DNSServiceProcessResult returned (error = %ld)", (int) err);

    if (sdRef)
      DNSServiceRefDeallocate(sdRef);
  }
#else
  if(!CDNSNameCache::Lookup(hosttarget, strIP))
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserDNS: Could not resolve hostname %s",hosttarget);
    return;
  }
  service->SetIP(strIP);
#endif

  for(uint16_t i = 0; i < TXTRecordGetCount(txtLen, txtRecord); ++i)
  {
    char key[256];
    uint8_t valueLen;
    const void *value;
    std::string strvalue;
    err = TXTRecordGetItemAtIndex(txtLen, txtRecord,i ,sizeof(key) , key, &valueLen, &value);
    if(err != kDNSServiceErr_NoError)
      continue;

    if(value != NULL && valueLen > 0)
      strvalue.append((const char *)value, valueLen);

    recordMap.insert(std::make_pair(key, strvalue));
  }
  service->SetTxtRecords(recordMap);
  service->SetPort(ntohs(port));
}

/// adds the service to list of found services
void CZeroconfBrowserDNS::addDiscoveredService(DNSServiceRef browser, CZeroconfBrowser::ZeroconfService const& fcr_service)
{
return;
  CSingleLock lock(m_data_guard);
  tDiscoveredServicesMap::iterator browserIt = m_discovered_services.find(browser);
  if(browserIt == m_discovered_services.end())
  {
    //first service by this browser
    browserIt = m_discovered_services.insert(make_pair(browser, std::vector<std::pair<ZeroconfService, unsigned int> >())).first;
  }
  //search this service
  std::vector<std::pair<ZeroconfService, unsigned int> >& services = browserIt->second;
  std::vector<std::pair<ZeroconfService, unsigned int> >::iterator serviceIt = services.begin();
  for( ; serviceIt != services.end(); ++serviceIt)
  {
    if(serviceIt->first == fcr_service)
      break;
  }
  if(serviceIt == services.end())
    services.push_back(std::make_pair(fcr_service, 1));
  else
    ++serviceIt->second;
}

void CZeroconfBrowserDNS::removeDiscoveredService(DNSServiceRef browser, CZeroconfBrowser::ZeroconfService const& fcr_service)
{
return;
  CSingleLock lock(m_data_guard);
  tDiscoveredServicesMap::iterator browserIt = m_discovered_services.find(browser);
  //search this service
  std::vector<std::pair<ZeroconfService, unsigned int> >& services = browserIt->second;
  std::vector<std::pair<ZeroconfService, unsigned int> >::iterator serviceIt = services.begin();
  for( ; serviceIt != services.end(); ++serviceIt)
    if(serviceIt->first == fcr_service)
      break;
  if(serviceIt != services.end())
  {
    //decrease refCount
    --serviceIt->second;
    if(!serviceIt->second)
    {
      //eventually remove the service
      services.erase(serviceIt);
    }
  } else
  {
    //looks like we missed the announce, no problem though..
  }
}


bool CZeroconfBrowserDNS::doAddServiceType(const CStdString& fcr_service_type)
{
return false;
  DNSServiceErrorType err;
  DNSServiceRef browser = NULL;

  if(m_browser == NULL)
  {
    err = DNSServiceCreateConnection(&m_browser);
    if (err != kDNSServiceErr_NoError)
    {
      CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceCreateConnection failed with error = %ld", (int) err);
      return false;
    }
#if 0
    err = WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_browser ), g_hWnd, BONJOUR_BROWSER_EVENT, FD_READ | FD_CLOSE );
    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfBrowserDNS: WSAAsyncSelect failed with error = %ld", (int) err);
#endif
  }

  m_thread_stop = false;
  if (!IsRunning())
  {
#if defined(TARGET_ANDROID)
    //temporarily removed
#else
    Create();
#endif
  }

  {
    CSingleLock lock(m_data_guard);
    browser = m_browser;
    err = DNSServiceBrowse(&browser, kDNSServiceFlagsShareConnection, kDNSServiceInterfaceIndexAny, fcr_service_type.c_str(), NULL, BrowserCallback, this);
  }

  if( err != kDNSServiceErr_NoError )
  {
    if (browser)
      DNSServiceRefDeallocate(browser);

    CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceBrowse returned (error = %ld)", (int) err);
    return false;
  }

  //store the browser
  {
    CSingleLock lock(m_data_guard);
    m_service_browsers.insert(std::make_pair(fcr_service_type, browser));
  }

  return true;
}

bool CZeroconfBrowserDNS::doRemoveServiceType(const CStdString& fcr_service_type)
{
return false;
  //search for this browser and remove it from the map
  DNSServiceRef browser = 0;
  bool bEmpty = false;
  {
    CSingleLock lock(m_data_guard);
    tBrowserMap::iterator it = m_service_browsers.find(fcr_service_type);
    if(it == m_service_browsers.end())
    {
      return false;
    }
    browser = it->second;
    m_service_browsers.erase(it);
    if (m_service_browsers.empty())
    {
      bEmpty = true;
    }
  }

  if (bEmpty && IsRunning())
  {
    StopThread(true);
  }

  //remove the services of this browser
  {
    CSingleLock lock(m_data_guard);
    tDiscoveredServicesMap::iterator it = m_discovered_services.find(browser);
    if(it != m_discovered_services.end())
      m_discovered_services.erase(it);
  }

  DNSServiceRefDeallocate(browser);

  return true;
}

std::vector<CZeroconfBrowser::ZeroconfService> CZeroconfBrowserDNS::doGetFoundServices()
{
  std::vector<CZeroconfBrowser::ZeroconfService> ret;
  CSingleLock lock(m_data_guard);
  for(tDiscoveredServicesMap::const_iterator it = m_discovered_services.begin();
    it != m_discovered_services.end(); ++it)
  {
    const std::vector<std::pair<CZeroconfBrowser::ZeroconfService, unsigned int> >& services = it->second;
    for(unsigned int i = 0; i < services.size(); ++i)
    {
      ret.push_back(services[i].first);
    }
  }
  return ret;
}

bool CZeroconfBrowserDNS::doResolveService(CZeroconfBrowser::ZeroconfService& fr_service, double f_timeout)
{
return false;
  DNSServiceErrorType err;
  DNSServiceRef sdRef = NULL;

  CLog::Log(LOGINFO, "ZeroconfBrowserDNS: doResolveService begin");
  err = DNSServiceResolve(&sdRef, 0, kDNSServiceInterfaceIndexAny, fr_service.GetName(), fr_service.GetType(), fr_service.GetDomain(), ResolveCallback, &fr_service);

  if( err != kDNSServiceErr_NoError )
  {
    if (sdRef)
      DNSServiceRefDeallocate(sdRef);

    CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceResolve returned (error = %ld)", (int) err);
    return false;
  }

#if defined(__VIDONME_MEDIACENTER__)
  //don't block in DNSServiceProcessResult, timeout is 0.

  int dns_sd_fd = -1;

  if (sdRef)
  {
    dns_sd_fd = DNSServiceRefSockFD(sdRef);
  }

  int nfds = dns_sd_fd + 1;
  fd_set readfds;
  struct timeval tv;
  int result;

  // 1. Set up the fd_set as usual here.
  // This example client has no file descriptors of its own,
  // but a real application would call FD_SET to add them to the set here
  FD_ZERO(&readfds);

  // 2. Add the fd for our client(s) to the fd_set
  if (sdRef) FD_SET(dns_sd_fd , &readfds);

  // 3. Set up the timeout.
  tv.tv_sec  = 1;
  tv.tv_usec = 0;

  CLog::Log(LOGINFO, "ZeroconfBrowserDNS: select begin");
  result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

  if (result > 0)
  {
    CLog::Log(LOGINFO, "ZeroconfBrowserDNS: select successfully!");
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    if (sdRef && FD_ISSET(dns_sd_fd , &readfds))
    {
      CLog::Log(LOGINFO, "ZeroconfBrowserDNS: begin DNSServiceProcessResult! name: %s, type: %s, domain: %s", fr_service.GetName().c_str(), fr_service.GetType().c_str(), fr_service.GetDomain().c_str());
      err = DNSServiceProcessResult(sdRef);
    }

    if (err)
    {
      CLog::Log(LOGINFO, "ZeroconfBrowserDNS: DNSServiceProcessResult returned %d\n", err);
    }
  }
  else
  {
    CLog::Log(LOGINFO, "select() returned %d errno %d %s\n", result, errno, strerror(errno));
  }
#else
  err = DNSServiceProcessResult(sdRef);
#endif//__VIDONME_MEDIACENTER__

  if (err != kDNSServiceErr_NoError)
    CLog::Log(LOGERROR, "ZeroconfBrowserDNS::doResolveService DNSServiceProcessResult returned (error = %ld)", (int) err);

  if (sdRef)
    DNSServiceRefDeallocate(sdRef);

  return true;
}

#if 0
void CZeroconfBrowserDNS::ProcessResults()
{
return;
  CSingleLock lock(m_data_guard);
  DNSServiceErrorType err = DNSServiceProcessResult(m_browser);
  if (err != kDNSServiceErr_NoError)
    CLog::Log(LOGERROR, "ZeroconfWIN: DNSServiceProcessResult returned (error = %ld)", (int) err);
}
#endif

void CZeroconfBrowserDNS::Process()
{

return;

  int dns_sd_fd = -1;

  {
    CSingleLock lock(m_data_guard);
    dns_sd_fd = DNSServiceRefSockFD(m_browser);
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
          err = DNSServiceProcessResult(m_browser);
        }

        if (err != kDNSServiceErr_NoError)
          CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceProcessResult returned (error = %ld)", (int) err);
      }

      if (err)
      { 
        CLog::Log(LOGERROR, "ZeroconfBrowserDNS: DNSServiceProcessResult returned (error = %ld)", (int) err);
        stopNow = true; 
      }
    }
    else
    {
      CLog::Log(LOGERROR, "select() returned %d errno %d %s\n", result, errno, strerror(errno));
    }

    Sleep(3000);

    {
      CSingleLock lock(m_data_guard);
      stopNow = m_thread_stop;
    }
  }

  CLog::Log(LOGINFO, "process thread over!");
}

void CZeroconfBrowserDNS::StopThread(bool bWait)
{
return;
  {
    CSingleLock lock(m_data_guard);
    m_thread_stop = true;
  }

  CThread::StopThread(bWait);
}
