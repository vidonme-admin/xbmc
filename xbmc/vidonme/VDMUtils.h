#ifndef _VDMUTILS_H_
#define _VDMUTILS_H_

#pragma once

#include <vector>
#include "utils/StdString.h"
#include "threads/CriticalSection.h"
#include "threads/Event.h"
#include "utils/XBMCTinyXML.h"
#include "utils/Job.h"

namespace VidOnMe
{
  class CLongJob : public CJob
  {
  public:
    CLongJob();
    virtual ~CLongJob();
    void Start();
    bool Stop();
    void Pause();
    void Resume();
  protected:
    CCriticalSection m_section;
    bool m_bStop;
  };

  struct VDMServer
  {
    VDMServer(bool bLocal = false);
    void Reset(bool bLocal = false);
    CStdString m_strName;
    CStdString m_strIP;
    int m_nMediaPort;
    int m_nJsonRPCPort;
    int m_nEventPort;
    int m_nWebPort;
    bool m_bLocal;
  };
  typedef std::vector<VDMServer> VDMServerBatch;

  struct AirplayDevice
  {
    CStdString m_strName;
    CStdString m_strIP;
    int m_nPort;
  };
  typedef std::vector<AirplayDevice> AirplayDeviceBatch;

  enum RUNNINGMODE
  {
    RM_VIDONME,
    RM_XBMC,
  };

  enum PRODUCT_TYPE
  {
    PRODUCT_TYPE_PRO,
    PRODUCT_TYPE_FREE,
  };

  enum CPU_TYPE
  {
    CT_UNKNOWN,
#if defined(__ANDROID_ALLWINNER__)
    CT_ALLWINNER_A20,
    CT_ALLWINNER_A31,
#endif
  };

  class VDMUtils
  {
  public:  
    static VDMUtils& Instance();

    static void VDMServerAutoDiscover(VDMServerBatch& servers);
    static void VDMAirplayAutoDiscover(AirplayDeviceBatch& airplayDevices);

    static wchar_t* cs2wcs(const char* sz);
    static char* wcs2cs(const wchar_t* wsz);
    
    static bool KillProcess(const CStdString& strProcessName);
    static bool Install(const CStdString& strFilePath, bool bShow = true);
    
    static void SetXMLString(TiXmlNode* pRootNode, const char* strTag, const std::string& strValue);
    static bool GetXMLString(const TiXmlNode* pRootNode, const char* strTag, std::string& strStringValue);
    
    static CStdString ConvertVersionString(const CStdString& numVersion);

    static bool IsProbableBluraySource(const CStdString& strSourcePath);
    static bool IsProbableBlurayFolder(const CStdString& strDirectory);
    static bool IsProbableBlurayFile(const CStdString& strFilePath);
    static bool IsProbableDVDSource(const CStdString& strSourcePath);
    static bool IsProbableDVDFolder(const CStdString& strDirectory);
    static bool IsProbableDVDFile(const CStdString& strFilePath);

    static CPU_TYPE GetCPUType();

  public:
    void Initialize();
    void Uninitialize();
    bool InitCurrentServer();
    bool IsServerIniting() const;
    bool GetCurrentServer(VDMServer& server) const;
    void SetCurrentServer(const VDMServer& server);

    bool GetCurrentAirplay(AirplayDevice& airplay) const;
    void SetCurrentAirplay(const AirplayDevice& airplay);

    RUNNINGMODE GetRunningMode() const;
    void SetRunningMode(RUNNINGMODE runningMode);

    CStdString GetCurrentVersion() const;
    CStdString GetCurrentVersionString() const;

    PRODUCT_TYPE GetProductType() const;
  private:
    VDMUtils();
    void SetProductType(PRODUCT_TYPE productType);
    void SetCurrentVersion(const CStdString& strVersion);
  private:
    VDMServer m_currentServer;
    bool m_bServerInited;
    CCriticalSection m_critSectionServerIniting;
    bool m_bServerIniting;
    CCriticalSection m_critSectionServer;

    AirplayDevice m_currentAirplay;
    bool m_bAirplayInited;
    CCriticalSection m_critSectionAirplay;

    RUNNINGMODE m_runningMode;
    CCriticalSection m_critSectionRunningMode;

    PRODUCT_TYPE m_productType;

    friend class CVersionCheck;
  };
}
#endif