
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMFeatureLimitNoteDlgEx.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/Builtins.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#define		CONTROL_BUTTON_SEND				100

CVDMFeatureLimitNoteDlgEx::CVDMFeatureLimitNoteDlgEx(void) 
	: CVDMDialog(VDM_WINDOW_FEATURE_LIMIT_NOTEEX)
{
	m_loadType = LOAD_ON_GUI_INIT;

	AddDefaultControl(100);
	AddGroup(242,124,796, 472);
	AddImage(0, 0, 796, 472, "special://xbmc/media/VersionNote_BK1.png");
	AddImage(34, 110, 516, 334, "special://xbmc/media/VersionNote_BK2.png");
	AddLabel(60, 28, 700, 65, "60236", 0, "font14", "FFFFFFFF", "true");
	AddLabel(60, 138, 490, 30,"60320", 0 ,"font14", "FFE49815");
	AddLabel(42, 200, 500, 30,"60321", 0, "font12", "FFC2C2C2");

	AddButton(42, 240, 500, 30, "60322", 200, "", "", "font12", 
		"FF2E8ECB", "blue", 0, "VDMOpenURL(http://www.vidon.me)", 
		"left", "center", 100, 100, 100, 100);

	AddLabel(42, 280, 500, 60, "* ", 0 , "font12", "FFC2C2C2", "true");
	AddLabel(60, 280, 500, 60, "60323", 0 , "font12", "FFC2C2C2", "true");
	AddLabel(42, 345, 500, 60, "60324", 0 , "font12", "FFC2C2C2");
	AddImage(588, 174, 172, 172, "special://xbmc/media/Apple_2D_Code.png");

	AddButton(590, 394, 164, 42, "60325", 100, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
		"FF999999", "FFFFFFFF", 20, "", "center", "center", 200, 200, 200, 200);
}

CVDMFeatureLimitNoteDlgEx::~CVDMFeatureLimitNoteDlgEx(void)
{

}

bool CVDMFeatureLimitNoteDlgEx::ShowInfo(void)
{
	CVDMFeatureLimitNoteDlgEx* pDlg = (CVDMFeatureLimitNoteDlgEx*)g_windowManager.GetWindow(VDM_WINDOW_FEATURE_LIMIT_NOTE);

	if (pDlg)
	{
		pDlg->DoModal();
		return pDlg->IsSendEmail();
	}

	return false;
}

void CVDMFeatureLimitNoteDlgEx::OnInitWindow()
{
	m_bIsSendEmail		= false;

	CVDMDialog::OnInitWindow();
}

bool CVDMFeatureLimitNoteDlgEx::OnMessage(CGUIMessage& message)
{
	if (message.GetMessage() == GUI_MSG_CLICKED)
	{
		switch (message.GetSenderId())
		{
		case CONTROL_BUTTON_SEND:
			{
				// Send email start.
				// Send email end.

				m_bIsSendEmail = true;

				Close();
			}
		default:
			{
				break;
			}
		}
	}

	return CVDMDialog::OnMessage(message);
}

bool CVDMFeatureLimitNoteDlgEx::IsSendEmail(void)
{
	return m_bIsSendEmail;
}

#endif