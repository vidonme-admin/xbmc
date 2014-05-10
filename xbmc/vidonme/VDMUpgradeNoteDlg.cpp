
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMUpgradeNoteDlg.h"
#include "guilib/Key.h"
#include "guilib/GUILabelControl.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUIWindowManager.h"

#define		CONTROL_LABEL_VERSION				100
#define		CONTROL_LABEL_CHANGELOG			300
#define		CONTROL_BUTTON_UPGRADE			400
#define		CONTROL_BUTTON_CANCEL				500
#define		CONTROL_BUTTON_IGNORE				600

CVDMUpgradeNoteDlg::CVDMUpgradeNoteDlg(void) 
	: CVDMDialog(VDM_WINDOW_UPGRADE_NOTE)
{
	m_loadType		=  KEEP_IN_MEMORY;
	m_eOperate		= UpgradeLater;

  AddDefaultControl(400);
  AddGroup(230,75,800,550);
  AddImage(0, 0, 800, 550, "special://xbmc/media/title_bg.png");
  AddImage(5, 5, 790, 540, "special://xbmc/media/right_bg.png");
  AddLabel(50, 25, 700, 20, "70006", 0, "font14", "FF7C7C7C", "true", "left", "center");
  AddLabel(50, 50, 700, 20, "", 100, "font14", "FF7C7C7C", "true", "left", "center");
  AddLabel(50, 80, 700, 20, "60240", 200, "font16", "white", "true", "left", "center");
  AddLabel(50, 110, 700, 340, "", 300, "font14", "FF7C7C7C","true");

  AddButton(50, 460, 200, 50, "60241", 400, "special://xbmc/media/path_btn_hover.png", 
    "special://xbmc/media/path_btn.png", "font14", "white", "white", 20, "", 
    "center", "center", 600, 500, 400, 400);

  AddButton(300, 460, 200, 50, "60002", 500, "special://xbmc/media/path_btn_hover.png", 
    "special://xbmc/media/path_btn.png", "font14", "white", "white", 20, "", 
    "center", "center", 400, 600, 500, 500);

  AddButton(550, 460, 200, 50, "60242", 600, "special://xbmc/media/path_btn_hover.png", 
    "special://xbmc/media/path_btn.png", "font14", "white", "white", 20, "", 
    "center", "center", 500, 400, 600, 600);
}

CVDMUpgradeNoteDlg::~CVDMUpgradeNoteDlg(void)
{
	
}

void CVDMUpgradeNoteDlg::ShowUpgrade(CStdString strVersion, CStdString strChangeLog, UpgradeOperate& eOperate)
{
	CVDMUpgradeNoteDlg* pDlg = (CVDMUpgradeNoteDlg*)g_windowManager.GetWindow(VDM_WINDOW_UPGRADE_NOTE);
	if (pDlg)
	{
		pDlg->SetVersion(strVersion);
		pDlg->SetChangeLog(strChangeLog);
		pDlg->DoModal();
		eOperate = pDlg->GetOperate();
	}
}


void CVDMUpgradeNoteDlg::SetVersion(CStdString strVersion)
{
	m_strVersion = ConvertVersionString(strVersion);
}

void CVDMUpgradeNoteDlg::SetChangeLog(CStdString strChangeLog)
{
	m_strChangeLog = strChangeLog;
}

UpgradeOperate CVDMUpgradeNoteDlg::GetOperate(void)
{
	return m_eOperate;
}

void CVDMUpgradeNoteDlg::OnInitWindow()
{
	CStdString strVersion;
	strVersion.Format(g_localizeStrings.Get(70007).c_str(), m_strVersion.c_str());

	SET_CONTROL_LABEL(CONTROL_LABEL_CHANGELOG, m_strChangeLog);
	SET_CONTROL_LABEL(CONTROL_LABEL_VERSION, strVersion);

	CVDMDialog::OnInitWindow();
}

bool CVDMUpgradeNoteDlg::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
	case GUI_MSG_CLICKED:
		{
			OnClick(message);
			return true;
		}
	default:
		{
			break;
		}
	}
	return CVDMDialog::OnMessage(message);    
}

void CVDMUpgradeNoteDlg::OnClick(CGUIMessage& message)
{
	switch (message.GetSenderId())
	{
	case CONTROL_BUTTON_UPGRADE:
		{
			m_eOperate = UpgradeImmediate;
			Close();
			break;
		}
	case CONTROL_BUTTON_IGNORE:
		{
			m_eOperate = IgnoreThisVersion;
			Close();
			break;
		}
	case CONTROL_BUTTON_CANCEL:
		{
			m_eOperate = UpgradeLater;
			Close();
			break;
		}
	default:
		{
			break;
		}
	}
}

CStdString CVDMUpgradeNoteDlg::ConvertVersionString(CStdString& numVersion)
{
  if (numVersion.GetLength() >= 3)
  {
    int len = numVersion.GetLength();
    numVersion.Insert(len - 3, '.');
    numVersion.Insert(len - 1, '.');
    numVersion.Insert(len + 1, '.');
  }

  return numVersion;
}


#endif