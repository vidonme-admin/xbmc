#if defined(__VIDONME_MEDIACENTER__)
#pragma once
#include "TextureDatabaseEx.h"
#include "threads/Timer.h"

class CAutoClearCache : public ITimerCallback
{
public:
	CAutoClearCache(int nDiskSpaceMin = 100/*100MB*/,int nCacheCountMax = 500/*500 pictures*/,unsigned long nCacheCycleTime = 30*24*3600/*30x24x3600 second*/);
	virtual ~CAutoClearCache(void);
	virtual void OnTimeout();
public:
	void StartLoop();
	void StopLoop();
	bool ClearCacheOneTimes();
	bool DeleteCacheFiles(const int nCount);
private:
	//return xxMB
	int GetCacheDiskFree();
public:
	CTextureDatabaseEx m_TextureDatabaseEx;
	static time_t string2time(const char * str,const char * formatStr);
private:
	int m_nDiskSpaceMin;
	int m_nCacheCountMax;
	unsigned long m_nCacheCycleTime;
	CTimer* m_pLoopTimer;
};
#endif	//__VIDONME_MEDIACENTER__
