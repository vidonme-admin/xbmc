
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMUpdateError.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/Builtins.h"
#include "guilib/LocalizeStrings.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#define		CONTROL_LABLE_ERROR				50

#define		CONTROL_BUTTON_RETRY				100
#define		CONTROL_BUTTON_CANCEL				200

using namespace VidOnMe;


CVDMUpdateErrorDlg::CVDMUpdateErrorDlg(void) 
	: CVDMDialog(VDM_WINDOW_UPDATE_ERROR)
{
	m_loadType = LOAD_ON_GUI_INIT;
  m_bPressYes = false;
  m_nType = Upgrade_Error_None;

	AddDefaultControl(100);
	AddGroup(290,160,700, 300);
	AddImage(0, 0, 700, 300, "special://xbmc/media/VersionNote_BK1.png");
	AddImage(40, 25, 620, 250, "special://xbmc/media/VersionNote_BK2.png");
	AddLabel(80, 60, 540, 60, "70028", CONTROL_LABLE_ERROR, "font14", "FFFFFFFF", "true");
	AddLabel(80, 120, 540, 60, "70029", 0, "font14", "FFFFFFFF", "true");

	AddButton(180, 200, 140, 40, "70030", CONTROL_BUTTON_RETRY, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
	"FF999999", "FFFFFFFF", 20, "", "center", "center",  200, 200, 200, 200);

	AddButton(380, 200, 140, 40, "70031", CONTROL_BUTTON_CANCEL, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
		"FF999999", "FFFFFFFF", 20, "", "center", "center", 100, 100, 100, 100);
}

CVDMUpdateErrorDlg::~CVDMUpdateErrorDlg(void)
{

}

bool CVDMUpdateErrorDlg::ShowInfo(UpgradeErrorType type)
{
	CVDMUpdateErrorDlg* pDlg = (CVDMUpdateErrorDlg*)g_windowManager.GetWindow(VDM_WINDOW_UPDATE_ERROR);

  pDlg->m_nType = type;

	if (pDlg)
	{
		pDlg->DoModal();
	}

	return pDlg->m_bPressYes;;
}

void CVDMUpdateErrorDlg::OnInitWindow()
{
  if(m_nType ==  Upgrade_Error_NoSpace)
  {
    SET_CONTROL_LABEL(CONTROL_LABLE_ERROR,g_localizeStrings.Get(70044));
  }
  else
  {
    SET_CONTROL_LABEL(CONTROL_LABLE_ERROR,g_localizeStrings.Get(70028));
  }
	CVDMDialog::OnInitWindow();
}

bool CVDMUpdateErrorDlg::OnMessage(CGUIMessage& message)
{
  switch (message.GetMessage())
  {
  case GUI_MSG_CLICKED:
    {
      OnClick(message);
      return true;
    }
  } 
  return CVDMDialog::OnMessage(message);
}

void CVDMUpdateErrorDlg::OnClick(const CGUIMessage& message)
{
  switch (message.GetSenderId())
  {
  case CONTROL_BUTTON_RETRY:
    {
      m_bPressYes = true;
      Close();
      break;
    }
  case CONTROL_BUTTON_CANCEL:
    {
      m_bPressYes = false;
      Close();
      break;
    }
  default:
    break;
  }
}

#endif