
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMWindowLogSend.h"
#include "guilib/Key.h"
#include "guilib/GUILabelControl.h"
#include "guilib/GUIWindowManager.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUIEditControl.h"
#include "vidonme/upgrade/VDMUpload.h"
#include "vidonme/VDMMessageBox.h"
#include "utils/JobManager.h"


#define		CONTROL_BUTTON_SEND					100
#define		CONTROL_EDIT_FORUMID				200
#define		CONTROL_EDIT_EMAIL					201
#define		CONTROL_BUTTON_WEB					300
#define		CONTROL_BUTTON_FORUM				301

class CLogSendJob : public CJob
{
public:
	CLogSendJob(const CStdString &strForumID, const CStdString &strEmail)
		: m_strForumID(strForumID)
		, m_strEmail(strEmail)
	{
		m_bSuccess = false;
	}

	bool IsSendSuccess(void) const
	{
		return m_bSuccess;
	}

protected:
	virtual bool DoWork()
	{
		m_bSuccess = VidOnMe::CVDMUpload::Upload(m_strForumID, m_strEmail);
		return true;
	}
private:
	CStdString		m_strForumID;
	CStdString		m_strEmail;
	bool					m_bSuccess;
};

CVDMWindowLogSend::CVDMWindowLogSend(void) 
	: CVDMDialog(VDM_WINDOW_LOG_SEND)
{
	m_loadType	=  KEEP_IN_MEMORY;
	
	m_nSendLogJob	= 0;

	AddGroup(336,150,600, 410);
	AddImage(0, 0, 600, 410, "special://xbmc/media/title_bg.png");
	AddImage(5, 5, 590, 400, "special://xbmc/media/right_bg.png");
	AddLabel(40, 20, 520, 25, "70020", 0, "font16", "FFFFCC20", "false", "center", "up");
	AddLabel(40, 70, 520, 90, "70021", 0, "font12", "white", "true", "left", "up");
	AddLabel(40, 180, 150, 25, "70022", 0, "font12", "white", "false", "left", "center");
	AddLabel(40, 220, 150, 25, "70023", 0, "font12", "white", "false", "left", "center");

	AddEdit(200, 180, 360, 25, "", 200, "special://xbmc/media/button-focus2.png", "special://xbmc/media/button-nofocus.png", 
		"font13", "grey2", "white", "black", "center", 200, 200, 100, 201);
	
	AddEdit(200, 220, 360, 25, "", 201, "special://xbmc/media/button-focus2.png", "special://xbmc/media/button-nofocus.png", 
		"font13", "grey2", "white", "black", "center", 201, 201, 200, 300);

	AddLabel(40, 270, 150, 25, "60330", 0, "font12", "white", "false", "left", "center");
	AddLabel(40, 310, 150, 25, "60331", 0, "font12", "white", "false", "left", "center");

	AddButton(200, 270, 360, 25, "http://www.vidon.me", 300, "", 
		"", "font12", "FF2E8ECB", "blue", 0, "VDMOpenURL(http://www.vidon.me)", 
		"left", "center", 300, 300, 201, 301);

	AddButton(200, 310, 360, 25, "http://forum.vidon.me", 301, "", 
		"", "font12", "FF2E8ECB", "blue", 0, "VDMOpenURL(http://forum.vidon.me)", 
		"left", "center", 301, 301, 300, 100);

	AddButton(200, 350, 200, 40, "60325", 100, "special://xbmc/media/path_btn_hover.png", 
		"special://xbmc/media/path_btn.png", "font13", "white", "white", 20, "", 
		"center", "center", 100, 100, 301, 200);
}

CVDMWindowLogSend::~CVDMWindowLogSend(void)
{
	
}

void CVDMWindowLogSend::Show(void)
{
	CVDMWindowLogSend* pDlg = (CVDMWindowLogSend*)g_windowManager.GetWindow(VDM_WINDOW_LOG_SEND);
	if (pDlg)
	{
		pDlg->DoModal();
	}
}

void CVDMWindowLogSend::OnInitWindow()
{
	CVDMDialog::OnInitWindow();
}

bool CVDMWindowLogSend::OnMessage(CGUIMessage& message)
{
	switch (message.GetMessage())
	{
	case GUI_MSG_CLICKED:
		{
			if (message.GetSenderId() == CONTROL_BUTTON_SEND)
			{
				CStdString strForum = ((CGUIEditControl*)GetControl(CONTROL_EDIT_FORUMID))->GetLabel2();
				CStdString strEmail = ((CGUIEditControl*)GetControl(CONTROL_EDIT_EMAIL))->GetLabel2();

				if( m_nSendLogJob!= 0)
				{
					CJobManager::GetInstance().CancelJob(m_nSendLogJob);
					m_nSendLogJob = 0;
				}

				m_nSendLogJob = CJobManager::GetInstance().AddJob(new CLogSendJob(strForum, strEmail), this, CJob::PRIORITY_HIGH);

				Close();
			}
			return true;
		}
	default:
		{
			break;
		}
	}
	return CVDMDialog::OnMessage(message);    
}

void CVDMWindowLogSend::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	if (jobID == m_nSendLogJob)
	{
		m_nSendLogJob = 0;

		if (success)
		{
			CLogSendJob* pJob = (CLogSendJob*)job;
			if (pJob && pJob->IsSendSuccess())
			{
				CVDMMessageBox::ShowMessageOK(g_localizeStrings.Get(70024).c_str());
				return;
			}
		}

		CVDMMessageBox::ShowMessageOK(g_localizeStrings.Get(70025).c_str(), "" , "http://forum.vidon.me");
	}
}

#endif