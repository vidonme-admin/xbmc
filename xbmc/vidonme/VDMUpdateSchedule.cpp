
#if defined(__VIDONME_MEDIACENTER__)

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

#define CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD						100


CVDMUpdateScheduleDlg::CVDMUpdateScheduleDlg(void) 
	: CVDMDialog(VDM_WINDOW_UPDATE_SCHEDULE)
{
  m_loadType = LOAD_ON_GUI_INIT;

  AddGroup(250,0,1280, 720);
  AddImage(396, 0, 388, 35, "special://xbmc/media/BuyNote_BK1.png");
  AddImage(409, 0, 362, 23, "special://xbmc/media/BuyNote_BK2.png");
  AddSlider(407, 23, 350, 12, CONTROL_VDMSLIDER_UPGRADE_DOWNLOAD, "special://xbmc/media/OSDKey_LightSliderNF.png", "special://xbmc/media/OSDKey_LightSliderFO.png", "","");
  AddLabel(410, 0, 315, 20, "70027", 0, "font12", "FFFFFFFF");

}

CVDMUpdateScheduleDlg::~CVDMUpdateScheduleDlg(void)
{

}

void CVDMUpdateScheduleDlg::OnInitWindow()
{
  CVDMUpgradeManager::Instance().AddCallback(this);
  CVDMDialog::OnInitWindow();
}

void CVDMUpdateScheduleDlg::FrameMove()
{
  if(g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
  {
    SetVisible(false, true);
  }
  else
  {
    SetVisible(true);
  }
}

bool CVDMUpdateScheduleDlg::OnMessage(CGUIMessage& message)
{
  return CVDMDialog::OnMessage(message);
}
void CVDMUpdateScheduleDlg::OnDownloadProgress(const CStdString& strFileUrl, int nDownloadSize, int nTotalSize)
{
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
}

#endif