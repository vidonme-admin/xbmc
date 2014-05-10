
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"

class CVDMFeatureLimitNoteDlg : public CVDMDialog
{
public:
	CVDMFeatureLimitNoteDlg(void);
	virtual ~CVDMFeatureLimitNoteDlg(void);

	static bool ShowInfo(void);

protected:
	virtual void OnInitWindow();

private:
	bool IsSendEmail(void);

private:
	bool		m_bIsSendEmail;
};

#endif