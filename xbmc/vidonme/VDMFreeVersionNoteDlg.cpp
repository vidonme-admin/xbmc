
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMFreeVersionNoteDlg.h"
#include "guilib/Key.h"
#include "guilib/GUIWindowManager.h"
#include "interfaces/Builtins.h"

#if defined(TARGET_ANDROID)
#include "android/activity/XBMCApp.h"
#endif

#define		CONTROL_BUTTON_OK				100
#define		CONTROL_BUTTON_CANCEL		200

CVDMFreeVersionNoteDlg::CVDMFreeVersionNoteDlg(void) 
	: CGUIDialog(VDM_WINDOW_FREEVERSION_NOTE, "VDMFreeVersionNote.xml")
{
	m_loadType = LOAD_ON_GUI_INIT;
}

CVDMFreeVersionNoteDlg::~CVDMFreeVersionNoteDlg(void)
{

}

void CVDMFreeVersionNoteDlg::ShowVersionInfo(void)
{
	CVDMFreeVersionNoteDlg* pDlg = (CVDMFreeVersionNoteDlg*)g_windowManager.GetWindow(VDM_WINDOW_FREEVERSION_NOTE);

	if (pDlg)
	{
		pDlg->DoModal();
	}
}

bool CVDMFreeVersionNoteDlg::OnMessage(CGUIMessage& message)
{
	if (message.GetMessage() == GUI_MSG_CLICKED)
	{
		switch (message.GetSenderId())
		{
		case CONTROL_BUTTON_OK:
			{
				CStdString strCmd = "http://www.vidon.me/";
				strCmd.Format("VDMOpenURL(%s)", strCmd);
				CBuiltins::Execute(strCmd);

				Close();
				break;
			}
		case CONTROL_BUTTON_CANCEL:
			{
				Close();
				break;
			}
		default:
			{
				break;
			}
		}
	}

	return CGUIDialog::OnMessage(message);
}

#endif