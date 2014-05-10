
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "guilib/GUIDialog.h"

class CVDMFreeVersionNoteDlg : public CGUIDialog
{
public:
	CVDMFreeVersionNoteDlg(void);
	virtual ~CVDMFreeVersionNoteDlg(void);
	virtual bool OnMessage(CGUIMessage& message);
	static void ShowVersionInfo(void);
};

#endif