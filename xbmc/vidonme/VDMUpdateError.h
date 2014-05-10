
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"
#include "vidonme/upgrade/VDMUpgrade.h"
#include <vector>
#include <map>
#include "guilib/GUIWindow.h"
#include "view/GUIViewControl.h"
#include "FileItem.h"
#include "VDMUtils.h"

using namespace VidOnMe;

enum UpgradeErrorType
{
  Upgrade_Error_None,
  Upgrade_Error_NetWork,
  Upgrade_Error_NoSpace,
};

class CVDMUpdateErrorDlg : public CVDMDialog
                              , public VidOnMe::IVDMUpgradeCallback
{
public:
	CVDMUpdateErrorDlg(void);
	virtual ~CVDMUpdateErrorDlg(void);

	static bool ShowInfo(UpgradeErrorType type);

protected:
  virtual void OnInitWindow();
  virtual bool OnMessage(CGUIMessage& message);

private:
  void OnClick(const CGUIMessage& message);

private:
	bool		m_bPressYes;
  UpgradeErrorType m_nType;
};

#endif