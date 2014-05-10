
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "vidonme/VDMDialog.h"

class CVDMBuyNoteDlg : public CVDMDialog
{
public:
	CVDMBuyNoteDlg(void);
	virtual ~CVDMBuyNoteDlg(void);
	virtual void OnInitWindow();
};

#endif