
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMSystemUpdate.h"
#include "VDMUpdateSchedule.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "interfaces/Builtins.h"

#include "vidonme/controls/GUITabControl.h"
#include "vidonme/controls/VDMSliderControl.h"
#include "guilib/GUIButtonControl.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

using namespace VidOnMe;

#define TMSG_UPDATE_UPGRADE   (GUI_MSG_USER + 2000)

#define		CONTROL_BUTTON_SEND				100
#define SEND_TMSG_TO_UPDATE_UPGRADE \
  do \
  { \
  CGUIMessage tMsg(TMSG_UPDATE_UPGRADE, GetID(), GetID()); \
  g_windowManager.SendThreadMessage(tMsg, GetID()); \
  } while (0);

//upgrade
//{
//check new version
#define CONTROL_GROUP_UPGRADE_CHECK						4810
#define	CONTROL_LABEL_UPGRADE_CURVERSION			4811
#define CONTROL_BUTTON_UPGRADE_CHECK					4812
#define	CONTROL_LABEL_UPGRADE_LASTVERSION			4813
#define CONTROL_LABEL_CHECKING_NEWVERSION     4814
#define CONTROL_LABEL_CHECK_NEWVER_FALIED     4815
#define CONTROL_BUTTON_UPGRADE_CHECK_CANCEL   4816
#define CONTROL_BUTTON_UPGRADE_CHECK_RESTART  4817
#define	CONTROL_GROUP_UPGRADE_NEW_VERSION			4820
#define CONTROL_LABEL_UPGRADE_VERSION					4821
#define CONTROL_LABEL_UPGRADE_CHANGELOG				4823

//download
#define CONTROL_BUTTON_UPGRADE_DOWNLOAD				4830
#define CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL 4831

#define CONTROL_GROUP_UPGRADE_DOWNLOAD				4840
#define CONTROL_LABEL_UPGRADE_DOWNLOADING     4841
#define CONTROL_LABEL_UPGRADE_DOWNLOAD				4842
#define CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD		4843
#define CONTROL_LABEL_UPGRADE_DOWNLOAD_FAILED 4844

//install
#define CONTROL_BUTTON_UPGRADE_INSTALL				4850
#define CONTROL_LABEL_UPGRADE_DOWNLOAD_DONE		4851
#define CONTROL_LABEL_UPGRADE_INSTALLING		  4852
#define CONTROL_LABEL_UPGRADE_INSTALL_SUCC		4853
#define CONTROL_LABEL_UPGRADE_INSTALL_FAIL		4854
#define CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL 4855
//}upgrade

CVDMSystemUpdateDlg::CVDMSystemUpdateDlg(void) 
	: CVDMDialog(VDM_WINDOW_SYSTEM_UPDATE)
{
	m_loadType = LOAD_ON_GUI_INIT;
  m_eUpgradeState = Upgrade_Check_None;

  AddGroup(230,75,800,550);
  AddImage(0, 0, 800, 550, "special://xbmc/media/title_bg.png");
  AddImage(5, 5, 790, 540, "special://xbmc/media/right_bg.png");

	AddGroup(230,125,800,550,CONTROL_GROUP_UPGRADE_CHECK);
	AddLabel(150, 0, 500, 66, "60243", CONTROL_LABEL_UPGRADE_CURVERSION, "font18", "FF999999", "true","center","center");
  AddButton(250, 400, 310, 50, "60244", CONTROL_BUTTON_UPGRADE_CHECK, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FF999999", "FFFFFFFF", 20, "", "center", "center");
  AddButton(60, 400, 310, 50, "60244", CONTROL_BUTTON_UPGRADE_CHECK_RESTART, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FF999999", "FFFFFFFF", 20, "", "center", "center",0,CONTROL_BUTTON_UPGRADE_CHECK_CANCEL);
  AddButton(430, 400, 310, 50, "60002", CONTROL_BUTTON_UPGRADE_CHECK_CANCEL, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FF999999", "FFFFFFFF", 20, "", "center", "center",CONTROL_BUTTON_UPGRADE_CHECK_RESTART,0);
	AddLabel(100, 100, 600, 66, "60248", CONTROL_LABEL_UPGRADE_LASTVERSION, "font16", "FF999999", "true","center","center");
	AddLabel(100, 100, 600, 66, "60254", CONTROL_LABEL_CHECKING_NEWVERSION, "font16", "FF999999", "true","center","center");
	AddLabel(100, 100, 600, 66, "60255", CONTROL_LABEL_CHECK_NEWVER_FALIED, "font16", "FF999999", "true","center","center");

  AddGroup(230,125,800,200,CONTROL_GROUP_UPGRADE_NEW_VERSION);
	AddLabel(100, 10, 600, 20, "", CONTROL_LABEL_UPGRADE_VERSION, "font14", "FF999999", "true","center","center");
	AddLabel(100, 40, 600, 20, "60240", 4822, "font16", "FF999999", "true","center","center");
	AddLabel(50, 70, 700, 300, "60240", CONTROL_LABEL_UPGRADE_CHANGELOG, "font14", "FF999999", "true");



  AddGroup(230,125,800,550);
  AddButton(60, 400, 310, 50, "60241", CONTROL_BUTTON_UPGRADE_DOWNLOAD, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FF999999", "FFFFFFFF", 20, "", "center", "center",0,CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL);

  AddGroup(230,125,800,550);
  AddButton(430, 400, 310, 50, "60002", CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FF999999", "FFFFFFFF", 20, "", "center", "center",CONTROL_BUTTON_UPGRADE_DOWNLOAD,0);

  AddGroup(230,125,800,550,CONTROL_GROUP_UPGRADE_DOWNLOAD);
  AddLabel(150, 250, 500, 100, "60250", CONTROL_LABEL_UPGRADE_DOWNLOADING ,"font14", "FFFFFFFF", "true","center","center");
  AddSlider(210, 350, 350, 20, CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD, "special://xbmc/media/OSDKey_LightSliderNF.png", "special://xbmc/media/OSDKey_LightSliderFO.png", "","");
  AddLabel(620, 350, 100, 20, "", CONTROL_LABEL_UPGRADE_DOWNLOAD ,"font14", "FFFFFFFF", "true","left","center");
  AddLabel(300, 350, 200, 20, "60256", CONTROL_LABEL_UPGRADE_DOWNLOAD_FAILED ,"font14", "FFFFFFFF", "true","center","center");

  AddGroup(230,125,800,550);
  AddButton(60, 400, 310, 50, "60245", CONTROL_BUTTON_UPGRADE_INSTALL, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FFFFFFFF", "FFFFFFFF", 20, "", "center", "center",0,CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL);

  AddGroup(230,125,800,550);
  AddButton(430, 400, 310, 50, "60002", CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font16", 
    "FFFFFFFF", "FFFFFFFF", 20, "", "center", "center",CONTROL_BUTTON_UPGRADE_INSTALL,0);

  AddGroup(230,125,800,550);
  AddLabel(100, 350, 600, 50, "70032", CONTROL_LABEL_UPGRADE_DOWNLOAD_DONE ,"font14", "FFFFFFFF", "true","center","center");

  AddGroup(230,125,800,550);
  AddLabel(100, 350, 600, 50, "60257", CONTROL_LABEL_UPGRADE_INSTALLING ,"font14", "FFFFFFFF", "true","center","center");

  AddGroup(230,125,800,550);
  AddLabel(100, 350, 600, 50, "60258", CONTROL_LABEL_UPGRADE_INSTALL_SUCC, "font14", "FFFFFFFF", "true","center","center");

  AddGroup(230,125,800,550);
  AddLabel(100, 350, 600, 50, "60259", CONTROL_LABEL_UPGRADE_INSTALL_FAIL, "font14", "FFFFFFFF", "true","center","center");
}

CVDMSystemUpdateDlg::~CVDMSystemUpdateDlg(void)
{

}

bool CVDMSystemUpdateDlg::ShowInfo(void)
{
	CVDMSystemUpdateDlg* pDlg = (CVDMSystemUpdateDlg*)g_windowManager.GetWindow(VDM_WINDOW_SYSTEM_UPDATE);

	if (pDlg)
	{
		pDlg->DoModal();
		return true;
	}

	return false;
}

void CVDMSystemUpdateDlg::OnInitWindow()
{
  UpdateUpgradeSettings();
	CVDMDialog::OnInitWindow();
}

void CVDMSystemUpdateDlg::OnDeinitWindow(int nextWindowID)
{
  if( m_eUpgradeState = Upgrade_Check_NewAvailable)
  {
     m_eUpgradeState = Upgrade_Check_None;
  }
  CVDMDialog::OnDeinitWindow(nextWindowID);
}

bool CVDMSystemUpdateDlg::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
    case GUI_MSG_CLICKED:
    {
      OnClick(message);
      return true;
    }
    case TMSG_UPDATE_UPGRADE:
    {
      UpdateUpgradeSettings();
      break;
    }
  } 
	return CVDMDialog::OnMessage(message);
}

void CVDMSystemUpdateDlg::OnClick(const CGUIMessage& message)
{
  switch (message.GetSenderId())
  {
    case CONTROL_BUTTON_UPGRADE_CHECK:
    case CONTROL_BUTTON_UPGRADE_CHECK_RESTART:
    {
      m_eUpgradeState = Upgrade_Checking;
      CVDMUpdateScheduleDlg* pDlg = (CVDMUpdateScheduleDlg*)g_windowManager.GetWindow(VDM_WINDOW_UPDATE_SCHEDULE);
      if(pDlg && pDlg->IsActive())
      {   
        pDlg->Close();
      }
      CVDMUpgradeManager::Instance().SetManualUpdate(true);
      CVDMUpgradeManager::Instance().AddCallback(this);
      UpdateManualUpgradeState();
      CVDMUpgradeManager::Instance().CheckNewVersion();
      break;
    }
    case CONTROL_BUTTON_UPGRADE_DOWNLOAD:
    {
      CVDMUpgradeManager::Instance().DownloadFiles();
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL,0);
      break;
    }
    case CONTROL_BUTTON_UPGRADE_CHECK_CANCEL:
    case CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL:
    case CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL:
    {
      {
        CVDMUpgradeManager::Instance().StopUpgrade();
        CSingleLock lock(m_SectionUpgrade);
        m_eUpgradeState = Upgrade_Check_None;

        SEND_TMSG_TO_UPDATE_UPGRADE;
      }
      break;
    }
    case CONTROL_BUTTON_UPGRADE_INSTALL:
    {
      CVDMUpgradeManager::Instance().Install();
      break;
    }
    default:
      break;
  }
}

void CVDMSystemUpdateDlg::UpdateUpgradeSettings(void)
{
  CStdString strNewVersion;
  CStdString strChangeLog;
  ManualUpgradeState upgState = Upgrade_Check_None;
  {
    CSingleLock lock(m_SectionUpgrade);
    strNewVersion = m_strNewVersion;
    strChangeLog = m_strChangeLog;
    upgState = m_eUpgradeState;
  }

  CStdString strCurVersion;
  strCurVersion	= VDMUtils::Instance().GetCurrentVersion();
  strCurVersion	= VDMUtils::ConvertVersionString(strCurVersion);
  strCurVersion.Format(g_localizeStrings.Get(60243).c_str(), strCurVersion.c_str());
  SET_CONTROL_LABEL(CONTROL_LABEL_UPGRADE_CURVERSION, strCurVersion);

  strNewVersion = VDMUtils::ConvertVersionString(strNewVersion);
  strNewVersion.Format(g_localizeStrings.Get(60239).c_str(), strNewVersion.c_str());
  SET_CONTROL_LABEL(CONTROL_LABEL_UPGRADE_VERSION, strNewVersion);
  SET_CONTROL_LABEL(CONTROL_LABEL_UPGRADE_CHANGELOG, strChangeLog);

  SET_CONTROL_HIDDEN(CONTROL_GROUP_UPGRADE_CHECK);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_CHECK);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);  
  SET_CONTROL_HIDDEN(CONTROL_LABEL_CHECKING_NEWVERSION);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_LASTVERSION);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_CHECK_NEWVER_FALIED);
  SET_CONTROL_HIDDEN(CONTROL_GROUP_UPGRADE_NEW_VERSION);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_DOWNLOAD);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL);
  SET_CONTROL_HIDDEN(CONTROL_GROUP_UPGRADE_DOWNLOAD);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_DOWNLOADING);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_DOWNLOAD_FAILED);
  SET_CONTROL_HIDDEN(CONTROL_BUTTON_UPGRADE_INSTALL);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_DOWNLOAD_DONE);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_INSTALLING);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_INSTALL_SUCC);
  SET_CONTROL_HIDDEN(CONTROL_LABEL_UPGRADE_INSTALL_FAIL);

  CONTROL_ENABLE(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);
  CONTROL_ENABLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD);

  switch (upgState)
  {
  case Upgrade_Check_None:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_CHECK);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_CHECK,0);
      break;
    }
  case Upgrade_Checking:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_CHECK);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_CHECKING_NEWVERSION);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);
      CONTROL_DISABLE(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL,0);
      break;
    }
  case Upgrade_Check_Last:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_CHECK);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_LASTVERSION);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL,0);
      break;
    }
  case Upgrade_Check_NewAvailable:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL,0);
      break;
    }
  case Upgrade_Check_Failed:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_CHECK);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_CHECK_NEWVER_FALIED);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_CHECK_RESTART);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_CHECK_CANCEL,0);
      break;
    }
  case Upgrade_Downloading:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL);
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_DOWNLOADING);
      CONTROL_DISABLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD);

      CGUISliderControl* pSliderDownload = (CGUISliderControl*)GetControl(CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD);
      if (pSliderDownload)
      {
        CStdString strPercentage;
        float fPercentage = pSliderDownload->GetPercentage();
        strPercentage.Format("%.1f", fPercentage);
        strPercentage += "%";

        SET_CONTROL_LABEL(CONTROL_LABEL_UPGRADE_DOWNLOAD, strPercentage);
      }
      break;
    }
  case Upgrade_Download_Done:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_DOWNLOAD_DONE);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_INSTALL);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL,0);
      break;
    }
  case Upgrade_Download_Failed:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL);
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_DOWNLOAD);
      SET_CONTROL_HIDDEN(CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_DOWNLOAD_FAILED);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_DOWNLOAD_CANCEL,0);
      break;
    }
  case Upgrade_Installing:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_INSTALLING);

      break;
    }
  case Upgrade_Install_Done:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_INSTALL_SUCC);

      break;
    }
  case Upgrade_Install_Failed:
    {
      SET_CONTROL_VISIBLE(CONTROL_GROUP_UPGRADE_NEW_VERSION);
      SET_CONTROL_VISIBLE(CONTROL_LABEL_UPGRADE_INSTALL_FAIL);
      SET_CONTROL_VISIBLE(CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL);
      SET_CONTROL_FOCUS(CONTROL_BUTTON_UPGRADE_INSTALL_CANCEL,0);
      break;
    }
  default:

    break;
  }
}

//upgrade state update
void CVDMSystemUpdateDlg::UpdateManualUpgradeState()
{
  VMCUpgradeInfo upgInfo;
  CVDMUpgradeManager::Instance().GetUpgradeInfo(upgInfo);
  UPGRADE_PROGRESS upgProcess = CVDMUpgradeManager::Instance().GetUpgradeProgress();
  UPGRADE_STATUS upgStatus = CVDMUpgradeManager::Instance().GetUpgradeStatus();

  CSingleLock lock(m_SectionUpgrade);
  m_strNewVersion = upgInfo.strNewVersion;
  m_strChangeLog = upgInfo.strChangeLog;

  if (upgProcess == UPG_PROGRESS_CHECK_VERSION)
  {
    m_eUpgradeState = Upgrade_Check_None;
    if (upgStatus == UPG_STATUS_STARTED || upgStatus == UPG_STATUS_PAUSED)
    {
      m_eUpgradeState = Upgrade_Checking;
    }
    else if (upgStatus == UPG_STATUS_ENDED)
    {
      if (upgInfo.curVersionStatus == VERSION_STATUS_LATEST)
      {
        m_eUpgradeState = Upgrade_Check_Last;
      }
      else if (upgInfo.curVersionStatus == VERSION_STATUS_NORMAL)
      {
        m_eUpgradeState = Upgrade_Check_NewAvailable;
      }
      else
      {
        m_eUpgradeState = Upgrade_Check_Failed;
      }
    }
    else if (upgStatus == UPG_STATUS_STOPPED)
    {
      m_eUpgradeState = Upgrade_Check_Failed;
    }
  }
  else if (upgProcess == UPG_PROGRESS_DOWNLOAD)
  {
    m_eUpgradeState = Upgrade_Check_NewAvailable;
    if (upgStatus == UPG_STATUS_STARTED || upgStatus == UPG_STATUS_PAUSED)
    {
      m_eUpgradeState = Upgrade_Downloading;
    }
    else if (upgStatus == UPG_STATUS_ENDED)
    {
      m_eUpgradeState = Upgrade_Download_Done;
    }
    else if (upgStatus == UPG_STATUS_STOPPED)
    {
      m_eUpgradeState = Upgrade_Download_Failed;
    }
  }
  else if (upgProcess == UPG_PROGRESS_INSTALL)
  {
    m_eUpgradeState = Upgrade_Download_Done;
    if (upgStatus == UPG_STATUS_STARTED || upgStatus == UPG_STATUS_PAUSED)
    {
      m_eUpgradeState = Upgrade_Installing;
    }
    else if (upgStatus == UPG_STATUS_ENDED)
    {
      m_eUpgradeState = Upgrade_Install_Done;
    }
    else if (upgStatus == UPG_STATUS_STOPPED)
    {
      m_eUpgradeState = Upgrade_Install_Failed;
    }
  }

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

//upgrade callbacks
void CVDMSystemUpdateDlg::OnCheckNewVersionStarted()
{
  CSingleLock lock(m_SectionUpgrade);
  m_eUpgradeState = Upgrade_Checking;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnCheckNewVersionFailed(const CStdString& strFailReason)
{

  CSingleLock lock(m_SectionUpgrade);
  m_eUpgradeState = Upgrade_Check_Failed;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnCheckNewVersionResult(const VMCUpgradeInfo& upgradeInfo, bool& bContinueDownload)
{
  CSingleLock lock(m_SectionUpgrade);
  bContinueDownload = false;

  m_strNewVersion.Empty();
  m_strChangeLog.Empty();

  if (upgradeInfo.curVersionStatus == VERSION_STATUS_NORMAL || upgradeInfo.curVersionStatus == VERSION_STATUS_FREEVER_UPGRADE)
  {
    m_strNewVersion = upgradeInfo.strNewVersion;
    m_strChangeLog = upgradeInfo.strChangeLog;
    m_eUpgradeState = Upgrade_Check_NewAvailable;
  }
  else
  {
    if (upgradeInfo.curVersionStatus == VERSION_STATUS_LATEST)
    {
      m_eUpgradeState = Upgrade_Check_Last;
    }
    else
    {
      m_eUpgradeState = Upgrade_Check_Failed;
    }
  }

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadStarted(const CStdString& strFileUrl, const CStdString& strDownloadPath)
{
  CSingleLock lock(m_SectionUpgrade);
  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadProgress(const CStdString& strFileUrl, int nDownloadSize, int nTotalSize)
{
  CSingleLock lock(m_SectionUpgrade);
  m_eUpgradeState = Upgrade_Downloading;

  CVDMUpdateScheduleDlg* pDlg = (CVDMUpdateScheduleDlg*)g_windowManager.GetWindow(VDM_WINDOW_UPDATE_SCHEDULE);
  if(pDlg && pDlg->IsActive())
  {   
    pDlg->Close();
  }

  CVDMSliderControl* pSlider = (CVDMSliderControl*)GetControl(CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD);
  if (pSlider)
  {
    if (nTotalSize <= 0)
    {
      nTotalSize = 1;
    }

    if (nDownloadSize < 0)
    {
      nDownloadSize = 0;
    }
    else if (nDownloadSize > nTotalSize)
    {
      nDownloadSize = nTotalSize;
    }

    pSlider->SetPercentage(nDownloadSize / (float)nTotalSize * 100);
  }

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadStopped(const CStdString& strFileUrl, const CStdString& strStopReason, bool& bRestart)
{
  CSingleLock lock(m_SectionUpgrade);
  bRestart = false;
  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadEnded(const CStdString& strFileUrl, int nFilesize, const CStdString& strDownloadPath)
{
  CSingleLock lock(m_SectionUpgrade);

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadFilesStarted()
{
  CSingleLock lock(m_SectionUpgrade);
  m_eUpgradeState = Upgrade_Downloading;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadFilesStopped(const CStdString& strStopReason, bool& bRestart)
{
  CSingleLock lock(m_SectionUpgrade);
  bRestart = false;
  m_eUpgradeState = Upgrade_Download_Failed;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnDownloadFilesEnded(const CStdStringArray& downloadFiles, bool& bContinueInstall)
{
  CSingleLock lock(m_SectionUpgrade);
  bContinueInstall = false;
  m_eUpgradeState = Upgrade_Download_Done;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnInstallStarted()
{
  CSingleLock lock(m_SectionUpgrade);
 // m_eUpgradeState = Upgrade_Installing;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnInstallProgress(int nProgress, int nTotal)
{
  CSingleLock lock(m_SectionUpgrade);
 // m_eUpgradeState = Upgrade_Installing;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnInstallStopped(const CStdString& strStopReason, bool& bRestart)
{
  CSingleLock lock(m_SectionUpgrade);
  bRestart = false;
  m_eUpgradeState = Upgrade_Install_Failed;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}

void CVDMSystemUpdateDlg::OnInstallEnded(bool bSuccessful)
{
  CSingleLock lock(m_SectionUpgrade);
 // m_eUpgradeState = Upgrade_Install_Done;

  SEND_TMSG_TO_UPDATE_UPGRADE;
}


#endif