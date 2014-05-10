
#if defined(__VIDONME_MEDIACENTER__)

#include "VDMDumpUpload.h"
#include "guilib/LocalizeStrings.h"
#include "vidonme/upgrade/VDMUpload.h"
#include "utils/JobManager.h"
#include "utils/URIUtils.h"
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "filesystem/SpecialProtocol.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "FileItem.h"

class CDumpSendJob : public CJob
{
public:
	CDumpSendJob()
	{
		m_bSuccess = false;
	}

	bool IsSendSuccess(void) const
	{
		return m_bSuccess;
	}

protected:
	virtual bool DoWork()
	{
		CFileItemList itemlist;
		XFILE::CDirectory::GetDirectory(g_settings.m_logFolder, itemlist, ".dmp");

		CStdStringArray  vecPaths;
		for (int i = 0 ; i < itemlist.Size(); ++i)
		{
			CStdString strPath = itemlist[i]->GetPath();

			if (itemlist[i]->IsFileFolder() || strPath.Find(".dmp") < 0)
			{
				continue;
			}

			vecPaths.push_back(itemlist.Get(i)->GetPath());
		}

		if (vecPaths.empty())
		{
			m_bSuccess = false;
			return true;
		}

    if (!vecPaths.empty())
    {
      CStdString strLogFilePath = CSpecialProtocol::TranslatePath(g_settings.m_logFolder);
      if (!URIUtils::HasSlashAtEnd(strLogFilePath))
      {
        URIUtils::AddSlashAtEnd(strLogFilePath);
      }

      strLogFilePath += "VDMMediaCenter.old.log";
      if (XFILE::CFile::Exists(strLogFilePath))
      {
        vecPaths.push_back(strLogFilePath);
      }
    }
    
    m_bSuccess = VidOnMe::CVDMUpload::Upload(vecPaths);
		if (!m_bSuccess)
		{
			return false;
		}

		for (int i = 0 ; i < vecPaths.size() ; ++i)
		{
			XFILE::CFile::Delete(vecPaths[i]);
		}

		return true;
	}
private:
	CStdString		m_strForumID;
	CStdString		m_strEmail;
	bool					m_bSuccess;
};

CVDMDumpUpload::CVDMDumpUpload(void) 
{
	m_nSendDumpJob	= 0;
}

CVDMDumpUpload::~CVDMDumpUpload(void)
{
	
}

void CVDMDumpUpload::CheckDump(void)
{
	m_nSendDumpJob = CJobManager::GetInstance().AddJob(new CDumpSendJob(), this, CJob::PRIORITY_HIGH);
}

void CVDMDumpUpload::OnJobComplete(unsigned int jobID, bool success, CJob *job)
{
	if (jobID == m_nSendDumpJob)
	{
		m_nSendDumpJob = 0;

		if (success)
		{
			CDumpSendJob* pJob = (CDumpSendJob*)job;
			if (pJob && pJob->IsSendSuccess())
			{
				CLog::Log(LOGINFO, "Upload dump file success.");
			}

			return;
		}

		CLog::Log(LOGERROR, "Upload dump file failed");
	}
}

#endif