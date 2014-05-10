
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "utils/StdString.h"
#include "utils/Job.h"

class CVDMDumpUpload : public IJobCallback
{
public:
	CVDMDumpUpload(void);
	virtual ~CVDMDumpUpload(void);

	static CVDMDumpUpload& Instance()
	{
		static CVDMDumpUpload s_instance;
		return s_instance;
	}

	void CheckDump(void);

private:
	virtual void OnJobComplete(unsigned int jobID, bool success, CJob *job);

private:
	unsigned int m_nSendDumpJob;
};

#endif