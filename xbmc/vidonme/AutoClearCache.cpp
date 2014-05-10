#include "AutoClearCache.h"
#include "TextureCache.h"
#include "utils/log.h"
#include "filesystem/SpecialProtocol.h"
#if defined(_WIN32)
#include "Win32/WIN32Util.h"
#endif
#include "settings/Settings.h"
#ifdef _LINUX
#include <sys/vfs.h>
#endif

#undef MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#undef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))

CAutoClearCache::CAutoClearCache(int nDiskSpaceMin,int nCacheCountMax,unsigned long nCacheCycleTime)
{
	m_nDiskSpaceMin = nDiskSpaceMin;
	m_nCacheCountMax = nCacheCountMax;
	m_nCacheCycleTime = nCacheCycleTime;
	m_pLoopTimer = new CTimer(this);
}

CAutoClearCache::~CAutoClearCache(void)
{
	StopLoop();
	delete m_pLoopTimer;
	m_pLoopTimer = NULL;
}

void CAutoClearCache::OnTimeout()
{
	ClearCacheOneTimes();
}

void CAutoClearCache::StartLoop()
{
	if(!m_pLoopTimer->IsRunning())
	{
		//30 min loop
		m_pLoopTimer->Start(30*60*1000,true);
	}
	ClearCacheOneTimes();
}

void CAutoClearCache::StopLoop()
{
	if(m_pLoopTimer->IsRunning())
	{
		m_pLoopTimer->Stop(true);
	}
}

int CAutoClearCache::GetCacheDiskFree()
{
#if defined(TARGET_DARWIN)
	return -1;
#elif defined(TARGET_ANDROID)
	char sPath[512]= "/mnt/sdcard/Android/data/org.vidonme.vidonme/";
	strcpy(sPath,CSpecialProtocol::TranslatePath("special://home/addons").c_str());
	struct statfs buf;
	int nRet = statfs(sPath,&buf);
	return (buf.f_bfree*buf.f_bsize)/(1024*1024);
#elif defined(TARGET_FREEBSD)

#elif defined(_LINUX)
	char sPath[512]= "/mnt/sdcard/Android/data/org.vidonme.vidonme/";
	strcpy(sPath,CSpecialProtocol::TranslatePath("special://home/addons").c_str());
	struct statfs buf;
	int nRet = statfs(sPath,&buf);
	return (buf.f_bfree*buf.f_bsize)/(1024*1024);
#elif _WIN32
	char ch[3];
	strncpy(ch,CSpecialProtocol::TranslatePath("special://home/addons").c_str(),2);
	ch[2]='\0';
	ULARGE_INTEGER FreeBytes, TotalBytes;
	GetDiskFreeSpaceEx(ch,&FreeBytes,&TotalBytes,NULL);
	return FreeBytes.QuadPart/1024;
#endif
	assert(0);
	return -1;
}

bool CAutoClearCache::DeleteCacheFiles(const int nCount)
{
	structTextureDetails textureDetails;
	for(int i=0;i<nCount;i++)
	{
		if(!m_TextureDatabaseEx.GetAllTextAt(i,textureDetails))
			return false;
		CTextureCache::Get().ClearCachedImage(textureDetails.strUrl);
	}
}

bool CAutoClearCache::ClearCacheOneTimes()
{
	int nSize = 0;
	if(!m_TextureDatabaseEx.IsOpen())
	{
		if(!m_TextureDatabaseEx.Open())
			return false;
	}
	if(!m_TextureDatabaseEx.QueryAllTexture())
		return false;
	nSize = m_TextureDatabaseEx.GetAllTextureSize();
	
	int nFreeDiskforMB = GetCacheDiskFree();
	if(nFreeDiskforMB<0)
		return false;
	while(nFreeDiskforMB>-1 && nFreeDiskforMB<m_nDiskSpaceMin && nSize > 20)
	{
		int nDelCount = MAX(nSize/20,5);
		DeleteCacheFiles(nDelCount);
		nSize-=nDelCount;
		nFreeDiskforMB = GetCacheDiskFree();
	}
	/*
	if(!m_TextureDatabaseEx.QueryAllTexture())
		return false;
	nSize = m_TextureDatabaseEx.GetAllTextureSize()
	*/
	if(nSize>m_nCacheCountMax)
	{
		DeleteCacheFiles(nSize-m_nCacheCountMax);
		if(!m_TextureDatabaseEx.QueryAllTexture())
			return false;
		nSize = m_TextureDatabaseEx.GetAllTextureSize();
	}

	if(nSize>2)
	{
		structTextureDetails latest,textDetails;
		time_t t_latest,t_time;
		m_TextureDatabaseEx.GetAllTextAt(nSize-1,latest);
		long diffTime = 0;
		
		t_latest = string2time(latest.strLastUseTime.c_str(),"%d-%d-%d %d:%d:%d");
		if(t_latest == -1)
		{
			assert(0);
			perror("time string format error \n");
		}
		for(int i=0;i<nSize-1;i++)
		{
			if(!m_TextureDatabaseEx.GetAllTextAt(i,textDetails))
				return false;
			t_time = string2time(textDetails.strLastUseTime.c_str(),"%d-%d-%d %d:%d:%d");
			if(t_time == -1)
			{
				assert(0);
				perror("time string format error \n");
			}
			if(t_latest - t_time > m_nCacheCycleTime)
			{
				CTextureCache::Get().ClearCachedImage(textDetails.strUrl);
			}
			else
			{
				break;
			}
		}
	}
	return true;
}

time_t CAutoClearCache::string2time(const char * str,const char * formatStr)  
{  
  struct tm tm1;  
  int year,mon,mday,hour,min,sec;  
  if( -1 == sscanf(str,formatStr,&year,&mon,&mday,&hour,&min,&sec)) return -1;  
  tm1.tm_year=year-1900;  
  tm1.tm_mon=mon-1;  
  tm1.tm_mday=mday;  
  tm1.tm_hour=hour;  
  tm1.tm_min=min;  
  tm1.tm_sec=sec;  
  return mktime(&tm1);  
}  
