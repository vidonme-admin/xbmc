
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "VDMDialog.h"
#include "utils/StdString.h"
#include "utils/Job.h"

class CVDMWindowLogSend : public CVDMDialog
												, public IJobCallback
{
public:
	CVDMWindowLogSend(void);
	virtual ~CVDMWindowLogSend(void);

	static void Show(void);

private:
	virtual void OnInitWindow();
	virtual bool OnMessage(CGUIMessage& message);

	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);

private:
	unsigned int m_nSendLogJob;
};

#endif