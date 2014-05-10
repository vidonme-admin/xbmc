
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"

class CVDMFeatureLimitNoteDlgEx : public CVDMDialog
{
public:
	CVDMFeatureLimitNoteDlgEx(void);
	virtual ~CVDMFeatureLimitNoteDlgEx(void);

	static bool ShowInfo(void);

protected:
	virtual void OnInitWindow();
	virtual bool OnMessage(CGUIMessage& message);

private:
	bool IsSendEmail(void);

private:
	bool		m_bIsSendEmail;
};

#endif