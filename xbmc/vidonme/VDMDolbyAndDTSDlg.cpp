
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMDolbyAndDTSDlg.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/Builtins.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#define		CONTROL_BUTTON_SEND				100

CVDMDolbyAndDTSDlg::CVDMDolbyAndDTSDlg(void) 
	: CVDMDialog(VDM_WINDOW_DOLBY_DTS)
{
	m_loadType = LOAD_ON_GUI_INIT;

	AddDefaultControl(100);
	AddGroup(390,160,500, 300);
	AddImage(0, 0, 500, 300, "special://xbmc/media/VersionNote_BK1.png");
	AddImage(40, 25, 420, 250, "special://xbmc/media/VersionNote_BK2.png");
	AddLabel(80, 60, 340, 120, "70005", 0, "font14", "FFFFFFFF", "true");

	AddButton(80, 200, 140, 40, "186", 100, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
	"FF999999", "FFFFFFFF", 20, "VDMOpenURL(http://www.vidon.me)", "center", "center",  200, 200, 200, 200);

	AddButton(280, 200, 140, 40, "60002", 200, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
		"FF999999", "FFFFFFFF", 20, "PreviousMenu", "center", "center", 100, 100, 100, 100);
}

CVDMDolbyAndDTSDlg::~CVDMDolbyAndDTSDlg(void)
{

}

bool CVDMDolbyAndDTSDlg::ShowInfo(void)
{
	CVDMDolbyAndDTSDlg* pDlg = (CVDMDolbyAndDTSDlg*)g_windowManager.GetWindow(VDM_WINDOW_DOLBY_DTS);

	if (pDlg)
	{
		pDlg->DoModal();
		return true;
	}

	return false;
}

void CVDMDolbyAndDTSDlg::OnInitWindow()
{
	m_bIsSendEmail		= true;

	CVDMDialog::OnInitWindow();
}

bool CVDMDolbyAndDTSDlg::OnMessage(CGUIMessage& message)
{
	switch ( message.GetMessage() )
	{
	case GUI_MSG_CLICKED:
		{
			if (message.GetSenderId() == CONTROL_BUTTON_SEND)
			{
				Close();
			}

			break;
		}
	default:
		{
			break;
		}
	}

	return CVDMDialog::OnMessage(message);
}

#endif