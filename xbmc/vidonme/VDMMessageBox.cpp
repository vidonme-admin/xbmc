
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMMessageBox.h"
#include "guilib/Key.h"
#include "guilib/GUILabelControl.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "interfaces/Builtins.h"


#define		LABEL_LINE_HIGHT									30

#define		CONTROL_LABEL_MESSAGE				100
#define		CONTROL_BUTTON_YES					200
#define		CONTROL_BUTTON_OK						201
#define		CONTROL_BUTTON_NO						202
#define		CONTROL_BUTTON_MESSAGE			300

CVDMMessageBox::CVDMMessageBox(void) 
	: CVDMDialog(VDM_WINDOW_MESSAGEBOX)
{
	m_bPressYes		= false;
	m_bModeYesNo	= false;
	
	m_strLabelYes	= g_localizeStrings.Get(107);
	m_strLabelNo	= g_localizeStrings.Get(106);
	m_strLabelOK	= g_localizeStrings.Get(186);
	
	m_loadType	=  KEEP_IN_MEMORY;

	AddGroup(400,240,800, 400);
	AddImage(0, 0, 500, 220, "special://xbmc/media/title_bg.png");
	AddImage(5, 5, 490, 210, "special://xbmc/media/right_bg.png");
	AddLabel(30, 25, 440, 120, "", 100, "font13", "white", "true", "left", "center");

	AddButton(30, 25, 440, 120, "", 300, "","", "font13", "FF2E8ECB", "blue", 0, "", 
		"left", "center", 300, 300, 201, 201, "true");

	AddButton(30, 150, 200, 50, "107", 200, "special://xbmc/media/path_btn_hover.png", 
		"special://xbmc/media/path_btn.png", "font12", "white", "white", 20, "", 
		"center", "center", 202, 202, 200, 200);

	AddButton(150, 150, 200, 50, "186", 201, "special://xbmc/media/path_btn_hover.png", 
		"special://xbmc/media/path_btn.png", "font12", "white", "white", 20, "", 
		"center", "center", 201, 201, 300, 300);

	AddButton(270, 150, 200, 50, "106", 202, "special://xbmc/media/path_btn_hover.png", 
		"special://xbmc/media/path_btn.png", "font12", "white", "white", 20, "", 
		"center", "center", 200, 200, 202, 202);
}

CVDMMessageBox::~CVDMMessageBox(void)
{
	
}

bool CVDMMessageBox::ShowMessageYesNo(CStdString strMessage, CStdString strLabelYes, CStdString strLabelNo)
{
	CVDMMessageBox*		pDlg = (CVDMMessageBox*)g_windowManager.GetWindow(VDM_WINDOW_MESSAGEBOX);

	if (!pDlg)
	{
		return false;
	}

	pDlg->m_strMessage	= strMessage;
	
	if (!strLabelYes.IsEmpty())
	{
		pDlg->m_strLabelYes	= strLabelYes;
	}
	else
	{
		pDlg->m_strLabelYes	= g_localizeStrings.Get(107);
	}

	if (!strLabelNo.IsEmpty())
	{
		pDlg->m_strLabelNo	= strLabelNo;
	}
	else
	{
		pDlg->m_strLabelNo	= g_localizeStrings.Get(106);
	}

	pDlg->m_bModeYesNo	= true;

	pDlg->DoModal();

	return pDlg->m_bPressYes;
}

void CVDMMessageBox::ShowMessageOK(CStdString strMessage, CStdString strLabelOK, CStdString strURL)
{
	CVDMMessageBox*		pDlg = (CVDMMessageBox*)g_windowManager.GetWindow(VDM_WINDOW_MESSAGEBOX);

	if (!pDlg)
	{
		return;
	}

	pDlg->m_strMessage	= strMessage;
	pDlg->m_strURL			= strURL;

	if (!strLabelOK.IsEmpty())
	{
		pDlg->m_strLabelOK	= strLabelOK;
	}
	else
	{
		pDlg->m_strLabelOK	= g_localizeStrings.Get(186);
	}

	pDlg->m_bModeYesNo	= false;

	pDlg->DoModal();
}

void CVDMMessageBox::OnInitWindow()
{
	m_bMultiLine = false;
	if (m_bModeYesNo)
	{
		m_bPressYes	= false;

		SET_CONTROL_VISIBLE(CONTROL_BUTTON_YES);
		SET_CONTROL_VISIBLE(CONTROL_BUTTON_NO);
		SET_CONTROL_HIDDEN(CONTROL_BUTTON_OK);

		SET_CONTROL_LABEL(CONTROL_BUTTON_YES, m_strLabelYes);
		SET_CONTROL_LABEL(CONTROL_BUTTON_NO, m_strLabelNo);
	}
	else
	{
		SET_CONTROL_HIDDEN(CONTROL_BUTTON_YES);
		SET_CONTROL_HIDDEN(CONTROL_BUTTON_NO);
		SET_CONTROL_VISIBLE(CONTROL_BUTTON_OK);
		SET_CONTROL_LABEL(CONTROL_BUTTON_OK, m_strLabelOK);
	}

	if (m_strURL.IsEmpty())
	{
		SET_CONTROL_VISIBLE(CONTROL_LABEL_MESSAGE);
		SET_CONTROL_HIDDEN(CONTROL_BUTTON_MESSAGE);
		SET_CONTROL_LABEL(CONTROL_LABEL_MESSAGE , m_strMessage);
	}
	else
	{
		SET_CONTROL_HIDDEN(CONTROL_LABEL_MESSAGE);
		SET_CONTROL_VISIBLE(CONTROL_BUTTON_MESSAGE);
		SET_CONTROL_LABEL(CONTROL_BUTTON_MESSAGE , m_strMessage);
	}

	CVDMDialog::OnInitWindow();
}

void CVDMMessageBox::FrameMove()
{
	CVDMDialog::FrameMove();

	CGUILabelControl* pLabelMessage = (CGUILabelControl*)GetControl(CONTROL_LABEL_MESSAGE);
	if (pLabelMessage)
	{
		CRect	rectRender = pLabelMessage->CalcRenderRegion();
		if (rectRender.Height() >= LABEL_LINE_HIGHT && !m_bMultiLine)
		{
			pLabelMessage->SetAlignment(XBFONT_LEFT | XBFONT_CENTER_Y);	
			pLabelMessage->SetLabel(m_strMessage);

			m_bMultiLine = true;
		}
		else if (rectRender.Height() < LABEL_LINE_HIGHT && m_bMultiLine)
		{
			pLabelMessage->SetAlignment(XBFONT_CENTER_X | XBFONT_CENTER_Y);
			pLabelMessage->SetLabel(m_strMessage);

			m_bMultiLine = false;
		}
	}
}

bool CVDMMessageBox::OnAction(const CAction &action)
{
  return CVDMDialog::OnAction(action);
}

bool CVDMMessageBox::OnMessage(CGUIMessage& message)
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

void CVDMMessageBox::OnClick(CGUIMessage& message)
{
	switch (message.GetSenderId())
	{
	case CONTROL_BUTTON_YES:
		{
			Close();
			m_bPressYes = true;
			break;
		}
	case CONTROL_BUTTON_NO:
		{
			Close();
			m_bPressYes = false;
			break;
		}
	case CONTROL_BUTTON_OK:
		{
			Close();
			break;
		}
	case CONTROL_BUTTON_MESSAGE:
		{
			CStdString strExecute;
			strExecute.Format("VDMOpenURL(%s)", m_strURL);
			CBuiltins::Execute(strExecute);
			break;
		}
	default:
		{
			break;
		}
	}
}

#endif