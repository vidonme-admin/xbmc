
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"
#include "guilib/GUIListItem.h"
#include "utils/StdString.h"

enum UpgradeOperate
{
	UpgradeImmediate,
	UpgradeLater,
	IgnoreThisVersion,
};

class CVDMUpgradeNoteDlg : public CVDMDialog
{
public:
	CVDMUpgradeNoteDlg(void);
	virtual ~CVDMUpgradeNoteDlg(void);

	static void ShowUpgrade(CStdString strVersion, CStdString strChangeLog, UpgradeOperate& eOperate);

	void SetVersion(CStdString strVersion);
	void SetChangeLog(CStdString strChangeLog);
	UpgradeOperate GetOperate(void);

protected:
	virtual void OnInitWindow();
	virtual bool OnMessage(CGUIMessage& message);

private:
	void OnClick(CGUIMessage& message);

private:
	CStdString ConvertVersionString(CStdString& numVersion);

private:
	CStdString				m_strVersion;
	CStdString				m_strChangeLog;
	UpgradeOperate		m_eOperate;
};

#endif