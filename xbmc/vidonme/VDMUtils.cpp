#include "VDMUtils.h"
#include "FileItem.h"
#include "network/ZeroconfBrowser.h"
#include "settings/GUISettings.h"
#include "utils/Stopwatch.h"
#include "Application.h"
#include "threads/SingleLock.h"
#include "threads/Thread.h"
#include "utils/JobManager.h"
#include "windowing/WinEvents.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/CPUInfo.h"
#include "URL.h"
#include "ApplicationMessenger.h"
#include "upgrade/VDMUpgrade.h"
#include "cores/vidonme/VDMPlayer.h"
#include "interfaces/json-rpc/ProxyJSONRPC.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "VDMDumpUpload.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#if defined(TARGET_WINDOWS)
#include <tlhelp32.h>
#endif

#define DEVICE_AIRPLAY_TYPE "_airplay._tcp."
#define DEVICE_VDMSERVER_MEDIASERVER_TYPE "_vdmserver-mediaserver._tcp."
#define DEVICE_VDMSERVER_JSONRPCSERVER_TYPE "_vdmserver-jsonrpc-h._tcp."
#define DEVICE_VDMSERVER_EVENTSERVER_TYPE "_vdmserver-jsonrpc._tcp."
#define DEVICE_VDMSERVER_WEBSERVER_TYPE "_vdmserver-web._tcp."

#define LOCAL_SERVER_NAME "XBMC Local Server"
#define LOCAL_SERVER_IP "127.0.0.1"
#define LOCAL_SERVER_PORT 80
#define LOCAL_EVENT_PORT 80

#define VIDONME_MEDIA_CENTER_VERSION "1005"
#define VIDONME_MEDIA_CENTER_VERSION_STRING "1.0.0.5"

#define VIDONME_MEDIA_SERVER_PROCESS_NAME "VMS.exe"
#define VIDONME_TRAY_PROCESS_NAME "VidOn XBMC Media Server.exe"

using namespace JSONRPC;

static CStdString s_strCurVersion = VIDONME_MEDIA_CENTER_VERSION;

namespace VidOnMe
{
  CLongJob::CLongJob() : m_bStop(false)
  {

  }

  CLongJob::~CLongJob()
  {

  }

  void CLongJob::Start()
  {

  }

  bool CLongJob::Stop()
  {
    CSingleLock lock(m_section);
    m_bStop = true;
    return true;
  }

  void CLongJob::Pause()
  {

  }

  void CLongJob::Resume()
  {

  }

  VDMServer::VDMServer(bool bLocal)
    : m_nMediaPort(0)
    , m_nJsonRPCPort(0)
		, m_nEventPort(0)
    , m_nWebPort(0)
    , m_bLocal(bLocal)
  {
    if (m_bLocal)
    {
      Reset(true);
    }
  }

  void VDMServer::Reset(bool bLocal)
  {
    m_strName.Empty();
    m_strIP.Empty();
    m_nMediaPort = 0;
    m_nJsonRPCPort = 0;
		m_nEventPort = 0;
    m_nWebPort = 0;
    m_bLocal = bLocal;
    if (m_bLocal)
    {
      m_strName = LOCAL_SERVER_NAME;
      m_strIP = LOCAL_SERVER_IP;
      m_nJsonRPCPort = LOCAL_SERVER_PORT;
			m_nEventPort = LOCAL_EVENT_PORT;
      m_nWebPort = LOCAL_SERVER_PORT;
    }
  }

  VDMUtils::VDMUtils()
    : m_bServerInited(false)
    , m_bServerIniting(false)
    , m_bAirplayInited(false)
    , m_runningMode(RM_XBMC)
    , m_productType(PRODUCT_TYPE_FREE)
  {

  }

  VDMUtils& VDMUtils::Instance()
  {
    static VDMUtils s_vdmUtils;
    return s_vdmUtils;
  }

  void VDMUtils::VDMServerAutoDiscover(VDMServerBatch& servers)
  {
    servers.clear();

    CZeroconfBrowser::GetInstance()->Start();

    CLog::Log(LOGINFO, "After browser start...");

    std::vector<CZeroconfBrowser::ZeroconfService> zss = CZeroconfBrowser::GetInstance()->GetFoundServices();
    CLog::Log(LOGINFO, "After GetFoundServices...");

    if (zss.empty())
    {
      if (!g_application.m_bStop)
      {
        CStopWatch watch;
        watch.Start();

        while (watch.GetElapsedSeconds() < 5)
        {
          if (g_application.IsCurrentThread())
          {
#ifdef _WIN32
            CWinEvents::MessagePump();
#endif
          }
          else
          {
            Sleep(200);
          }

          zss = CZeroconfBrowser::GetInstance()->GetFoundServices();
          if (!zss.empty())
          {
            break;
          }
        }

        watch.Stop();
      }
    }

    if (!zss.empty())
    {
      VDMServerBatch serversDetect;
      std::vector<CZeroconfBrowser::ZeroconfService>::iterator it;
      for (it = zss.begin(); it != zss.end(); it++)
      {
        CZeroconfBrowser::GetInstance()->ResolveService(*it);

        if (it->GetType() == DEVICE_VDMSERVER_MEDIASERVER_TYPE || it->GetType() == DEVICE_VDMSERVER_WEBSERVER_TYPE
          || it->GetType() == DEVICE_VDMSERVER_JSONRPCSERVER_TYPE || it->GetType() == DEVICE_VDMSERVER_EVENTSERVER_TYPE)
        {
          VDMServerBatch::iterator sIter = serversDetect.begin();
          for (; sIter != serversDetect.end(); ++sIter)
          {
            CLog::Log(LOGINFO, "server - name: %s, ip: %s, port: %d", it->GetName().c_str(), it->GetIP().c_str(), it->GetPort());
            if (it->GetName() == (*sIter).m_strName)
            {
              break;
            }
          }        

          VDMServer* pServer = NULL;
          if (sIter == serversDetect.end())
          {
            VDMServer server;
            server.m_strName = it->GetName();
            server.m_strIP = it->GetIP();
            serversDetect.push_back(server);
            pServer = &serversDetect[serversDetect.size() - 1];
          }
          else
          {
            pServer = &(*sIter);
          }

          if (it->GetType() == DEVICE_VDMSERVER_MEDIASERVER_TYPE) 
          {
            pServer->m_nMediaPort = it->GetPort();
          }
          else if (it->GetType() == DEVICE_VDMSERVER_JSONRPCSERVER_TYPE)
          {
            pServer->m_nJsonRPCPort = it->GetPort();
          }
          else if (it->GetType() == DEVICE_VDMSERVER_EVENTSERVER_TYPE)
          {
            pServer->m_nEventPort = it->GetPort();
          }
          else if (it->GetType() == DEVICE_VDMSERVER_WEBSERVER_TYPE)
          {
            pServer->m_nWebPort = it->GetPort();
          }
        }
      }

      VDMServerBatch::iterator sIter = serversDetect.begin();
      for (; sIter != serversDetect.end(); ++sIter)
      {
        if (!(*sIter).m_strIP.IsEmpty() && (*sIter).m_nMediaPort && (*sIter).m_nJsonRPCPort && (*sIter).m_nEventPort)
        {
          servers.push_back(*sIter);
        }
      }   
    }

    CZeroconfBrowser::GetInstance()->Stop();  
  }

  bool VDMUtils::IsServerIniting() const
  {
    CSingleLock lock(m_critSectionServerIniting);
    return m_bServerIniting;
  }

  void VDMUtils::Initialize()
  {
    CVDMUpgradeManager::Instance().StartUpgrade();
    CVDMPlayer::InitializePlayerCore();
		CVDMDumpUpload::Instance().CheckDump();
  }

  void VDMUtils::Uninitialize()
  {
    CVDMPlayer::UninitializePlayerCore();
  }

  bool VDMUtils::InitCurrentServer()
  {
    {
      CSingleLock lock(m_critSectionServer);
      m_bServerInited = false;
    }

    {
      CSingleLock lock(m_critSectionServerIniting);
      m_bServerIniting = true;
    }

    VDMServer server;
    server.m_strName = g_guiSettings.GetString("vdmserver.name");
    server.m_strIP = g_guiSettings.GetString("vdmserver.ip");
    server.m_nMediaPort = g_guiSettings.GetInt("vdmserver.mediaport");
    server.m_nJsonRPCPort = g_guiSettings.GetInt("vdmserver.jsonrpcport");
    server.m_nWebPort = g_guiSettings.GetInt("vdmserver.webport");
    server.m_bLocal = g_guiSettings.GetBool("vdmserver.local");

    VDMServerBatch servers;
    VDMServerAutoDiscover(servers);
    VDMServerBatch::const_iterator iter = servers.begin();
    for (; iter != servers.end(); ++iter)
    {
      if ((*iter).m_strName == server.m_strName)
      {
        break;
      }
    }

    if (iter != servers.end())
    {
      server = (*iter);
    }
    else
    {
      server.Reset(true);
    }

    SetCurrentServer(server);

    {
      CSingleLock lock(m_critSectionServerIniting);
      m_bServerIniting = false;
    }

    return true;
  }

  bool VDMUtils::GetCurrentServer(VDMServer& server) const
  {
    CSingleLock lock(m_critSectionServer);

    if (!m_bServerInited)
    {
      return false;
    }

    server = m_currentServer;
    return true;
  }

  void VDMUtils::SetCurrentServer(const VDMServer& server)
  {
    CSingleLock lock(m_critSectionServer);

    m_currentServer = server;
    m_bServerInited = true;
    g_guiSettings.SetString("vdmserver.name", m_currentServer.m_strName);
    g_guiSettings.SetString("vdmserver.ip", m_currentServer.m_strIP);
    g_guiSettings.SetInt("vdmserver.mediaport", m_currentServer.m_nMediaPort);
    g_guiSettings.SetInt("vdmserver.jsonrpcport", m_currentServer.m_nJsonRPCPort);
    g_guiSettings.SetInt("vdmserver.webport", m_currentServer.m_nWebPort);
    g_guiSettings.SetBool("vdmserver.local", m_currentServer.m_bLocal);
  }

  void VDMUtils::VDMAirplayAutoDiscover(AirplayDeviceBatch& airplayDevices)
  {
    airplayDevices.clear();

    CZeroconfBrowser::GetInstance()->Stop();  
    CZeroconfBrowser::GetInstance()->Start();
    std::vector<CZeroconfBrowser::ZeroconfService> zss = CZeroconfBrowser::GetInstance()->GetFoundServices();

    if (zss.empty())
    {
      if (!g_application.m_bStop)
      {
        CStopWatch watch;
        watch.Start();
        while (watch.GetElapsedSeconds() < 5)
        {
          if (g_application.IsCurrentThread())
          {
#if 0
            g_application.FrameMove(true);
#else
            CWinEvents::MessagePump();
#endif
          }
          else
          {
            Sleep(200);
          }

          zss = CZeroconfBrowser::GetInstance()->GetFoundServices();
          if (!zss.empty())
          {
            break;
          }
        }

        watch.Stop();
      }
    }

    if (!zss.empty())
    {
      std::vector<CZeroconfBrowser::ZeroconfService>::iterator it;
      for (it = zss.begin(); it != zss.end(); it++)
      {
        if (it->GetType() == "_airplay._tcp.")
        {
          CZeroconfBrowser::GetInstance()->ResolveService(*it);
          AirplayDevice airplay;
          airplay.m_strName = it->GetName();
          airplay.m_strIP = it->GetIP();
          airplay.m_nPort = it->GetPort();
          airplayDevices.push_back(airplay);
        }
      }
    }  
  }

  bool VDMUtils::GetCurrentAirplay(AirplayDevice& airplay) const
  {
    CSingleLock lock(m_critSectionAirplay);

    if (!m_bAirplayInited)
    {
      return false;
    }

    airplay = m_currentAirplay;
    return true;
  }

  void VDMUtils::SetCurrentAirplay(const AirplayDevice& airplay)
  {
    CSingleLock lock(m_critSectionAirplay);

    m_currentAirplay = airplay;
    m_bAirplayInited = true;
  }

  RUNNINGMODE VDMUtils::GetRunningMode() const
  {
    CSingleLock lock(m_critSectionRunningMode);

    return m_runningMode;
  }

  void VDMUtils::SetRunningMode(RUNNINGMODE runningMode)
  {
    CSingleLock lock(m_critSectionRunningMode);

    m_runningMode = runningMode;
  }

  CStdString VDMUtils::GetCurrentVersion() const
  {
    return s_strCurVersion;
  }

  CStdString VDMUtils::GetCurrentVersionString() const
  {
    return ConvertVersionString(s_strCurVersion);
  }

  void VDMUtils::SetCurrentVersion(const CStdString& strVersion)
  {
    s_strCurVersion = strVersion;
    g_guiSettings.SetString("vidonme.version", GetCurrentVersionString().c_str());
  }

  PRODUCT_TYPE VDMUtils::GetProductType() const
  {
    return m_productType;
  }

  void VDMUtils::SetProductType(PRODUCT_TYPE productType)
  {
#if defined(TARGET_WINDOWS) && defined(_DEBUG)
    if( PRODUCT_TYPE_FREE == productType  )
        return;
#endif 
    m_productType = productType;
  }

#if defined(TARGET_WINDOWS)
  wchar_t* VDMUtils::cs2wcs(const char* sz)
  {
    size_t len = strlen(sz) + 1;
    size_t converted = 0;
    wchar_t* wsz = (wchar_t*)malloc(len * sizeof(wchar_t));
    mbstowcs_s(&converted, wsz, len, sz, _TRUNCATE);
    return wsz; 
  }

  char* VDMUtils::wcs2cs(const wchar_t* wsz)
  {
    size_t len = wcslen(wsz) + 1;
    size_t converted = 0;
    char* sz = (char*)malloc(len * sizeof(char)); 
    wcstombs_s(&converted, sz, len, wsz, _TRUNCATE);
    return sz;
  }

  #endif

  bool VDMUtils::KillProcess(const CStdString& strProcessName)
  {
    if (strProcessName.IsEmpty()) return false;
#if defined(TARGET_WINDOWS)
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
      return false;
    }

    if(Process32First(hProcessSnap, &pe32))
    {
      bool bFound = false;
      if (!strcmp(pe32.szExeFile, strProcessName.c_str()))
      {
        bFound = true;
      }

      while((!bFound) && Process32Next(hProcessSnap, &pe32))
      {
        if (!strcmp(pe32.szExeFile, strProcessName.c_str()))
        {
          bFound = true;
          break;
        }
      }

      if (bFound)
      {
        CloseHandle(hProcessSnap);
        HANDLE handLe = OpenProcess(PROCESS_TERMINATE , FALSE, pe32.th32ProcessID);
        return TerminateProcess(handLe, 0) != FALSE;
      }
    }

    CloseHandle(hProcessSnap);
#else
#endif
    return false;
  }

  bool VDMUtils::Install(const CStdString& strFilePath,bool bShow)
  {
    bool bRet = false;
#if defined(TARGET_WINDOWS)
    KillProcess(VIDONME_TRAY_PROCESS_NAME);
    KillProcess(VIDONME_MEDIA_SERVER_PROCESS_NAME);

    HINSTANCE hInst = ShellExecuteA(NULL, "open", strFilePath.c_str(), NULL, NULL, bShow ? SW_SHOW : SW_HIDE);
    if ((int)hInst > 32)
    {
      bRet = true;
    }
#elif defined(TARGET_ANDROID)
    CStdString strAppPath;
    strAppPath.Format("file://%s", strFilePath.c_str());
    bRet = CXBMCApp::StartActivity("", "android.intent.action.VIEW", "application/vnd.android.package-archive", strAppPath.c_str());
#endif

    if (bRet)
    {
    //  CApplicationMessenger::Get().Quit();
    }

    return bRet;
  }

  void VDMUtils::SetXMLString(TiXmlNode* pRootNode, const char* strTag, const std::string& strValue)
  {
    TiXmlElement newElement(strTag);
    TiXmlNode *pNewNode = pRootNode->InsertEndChild(newElement);
    if (pNewNode)
    {
      TiXmlText value(strValue);
      pNewNode->InsertEndChild(value);
    }
  }

  bool VDMUtils::GetXMLString(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue)
  {
    const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag );
    if (!pElement) return false;
    const char* encoded = pElement->Attribute("urlencoded");
    const TiXmlNode* pNode = pElement->FirstChild();
    if (pNode != NULL)
    {
      strStringValue = pNode->Value();
      if (encoded && strcasecmp(encoded,"yes") == 0)
        CURL::Decode(strStringValue);
      return true;
    }
    strStringValue.clear();
    return false;
  }

	CStdString VDMUtils::ConvertVersionString(const CStdString& numVersion)
	{
		CStdString strVersion = numVersion;
		if (strVersion.GetLength() >= 3)
		{
			int len = strVersion.GetLength();
#if 0
			strVersion.Insert(len - 3, '.');
			strVersion.Insert(len - 1, '.');
			strVersion.Insert(len + 1, '.');
#endif
      strVersion.Insert(1, '.');
      strVersion.Insert(3, '.');
      strVersion.Insert(5, '.');
		}

    strVersion.Format("V%s", strVersion.c_str());
		return strVersion;
	}

  bool VDMUtils::IsProbableBluraySource(const CStdString& strSourcePath)
  {
    if (XFILE::CFile::Exists(strSourcePath))
    {
      return IsProbableBlurayFile(strSourcePath);
    }
    else if (XFILE::CDirectory::Exists(strSourcePath))
    {
      return IsProbableBlurayFolder(strSourcePath);
    }

    return false;
  }

  bool VDMUtils::IsProbableBlurayFolder(const CStdString& strDirectory)
  {
    if (strDirectory.IsEmpty())
    {
      return false;
    }

    CStdString strFolder = strDirectory;
    URIUtils::RemoveSlashAtEnd(strFolder);

    CStdString strFileName = URIUtils::GetFileName(strFolder);

    CProxyJSONRPC::FileNodes fileNodes;
    bool bRet = false;
    CProxyJSONRPC::GetDirectory(strFolder, "", bRet, fileNodes);

    for (int i = 0; i < fileNodes.size(); i++)
    {
      URIUtils::RemoveSlashAtEnd(fileNodes[i].strPath);
      CStdString strSubFileName = URIUtils::GetFileName(fileNodes[i].strPath);
      if (strFileName.CompareNoCase("BDMV") == 0)
      {
        if (strSubFileName.CompareNoCase("index.bdmv") == 0)
        {
          return true;
        }
      }
      else
      {
        if (strSubFileName.CompareNoCase("BDMV") == 0)
        {
          return IsProbableBlurayFolder(fileNodes[i].strPath);
        }
      }
    }

    return false;
  }

  bool VDMUtils::IsProbableBlurayFile(const CStdString& strFilePath)
  {
    CStdString strFolder;
    CStdString strFileName;
    URIUtils::Split(strFilePath, strFolder, strFileName);

    if (strFileName.CompareNoCase("index.bdmv") == 0)
    {
      URIUtils::RemoveSlashAtEnd(strFolder);
      if (URIUtils::GetFileName(strFolder).CompareNoCase("bdmv") == 0)
      {
        return true; 
      }
    }
    else
    {
      CStdString ext = URIUtils::GetExtension(strFileName);
      ext.ToLower();
      if (ext == ".iso" || ext == ".img")
      {
        CURL url2("udf://");
        url2.SetHostName(strFilePath);
        url2.SetFileName("BDMV/index.bdmv");
        if (XFILE::CFile::Exists(url2.Get()))
        {
          return true;
        }
      }
    }
    
    return false;
  }

  bool VDMUtils::IsProbableDVDSource(const CStdString& strSourcePath)
  {
    if (XFILE::CFile::Exists(strSourcePath))
    {
      return IsProbableDVDFile(strSourcePath);
    }
    else if (XFILE::CDirectory::Exists(strSourcePath))
    {
      return IsProbableDVDFolder(strSourcePath);
    }

    return false;
  }

  bool VDMUtils::IsProbableDVDFolder(const CStdString& strDirectory)
  {
    if (strDirectory.IsEmpty())
    {
      return false;
    }

    CStdString strFolder = strDirectory;
    URIUtils::RemoveSlashAtEnd(strFolder);

    CStdString strFileName = URIUtils::GetFileName(strFolder);

    CProxyJSONRPC::FileNodes fileNodes;
    bool bRet = false;
    CProxyJSONRPC::GetDirectory(strFolder, "", bRet, fileNodes);

    for (int i = 0; i < fileNodes.size(); i++)
    {
      URIUtils::RemoveSlashAtEnd(fileNodes[i].strPath);
      CStdString strSubFileName = URIUtils::GetFileName(fileNodes[i].strPath);
      if (strFileName.CompareNoCase("VIDEO_TS") == 0)
      {
        if (strSubFileName.CompareNoCase("video_ts.ifo") == 0)
        {
          return true;
        }
      }
      else
      {
        if (strSubFileName.CompareNoCase("VIDEO_TS") == 0)
        {
          return IsProbableDVDFolder(fileNodes[i].strPath);
        }
      }
    }

    return false;
  }

  bool VDMUtils::IsProbableDVDFile(const CStdString& strFilePath)
  {
    CStdString strFolder;
    CStdString strFileName;
    URIUtils::Split(strFilePath, strFolder, strFileName);

    if (strFileName.CompareNoCase("video_ts.ifo") == 0)
    {
      URIUtils::RemoveSlashAtEnd(strFolder);
      if (URIUtils::GetFileName(strFolder).CompareNoCase("video_ts") == 0)
      {
        return true; 
      }
    }
    else
    {
      CStdString ext = URIUtils::GetExtension(strFileName);
      ext.ToLower();
      if (ext == ".iso" || ext == ".img")
      {
        CURL url2("udf://");
        url2.SetHostName(strFilePath);
        url2.SetFileName("video_ts/video_ts.ifo");
        if (XFILE::CFile::Exists(url2.Get()))
        {
          return true;
        }
      }
    }

    return false;
  }

  CPU_TYPE VDMUtils::GetCPUType()
  {
    static CPU_TYPE s_type = CT_UNKNOWN;
    static bool s_bCPUTypeGot = false;

    if (s_bCPUTypeGot)
    {
      return s_type;
    }

#if defined(__ANDROID_ALLWINNER__)
    CStdString strHardware = g_cpuInfo.getCPUHardware();
    if (-1 != strHardware.Find("7i"))
    {
      s_type = CT_ALLWINNER_A20;
    }
    else if(-1 != strHardware.Find("6i"))
    {
      s_type = CT_ALLWINNER_A31;
    }
#endif

    s_bCPUTypeGot = true;

    return s_type;
  }
}
