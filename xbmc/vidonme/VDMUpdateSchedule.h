
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

class CVDMUpdateScheduleDlg : public CVDMDialog
                            , public VidOnMe::IVDMUpgradeCallback
{
public:
	CVDMUpdateScheduleDlg(void);
	virtual ~CVDMUpdateScheduleDlg(void);

	static bool ShowInfo(void);

protected:
	virtual void OnInitWindow();
  virtual void FrameMove();
  virtual bool OnMessage(CGUIMessage& message);
//  virtual void OnDownloadStarted(const CStdString& strFileUrl, const CStdString& strDownloadPath);
  virtual void OnDownloadProgress(const CStdString& strFileUrl, int nDownloadSize, int nTotalSize);
//  virtual void OnDownloadStopped(const CStdString& strFileUrl, const CStdString& strStopReason, bool& bRestart);
  virtual bool IsVisible() const { return CGUIControl::IsVisible(); }
private:
 void OnClick(const CGUIMessage& message);
private:
	bool		m_bIsSendEmail;
};

#endif