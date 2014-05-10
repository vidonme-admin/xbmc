
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMFeatureLimitNoteDlg.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/Builtins.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#define		CONTROL_BUTTON_SEND				100

CVDMFeatureLimitNoteDlg::CVDMFeatureLimitNoteDlg(void) 
	: CVDMDialog(VDM_WINDOW_FEATURE_LIMIT_NOTE)
{
	m_loadType = LOAD_ON_GUI_INIT;

	AddDefaultControl(100);
	AddGroup(390,160,500, 300);
	AddImage(0, 0, 500, 300, "special://xbmc/media/VersionNote_BK1.png");
	AddImage(40, 25, 420, 250, "special://xbmc/media/VersionNote_BK2.png");
	AddLabel(80, 60, 340, 120, "70003", 0, "font14", "FFFFFFFF", "true");

	AddButton(80, 200, 140, 40, "70004", 100, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
	"FF999999", "FFFFFFFF", 20, "VDMOpenURL(http://vidon.me/vidonme_tv.htm)", "center", "center",  200, 200, 200, 200);

	AddButton(280, 200, 140, 40, "60002", 200, "special://xbmc/media/BuyNote_OKFO.png", "special://xbmc/media/BuyNote_OKNF.png", "font12", 
		"FF999999", "FFFFFFFF", 20, "PreviousMenu", "center", "center", 100, 100, 100, 100);
}

CVDMFeatureLimitNoteDlg::~CVDMFeatureLimitNoteDlg(void)
{

}

bool CVDMFeatureLimitNoteDlg::ShowInfo(void)
{
	CVDMFeatureLimitNoteDlg* pDlg = (CVDMFeatureLimitNoteDlg*)g_windowManager.GetWindow(VDM_WINDOW_FEATURE_LIMIT_NOTE);

	if (pDlg)
	{
		pDlg->DoModal();
		return pDlg->IsSendEmail();
	}

	return false;
}

void CVDMFeatureLimitNoteDlg::OnInitWindow()
{
	m_bIsSendEmail		= true;

	CVDMDialog::OnInitWindow();
}

bool CVDMFeatureLimitNoteDlg::IsSendEmail(void)
{
	return m_bIsSendEmail;
}

#endif