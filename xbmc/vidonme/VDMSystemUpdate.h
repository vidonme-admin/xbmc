
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"
#include "vidonme/upgrade/VDMUpgrade.h"
#include <vector>
#include <map>
#include "guilib/GUIWindow.h"
#include "view/GUIViewControl.h"
#include "FileItem.h"
#include "VDMUtils.h"

using namespace VidOnMe;

class CVDMSystemUpdateDlg : public CVDMDialog
                          , public VidOnMe::IVDMUpgradeCallback
{
public:
	CVDMSystemUpdateDlg(void);
	virtual ~CVDMSystemUpdateDlg(void);

	static bool ShowInfo(void);

protected:
	virtual void OnInitWindow();
  virtual void OnDeinitWindow(int nextWindowID);
  virtual bool OnMessage(CGUIMessage& message);

private:
 void OnClick(const CGUIMessage& message);
 void UpdateUpgradeSettings();
 void UpdateManualUpgradeState();
 //upgrade callbacks
 virtual void OnCheckNewVersionStarted();
 virtual void OnCheckNewVersionFailed(const CStdString& strFailReason);
 virtual void OnCheckNewVersionResult(const VidOnMe::VMCUpgradeInfo& upgradeInfo, bool& bContinueDownload);
 virtual void OnDownloadStarted(const CStdString& strFileUrl, const CStdString& strDownloadPath);
 virtual void OnDownloadProgress(const CStdString& strFileUrl, int nDownloadSize, int nTotalSize);
 virtual void OnDownloadStopped(const CStdString& strFileUrl, const CStdString& strStopReason, bool& bRestart);
 virtual void OnDownloadEnded(const CStdString& strFileUrl, int nFilesize, const CStdString& strDownloadPath);
 virtual void OnDownloadFilesStarted();
 virtual void OnDownloadFilesStopped(const CStdString& strStopReason, bool& bRestart);
 virtual void OnDownloadFilesEnded(const CStdStringArray& downloadFiles, bool& bContinueInstall);
 virtual void OnInstallStarted();
 virtual void OnInstallProgress(int nProgress, int nTotal);
 virtual void OnInstallStopped(const CStdString& strStopReason, bool& bRestart);
 virtual void OnInstallEnded(bool bSuccessful);

 enum ManualUpgradeState
 {
   Upgrade_Check_None,
   Upgrade_Checking,
   Upgrade_Check_Last,
   Upgrade_Check_NewAvailable,
   Upgrade_Check_Failed,
   Upgrade_Downloading,
   Upgrade_Download_Done,
   Upgrade_Download_Failed,
   Upgrade_Installing,
   Upgrade_Install_Done,
   Upgrade_Install_Failed,
 };

 CCriticalSection m_SectionUpgrade;
 ManualUpgradeState m_eUpgradeState;
 CStdString m_strNewVersion;
 CStdString m_strChangeLog;

private:
	bool		m_bIsSendEmail;
};

#endif