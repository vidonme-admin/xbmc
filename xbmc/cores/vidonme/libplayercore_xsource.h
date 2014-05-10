#ifndef __LIBPLAYECORE_XSOURCE_H__
#define __LIBPLAYECORE_XSOURCE_H__

#include <map>

enum XSOURCE_CONTENT
{
    XSOURCE_CONTENT_UNKNOWN				= -1,
    XSOURCE_CONTENT_DATA,

    XSOURCE_CONTENT_BLURAY_BDMV		= 10,
    XSOURCE_CONTENT_BLURAY_BDAV,

    XSOURCE_CONTENT_DVD_VIDEO			= 20,
    XSOURCE_CONTENT_DVD_AVCHD,
    XSOURCE_CONTENT_DVD_AVCREC,
    XSOURCE_CONTENT_DVD_VR,
    XSOURCE_CONTENT_DVD_AUDIO,

    XSOURCE_CONTENT_HD_DVD				= 30,

    XSOURCE_CONTENT_MEDIA_FILE		= 40,
};

enum XSOURCE_MEDIUM
{
    XSOURCE_MEDIUM_FILE,
    XSOURCE_MEDIUM_FOLDER,
    XSOURCE_MEDIUM_DISC,
};

typedef int (*PFN_PLAYING_NOTIFY_CALLBACK)(int notify);

//////////////////////////////////////////////////////////////////////////

namespace XSource
{


class CPLStream
{
public:

    CPLStream();
    ~CPLStream();

};

typedef std::vector<CPLStream*> PLStreamPtrs;


class CPlaylist
{
public:

    CPlaylist();
    ~CPlaylist();

public:

    int m_durationMs;				//duration in millsecond
    int	m_Playlist;					//playlist value
    int	m_index;					//index in all playlist
    int m_angles;					//angle count of this playlist

    bool m_b3D;						//3D playlist
    bool m_bMainMovie;			    //Mainmovie
    bool m_bMenu;					//it's menu playlist

    int m_chapterCount;			    //

    //PLStreamPtrs m_streams;

};

typedef std::vector<CPlaylist*> PlaylistPtrs;

//meta information map

typedef std::map<std::string, std::string> MetaMap;

}; //namespace XSource

#if defined(__LIBPLAYERCORE_XSOURCE__)

class DllLibbluray;

//////////////////////////////////////////////////////////////////////////

namespace libudf
{
class CUDF25;
typedef boost::shared_ptr<CUDF25> CUDF25Ptr;
};

//////////////////////////////////////////////////////////////////////////

class XSourceItem
{
public:

    XSourceItem();
    virtual ~XSourceItem();

    //basic play path
    void CreateBasicPlayPath();

    //create playing path
    CStdString GetPlayPath(int& playlist, bool bEnableXdiscPlaylistSupport);

    void CreateCacheFolder(CStdString& strCacheFolder);
    void CreateMovieThumbnailPath(std::string& strPath);
    void SetMovieMeta(CStdString& movieName, bool bGetThumbnail);
    const char* GetMovieThumbnailPath();

    void GetSourcePlaylists();
    void GetSourceMetaInfo();
    void GetSourceThumbnail(int playlist);

    int  GetMainmoviePlaylist();
    bool Is3DPlaylist( int playlist );

    // cache and bluray object 

    bool IsNeedEnableBlurayCache( CStdString& strRootPath, CStdString& strStandardPath );
    bool IsEnableCache();
    void EnableCache( CStdString& strRootPath, int cacheType );
    
    void GetCacheFilePath( CStdString& strPath );
    void CopyFolderToCache( CStdString& strSrcRoot, CStdString& strDstRoot, const char* subFolder, bool bRecursive );
    bool CopyCacheFile( CStdString& strSrc, CStdString& strDest );

    // external subtitle function

    void ScanExternalSubtitle(std::vector<CStdString>& externalSubs);

protected:

    void ParserSource();
    void ParserSourceByNativeLibrary();

public:

    char							m_xdiscDrive;

    XSOURCE_CONTENT					m_content;
    XSOURCE_MEDIUM					m_medium;

    libudf::CUDF25Ptr				m_udf25Root;
    CStdString						m_strStandardPath;		        //standard path
    CStdString						m_strBasicPlayPath;		        //basic playing path, playing path can create from this path

    CStdString						m_strMovieName;
    CStdString						m_strMovieThumbnail;

    XSource::PlaylistPtrs		    m_playlists;					//playlist information
    XSource::MetaMap				m_metaMap;						//meat map information


    CStdString						m_strCacheFolder;			    //cache folder for save different file
    bool							m_bCreateCacheFolderFailed;     //create cache failed flag, if failed once will not try it again
    bool                            m_bEnableCache;                 //enable cache flag
    
    enum
    {
        BLURAY_CACHE_TYPE_BASIC     = 0x01,                         //clip, playlist information
        BLURAY_CACHE_TYPE_ADVANCE   = 0x02,                         //bdj information
    };


    typedef std::map<CStdString, CStdString> CacheMAP;
	
private:

    CacheMAP                        m_mapCache;                     //file,folder cache map
    CCriticalSection                m_fileacheSection;
	
protected:

    bool							m_bParsered;					//this item has been parsered
    CCriticalSection				m_section;						//

};

typedef boost::shared_ptr<XSourceItem> XSourceItemPtr;


//////////////////////////////////////////////////////////////////////////
struct AppSettings;

class XSourceManager
{
public:

    XSourceManager();
    virtual ~XSourceManager();

    //api

    bool Init(AppSettings* pAppSettings);
    XSourceItemPtr AddPlaySource(CStdString& strPath, int sourceType);
    bool CreatePlayPath(XSourceItemPtr& item, CStdString& strPath, int& playlist, bool bForGetThumb = false);
    void RemovePlaySource(CStdString& strPath, int sourceType);
    void RemovePlaySourceAll();

    // get playlist information , meta info, get thumbnail

    XSource::PlaylistPtrs& GetSourcePlaylists(CStdString& strPath);
    void GetSourceMetaInfo(CStdString& strPath);
    void GetSourceThumbnail(CStdString& strPath, int playlist);
    const char* GetSourceImage(CStdString& strPath);

    //source item function

    XSourceItemPtr QueryXSourceItem(CStdString& strPath, bool bStandardPath = false);
    XSourceItemPtr QueryXSourceItemByUrl(CStdString& strUrl);
    bool		   QueryXSourceItemByXDiscDrive(char cDrive, XSourceItemPtr& item);

    //tool function

    libudf::CUDF25Ptr   GetUDFRoot(CStdString& strUDFRootKey);

    // temporary cache folder for different source

    void CreateXSourceItemCacheFolder(CStdString& strStandardPath, CStdString& strCacheFolder);

    //version check ( trial version )

    bool IsTrialVersionExpired();

    //open error report

    void SendPlayingNotify(int notify);

    // BD, DVD region

    int GetBDRegion();
    void SetBDRegion( int BDRegion );

    int GetDVDRegion();
    void SetDVDRegion( int DVDRegion );

    DllLibbluray* GetBDjvmDll();

protected:

    bool IsImageFile(CStdString& strExtension);
    bool IsBDDVDFolderFile(CStdString& strPath, CStdString& strFolderPath);
    void GetFolderContent(CStdString& strPath, XSourceItemPtr& item);
    void GetFileContent(CStdString& strPath, XSourceItemPtr& item);
    void GetDiscContent(CStdString& strPath, XSourceItemPtr& item);

protected:

    //the key is user passing path param
    //the value is standard source playing path
    //the value rule:
    //file:			xxxx/xxxx.xxx
    //folder:
    //	DVD:		xxxx/VIDEO_TS/
    //	Bluray:		xxxx/BDMV/
    //	BDAV:		xxxx/BDAV/
    //Disc:
    //	DVD:		X:/
    //	Bluray:		X:/

    typedef std::map<CStdString, CStdString> SourcePathMap;
    SourcePathMap m_sourcePathMap;

    typedef std::map<CStdString, XSourceItemPtr> SourceItemMap;
    SourceItemMap m_sourceItemMap;

    bool			 m_bTrialVersionExpired;
    int				 m_BDRegion;

    CCriticalSection	m_section;

    //callback for
    PFN_PLAYING_NOTIFY_CALLBACK m_pfnPlayingNotify;

    //bluray support dll
    DllLibbluray* m_bdjvmdll;

public:

    //brief The only way through which the global instance of the XSourceManager should be accessed.
    //return the global instance.
    static XSourceManager& GetInstance();

};

#endif //__LIBPLAYERCORE_XSOURCE__

#endif //__LIBPLAYECORE_XSOURCE_H__
