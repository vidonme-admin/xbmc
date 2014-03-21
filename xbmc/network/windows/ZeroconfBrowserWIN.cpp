/*
 *      Copyright (C) 2012-2013 Team XBMC
 *      http://www.xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "ZeroconfBrowserWIN.h"
#include <utils/log.h>
#include <threads/SingleLock.h>
#include "guilib/GUIWindowManager.h"
#include "guilib/GUIMessage.h"
#include "GUIUserMessages.h"
#include "win32/WIN32Util.h"
#include "network/DNSNameCache.h"

#pragma comment(lib, "dnssd.lib")

extern HWND g_hWnd;

CZeroconfBrowserWIN::CZeroconfBrowserWIN()
{
  m_browser = NULL;
}

CZeroconfBrowserWIN::~CZeroconfBrowserWIN()
{
  CSingleLock lock(m_data_guard);
  //make sure there are no browsers anymore
  for(tBrowserMap::iterator it = m_service_browsers.begin(); it != m_service_browsers.end(); ++it )
    doRemoveServiceType(it->first);

  WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_browser ), g_hWnd, BONJOUR_BROWSER_EVENT, 0 );
  DNSServiceRefDeallocate(m_browser);
  m_browser = NULL;
}

void DNSSD_API CZeroconfBrowserWIN::BrowserCallback(DNSServiceRef browser,
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
    CZeroconfBrowserWIN* p_this = reinterpret_cast<CZeroconfBrowserWIN*>(context);
    //store the service
    ZeroconfService s(serviceName, regtype, replyDomain);

    if (flags & kDNSServiceFlagsAdd)
    {
      CLog::Log(LOGDEBUG, "ZeroconfBrowserWIN::BrowserCallback found service named: %s, type: %s, domain: %s", s.GetName().c_str(), s.GetType().c_str(), s.GetDomain().c_str());
      p_this->addDiscoveredService(browser, s);
    }
    else
    {
      CLog::Log(LOGDEBUG, "ZeroconfBrowserWIN::BrowserCallback service named: %s, type: %s, domain: %s disappeared", s.GetName().c_str(), s.GetType().c_str(), s.GetDomain().c_str());
      p_this->removeDiscoveredService(browser, s);
    }
    if(! (flags & kDNSServiceFlagsMoreComing) )
    {
      CGUIMessage message(GUI_MSG_NOTIFY_ALL, 0, 0, GUI_MSG_UPDATE_PATH);
      message.SetStringParam("zeroconf://");
      g_windowManager.SendThreadMessage(message);
      CLog::Log(LOGDEBUG, "ZeroconfBrowserWIN::BrowserCallback sent gui update for path zeroconf://");
    }
  }
  else
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserWIN::BrowserCallback returned (error = %ld)\n", (int)errorCode);
  }
}

#if defined(__VIDONME_MEDIACENTER__)
void DNSSD_API AddrinfoCallback(DNSServiceRef sdref, DNSServiceFlags flags, uint32_t interfaceIndex, DNSServiceErrorType errorCode, const char *hostname, const struct sockaddr *address, uint32_t ttl, void *context)
{
  if (errorCode)
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserWIN: AddrinfoCallback failed with error = %ld", (int) errorCode);
    return;
  }

  DNSServiceErrorType err;
  CStdString strIP;
  CZeroconfBrowser::ZeroconfService* service = (CZeroconfBrowser::ZeroconfService*) context;
  char addr[256] = "";
  if (address && address->sa_family == AF_INET)
  {
    const unsigned char *b = (const unsigned char *) &((struct sockaddr_in *)address)->sin_addr;
    snprintf(addr, sizeof(addr), "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
  }
  strIP = addr;
  service->SetIP(strIP);
}
#endif

void DNSSD_API CZeroconfBrowserWIN::ResolveCallback(DNSServiceRef                       sdRef,
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
    CLog::Log(LOGERROR, "ZeroconfBrowserWIN: ResolveCallback failed with error = %ld", (int) errorCode);
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

    err = DNSServiceGetAddrInfo(&sdRef, kDNSServiceFlagsReturnIntermediates, kDNSServiceInterfaceIndexAny,kDNSServiceProtocol_IPv4, hosttarget, AddrinfoCallback, service);
    if( err != kDNSServiceErr_NoError )
    {
      if (sdRef)
        DNSServiceRefDeallocate(sdRef);

      CLog::Log(LOGERROR, "ZeroconfBrowserWIN: DNSServiceGetAddrInfo returned (error = %ld)", (int) err);
      return;
    }
    err = DNSServiceProcessResult(sdRef);

    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfBrowserWIN::doGetAddrInfoService DNSServiceProcessResult returned (error = %ld)", (int) err);

    if (sdRef)
      DNSServiceRefDeallocate(sdRef);
  }
#else
  if(!CDNSNameCache::Lookup(hosttarget, strIP))
  {
    CLog::Log(LOGERROR, "ZeroconfBrowserWIN: Could not resolve hostname %s",hosttarget);
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
void CZeroconfBrowserWIN::addDiscoveredService(DNSServiceRef browser, CZeroconfBrowser::ZeroconfService const& fcr_service)
{
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

void CZeroconfBrowserWIN::removeDiscoveredService(DNSServiceRef browser, CZeroconfBrowser::ZeroconfService const& fcr_service)
{
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


bool CZeroconfBrowserWIN::doAddServiceType(const CStdString& fcr_service_type)
{
  DNSServiceErrorType err;
  DNSServiceRef browser = NULL;

  if(m_browser == NULL)
  {
    err = DNSServiceCreateConnection(&m_browser);
    if (err != kDNSServiceErr_NoError)
    {
      CLog::Log(LOGERROR, "ZeroconfBrowserWIN: DNSServiceCreateConnection failed with error = %ld", (int) err);
      return false;
    }
    err = WSAAsyncSelect( (SOCKET) DNSServiceRefSockFD( m_browser ), g_hWnd, BONJOUR_BROWSER_EVENT, FD_READ | FD_CLOSE );
    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfBrowserWIN: WSAAsyncSelect failed with error = %ld", (int) err);
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

    CLog::Log(LOGERROR, "ZeroconfBrowserWIN: DNSServiceBrowse returned (error = %ld)", (int) err);
    return false;
  }

  //store the browser
  {
    CSingleLock lock(m_data_guard);
    m_service_browsers.insert(std::make_pair(fcr_service_type, browser));
  }

  return true;
}

bool CZeroconfBrowserWIN::doRemoveServiceType(const CStdString& fcr_service_type)
{
  //search for this browser and remove it from the map
  DNSServiceRef browser = 0;
  {
    CSingleLock lock(m_data_guard);
    tBrowserMap::iterator it = m_service_browsers.find(fcr_service_type);
    if(it == m_service_browsers.end())
    {
      return false;
    }
    browser = it->second;
    m_service_browsers.erase(it);
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

std::vector<CZeroconfBrowser::ZeroconfService> CZeroconfBrowserWIN::doGetFoundServices()
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

bool CZeroconfBrowserWIN::doResolveService(CZeroconfBrowser::ZeroconfService& fr_service, double f_timeout)
{
  DNSServiceErrorType err;
  DNSServiceRef sdRef = NULL;

  err = DNSServiceResolve(&sdRef, 0, kDNSServiceInterfaceIndexAny, fr_service.GetName(), fr_service.GetType(), fr_service.GetDomain(), ResolveCallback, &fr_service);

  if( err != kDNSServiceErr_NoError )
  {
    if (sdRef)
      DNSServiceRefDeallocate(sdRef);

    CLog::Log(LOGERROR, "ZeroconfBrowserWIN: DNSServiceResolve returned (error = %ld)", (int) err);
    return false;
  }

#if defined(__VIDONME_MEDIACENTER__)
  //don't block in DNSServiceProcessResult, timeout is 0.
  int dns_sd_fd  = sdRef ? DNSServiceRefSockFD(sdRef) : -1;
  int nfds = dns_sd_fd + 1;
  fd_set readfds;
  struct timeval tv;
  int result;
#if 0
  while (!stopNow)
  {
    // 1. Set up the fd_set as usual here.
    // This example client has no file descriptors of its own,
    // but a real application would call FD_SET to add them to the set here
    FD_ZERO(&readfds);

    // 2. Add the fd for our client(s) to the fd_set
    if (sdRef) FD_SET(dns_sd_fd , &readfds);

    // 3. Set up the timeout.
    tv.tv_sec  = 0;
    tv.tv_usec = 0;

    result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

    if (result > 0)
    {
      DNSServiceErrorType err = kDNSServiceErr_NoError;
      if (sdRef && FD_ISSET(dns_sd_fd , &readfds))
        err = DNSServiceProcessResult(sdRef);
      if (err)
      { 
        fprintf(stderr, "DNSServiceProcessResult returned %d\n", err); 
        stopNow = 1; 
      }
    }
    else
    {
      printf("select() returned %d errno %d %s\n", result, errno, strerror(errno));
      if (errno != EINTR) stopNow = 1;
    }
  }
#else
  // 1. Set up the fd_set as usual here.
  // This example client has no file descriptors of its own,
  // but a real application would call FD_SET to add them to the set here
  FD_ZERO(&readfds);

  // 2. Add the fd for our client(s) to the fd_set
  if (sdRef) FD_SET(dns_sd_fd , &readfds);

  // 3. Set up the timeout.
  tv.tv_sec  = 1;
  tv.tv_usec = 0;

  result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);

  if (result > 0)
  {
    DNSServiceErrorType err = kDNSServiceErr_NoError;
    if (sdRef && FD_ISSET(dns_sd_fd , &readfds))
      err = DNSServiceProcessResult(sdRef);
    if (err)
    { 
      fprintf(stderr, "DNSServiceProcessResult returned %d\n", err); 
    }
  }
  else
  {
    printf("select() returned %d errno %d %s\n", result, errno, strerror(errno));
    if (errno != EINTR)
    {
      //stopNow = 1;
    }
  }
#endif
#else
  err = DNSServiceProcessResult(sdRef);
#endif//__VIDONME_MEDIACENTER__

  if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfBrowserWIN::doResolveService DNSServiceProcessResult returned (error = %ld)", (int) err);

  if (sdRef)
    DNSServiceRefDeallocate(sdRef);

  return true;
}

void CZeroconfBrowserWIN::ProcessResults()
{
  CSingleLock lock(m_data_guard);
  DNSServiceErrorType err = DNSServiceProcessResult(m_browser);
  if (err != kDNSServiceErr_NoError)
    CLog::Log(LOGERROR, "ZeroconfWIN: DNSServiceProcessResult returned (error = %ld)", (int) err);
}