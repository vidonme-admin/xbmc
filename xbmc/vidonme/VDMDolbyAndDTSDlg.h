
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"

class CVDMDolbyAndDTSDlg : public CVDMDialog
{
public:
	CVDMDolbyAndDTSDlg(void);
	virtual ~CVDMDolbyAndDTSDlg(void);

	static bool ShowInfo(void);

	virtual bool OnMessage(CGUIMessage& message);
protected:
	virtual void OnInitWindow();

private:
	bool		m_bIsSendEmail;
};

#endif