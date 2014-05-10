
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "guilib/GUIListItem.h"
#include "VDMDialog.h"
#include "utils/StdString.h"

class CVDMMessageBox : public CVDMDialog
{
public:
	CVDMMessageBox(void);
	virtual ~CVDMMessageBox(void);
	static bool ShowMessageYesNo(CStdString strMessage, CStdString strLabelYes = "", CStdString strLabelNo = "");
	static void ShowMessageOK(CStdString strMessage, CStdString strLabelOK = "" , CStdString strURL = "");

protected:
	virtual void OnInitWindow();
	virtual bool OnMessage(CGUIMessage& message);
	virtual bool OnAction(const CAction &action);
	virtual void FrameMove();

private:
	void OnClick(CGUIMessage& message);

private:
	CStdString	m_strMessage;
	CStdString	m_strLabelYes;
	CStdString	m_strLabelNo;
	CStdString	m_strLabelOK;
	CStdString	m_strURL;
	bool				m_bModeYesNo;

private:
	bool				m_bPressYes;
	bool				m_bMultiLine;
};

#endif