#ifndef __LIBPLAYERCORE_IPLAYER_H__
#define __LIBPLAYERCORE_IPLAYER_H__

#include <map>
#include <string>


//==========================================================================================================================//
// Indexs
// A. enums 
// B. callback define
// C. data struct
// D. class CCorePlayer



//==========================================================================================================================//
// A. enums

#define MAINMOVIE_PLAYLIST 0xFFFFF

#define MODE_NORMAL		     0
#define MODE_STRETCH_4x3   2
#define MODE_WIDE_ZOOM     3
#define MODE_STRETCH_16x9  4
#define MODE_ORIGINAL      5

enum
{
    PLAYING_MODE_NONE,
    PLAYING_MODE_PLAYERCORE,
    PLAYING_MODE_SYSTEM,
};

enum
{
    RENDER_DATIO_SOURCE,
    RENDER_DATIO_SCREEN,
};

enum
{
    PLAYER_SOURCE_TYPE_FILE,
    PLAYER_SOURCE_TYPE_FOLDER,
    PLAYER_SOURCE_TYPE_DISC,
};

enum
{
    PLAYER_TYPE_VIDEO,
    PLAYER_TYPE_AUDIO,
};

#if defined(__TARGET_ANDROID_ALLWINNER__)

//2D

#define ALLWINNER_DISPLAYMODE_2D_LEFT 			             0x1             
#define ALLWINNER_DISPLAYMODE_2D_RIGHT			            (0x1<<1)
#define ALLWINNER_DISPLAYMODE_2D_FULL   		            (0x1<<2)

//3D

#define ALLWINNER_DISPLAYMODE_3D_LEFT_RIGHT		          (0x1<<3)
#define ALLWINNER_DISPLAYMODE_3D_TOP_BOTTOM             (0x1<<4)
#define ALLWINNER_DISPLAYMODE_3D_DUAL_STREAM            (0x1<<5)
#define ALLWINNER_DISPLAYMODE_3D_MODE_LINE_INTERLEAVE   (0x1<<6)
#define ALLWINNER_DISPLAYMODE_3D_MODE_COLUME_INTERLEAVE (0x1<<7)

//3D ANAGLAGH
#define ALLWINNER_DISPLAYMODE_3D_MODE_SIDE_BY_SIDE      (0x1<<8)
#define ALLWINNER_DISPLAYMODE_3D_MODE_TOP_TO_BOTTOM     (0x1<<9)

#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_RED_BLUE      (0x1<<0) 
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_RED_GREEN     (0x1<<1)
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_RED_CYAN      (0x1<<2)
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_COLOR         (0x1<<3)
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_HALF_COLOR    (0x1<<4)
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_OPTIMIZED     (0x1<<5)
#define ALLWINNER_DISPLAYMODE_3D_ANAGLAGH_YELLOW_BLUE   (0x1<<6)

#endif //__TARGET_ANDROID_ALLWINNER__


enum ExSubtitleAlign
{
    ExSUBTITLE_ALIGN_MANUAL = 0,
    ExSUBTITLE_ALIGN_BOTTOM_INSIDE,
    ExSUBTITLE_ALIGN_BOTTOM_OUTSIDE,
    ExSUBTITLE_ALIGN_TOP_INSIDE,
    ExSUBTITLE_ALIGN_TOP_OUTSIDE
};

enum MediaStreamType
{
    MediaStream_NONE,    // if unknown
    MediaStream_AUDIO,   // audio stream
    MediaStream_VIDEO,   // video stream
    MediaStream_DATA,    // data stream
    MediaStream_SUBTITLE,// subtitle stream
    MediaStream_TELETEXT // Teletext data stream
};








//==========================================================================================================================//
// B. callback define

typedef void (*PFN_OnPlayBackEnded)(void* owner);
typedef void (*PFN_OnPlayBackStarted)(int type, void* owner);
typedef void (*PFN_OnPlayBackPaused)(void* owner);
typedef void (*PFN_OnPlayBackResumed)(void* owner);
typedef void (*PFN_OnPlayBackStopped)(void* owner);
typedef void (*PFN_OnPlayBackFailed)(int code, void* owner);
typedef void (*PFN_OnQueueNextItem)(void* owner);
typedef void (*PFN_OnPlayBackSeek)(int iTime, int seekOffset, void* owner);
typedef void (*PFN_OnPlayBackSeekChapter)(int iChapter, void* owner);
typedef void (*PFN_OnPlayBackSpeedChanged)(int iSpeed, void* owner);
typedef void (*PFN_OnCaptureResult)(char* sfile, bool bok, bool bbuffer, int nwidth, int height, void* owner);
typedef void (*PFN_OnNewVideoFrame)(void* owner);
typedef void (*PFN_OnPlaylistOpened)(void* owner);
typedef void (*PFN_OnVideoCodecOpened)(void* owner, int ndevice);

typedef void (*PFN_ONAudioCallbackInitialize)(void* owner, int iChannels, int iSamplesPerSec, int iBitsPerSample);
typedef void (*PFN_ONAudioCallbackAudioData)(void* owner, const float* pAudioData, int iAudioDataLength);

struct ICorePlayerCallback
{
    ICorePlayerCallback()
    {
        cbowner = NULL;
        pfn_OnPlayBackEnded = NULL;
        pfn_OnPlayBackStarted = NULL;
        pfn_OnPlayBackPaused = NULL;
        pfn_OnPlayBackResumed = NULL;
        pfn_OnPlayBackStopped = NULL;
        pfn_OnPlayBackFailed = NULL;
        pfn_OnQueueNextItem = NULL;
        pfn_OnPlayBackSeek = NULL;
        pfn_OnPlayBackSeekChapter = NULL;
        pfn_OnPlayBackSpeedChanged = NULL;
        pfn_OnCaptureResult = NULL;
#if defined(TARGET_DARWIN_IOS)
        pfn_OnCacheState = NULL ;
#endif
        pfn_OnNewVideoFrame = NULL;
        pfn_OnPlaylistOpened = NULL;
        pfn_OnVideoCodecOpened = NULL;
    }


    PFN_OnPlayBackEnded					pfn_OnPlayBackEnded;
    PFN_OnPlayBackStarted				pfn_OnPlayBackStarted;
    PFN_OnPlayBackPaused				pfn_OnPlayBackPaused;
    PFN_OnPlayBackResumed				pfn_OnPlayBackResumed;
    PFN_OnPlayBackStopped				pfn_OnPlayBackStopped;
    PFN_OnPlayBackFailed				pfn_OnPlayBackFailed;
    PFN_OnQueueNextItem					pfn_OnQueueNextItem;
    PFN_OnPlayBackSeek					pfn_OnPlayBackSeek;
    PFN_OnPlayBackSeekChapter		pfn_OnPlayBackSeekChapter;
    PFN_OnPlayBackSpeedChanged	pfn_OnPlayBackSpeedChanged;
    PFN_OnCaptureResult         pfn_OnCaptureResult;
#if defined(TARGET_DARWIN_IOS)
    PFN_OnCacheState            pfn_OnCacheState;
#endif
    PFN_OnNewVideoFrame			    pfn_OnNewVideoFrame;
    PFN_OnPlaylistOpened		    pfn_OnPlaylistOpened;
    PFN_OnVideoCodecOpened		  pfn_OnVideoCodecOpened;

    void*                       cbowner;
};

struct ICorePlayerAudioCallback
{

    ICorePlayerAudioCallback()
    {
        pfn_initialize = NULL;
        pfn_audiodata = NULL;
        cbowner = NULL;
    }

    PFN_ONAudioCallbackInitialize pfn_initialize;
    PFN_ONAudioCallbackAudioData  pfn_audiodata;

    void*                         cbowner;
};


//==========================================================================================================================//
// C. data struct

// __OUTER_URL_SUPPORT__
// read, stream interface. and this data-struct is be used as baseclass

struct CCoreStreamURL
{
    const char* name;

    //read File interface

    int (*url_open)(CCoreStreamURL* h, const char* url, int flags);
    int (*url_read)(CCoreStreamURL* h, unsigned char* buf, int size);
#if defined(TARGET_ANDROID_GENERIC)
    int64_t(*url_seek)(CCoreStreamURL* h, int64_t pos, int whence);
    int64_t(*url_timeseek)(CCoreStreamURL* h, int ms);
#else
    __int64(*url_seek)(CCoreStreamURL* h, __int64 pos, int whence);
    __int64(*url_timeseek)(CCoreStreamURL* h, int ms);
#endif
    int (*url_close)(CCoreStreamURL* h);

    //stream interface

    bool (*url_is_eof)(CCoreStreamURL* h);
    int (*url_get_totaltime_ms)(CCoreStreamURL* h);
    int (*url_get_time_ms)(CCoreStreamURL* h);

    int (*url_get_chapter)(CCoreStreamURL* h);
    int (*url_get_chapter_count)(CCoreStreamURL* h);
    bool(*url_seek_chapter)(CCoreStreamURL* h, int ch);

#if defined(TARGET_ANDROID_GENERIC)
    int64_t(*url_get_length)(CCoreStreamURL* h);
#else
    __int64(*url_get_length)(CCoreStreamURL* h);
#endif
};

// player options

struct IPlayerOptions
{
    IPlayerOptions()
    {
        starttime = 0LL;
        startpercent = 0LL;
        //		identify = false;
        fullscreen = false;
        video_only = false;
        url = NULL;
        playlist = -1;
        spropertys = NULL;
        strcpy(subtitlepath, "");

        restoreAudioId    = -1;
        restoreSubtitleId = -1;
    }

    double  starttime; /* start time in seconds */
    double  startpercent; /* start time in percent */
    //	bool    identify;  /* identify mode, used for checking format and length of a file */
    //	CStdString state;  /* potential playerstate to restore to */
    bool    fullscreen; /* player is allowed to switch to fullscreen */
    bool    video_only; /* player is not allowed to play audio streams, video streams only */
    char    username[255];
    char    password[255];

    CCoreStreamURL* url;

    int		  playlist;							//specify dvd or bluray playlist
    bool    b3dmode;                            //using 3D mode to play it if support it (player need using libplayercore provide function to check it)

    int     restoreAudioId;
    int     restoreSubtitleId;

    char    subtitlepath[255*2];
    const char* spropertys;

#if defined(__TARGET_ANDROID_ALLWINNER__)
    int displayMode;
#endif

};

// img items

struct WaittingImgItems
{
    int  index;
#if !defined(TARGET_DARWIN_IOS) && !defined(TARGET_ANDROID_GENERIC)
    char strimgpath[MAX_PATH];
#else
    char strimgpath[255];
#endif
    int  timeout;
    WaittingImgItems* next;
};


typedef struct
{
    //StreamType   type;
    int		     type;
    int          type_index;
    std::string  filename;
    std::string  filename2;  // for vobsub subtitles, 2 files are necessary (idx/sub)
    std::string  language;
    std::string  name;
    //CDemuxStream::EFlags flags;
    int		     flags;
    int          source;
    int          id;
    std::string  codec;
    int          channels;

    //add detailed info.write by supy
    int			 width;
    int			 height;
    double		 framerate;
    int			 samplerate;
    int			 bitrate;

} MediaStreamInfo;



// struct for OpenFile

//file :
//x: /xxxx/xx.mkv
//x: /xxxx/xx.iso          --  auto check Bluray / DVD / ?
//
//folder:
//x: /xxxx /               --  auto check folder
//x: /xxxx / BDMV /        --  BLURAY / AVCHD
//x: /xxxx / BDAV /        --  BDAV
//x: /xxxx / VIDEO_TS /    --  DVD
//
//disc:
//x:/                      --  auto check Bluray / DVD / ?

//default is playing menu, if want to play specify playlist, need specify playlist in IPlayerOptions
//if want playing mainmovie, set playlist = INT_MAX


struct SourceFileItem
{
    //source information 

    const char*     pszFile;
    const char*     pszMimeType;
    int             sourceType;

    //player information 

    int             playerType;


    //options 

    IPlayerOptions* pOptions;

    // extra information, mainly from fileItem class

    int startOffset;    //for some special audio case, like ape, flac which using cue to describe the file
    int endOffset;
    int startPartNumber;
    

};



//==========================================================================================================================//
// D. class CCorePlayer


class IPlayerCallbackBridge;
class CCriticalSection;
struct C3DContext;

class CCorePlayer
{
public:

    CCorePlayer(ICorePlayerCallback* callback);
    virtual ~CCorePlayer();

    virtual bool InitDisplayDevice(void* outerwindow, int width, int height);

    virtual void RegisterAudioCallback(ICorePlayerAudioCallback* audiocallback);
    virtual void UnRegisterAudioCallback();

    virtual bool OpenFile(SourceFileItem* sourceItem);
    virtual bool CloseFile();

    virtual bool CanOpenFile(const char* pszFile, const char* pszMimeType, IPlayerOptions* pOptions);

    virtual bool QueueNextFile(SourceFileItem* sourceItem);
    virtual void OnNothingToQueueNotify();

    virtual void Pause();
    virtual void Resume();
    virtual void Prev();
    virtual void Next();

#if defined (TARGET_DARWIN_IOS)
    virtual void Hangup(bool bhangup);
#endif
    virtual bool IsPlaying();
    virtual bool IsPaused();

    virtual bool HasVideo();
    virtual bool HasAudio();

    virtual double GetDuration();
    virtual double GetCurrentTime();

    virtual void SeektoTime(double time);
    virtual bool CaptureScreen(char* filename, int width, int height);

    virtual void ChangeDisplayMode(int mode);
    virtual void SetAudioVolume(float fVolume, bool isPercentage = true);
    virtual int	 GetAudioVolume();

    virtual void SetAudioLanguage(int nAudioIndex, int nAudioStreamID);
    virtual int  GetAudioLanguage();
    virtual void SetSubPicture(int nSubIndex,   int nSubStreamID);
    virtual int  GetSubPicture();
    virtual void CloseSubPicture();
    virtual void SetSubtitleVisible(bool bVisible);

    virtual void SetFullScreen(bool bShow);
    virtual bool GetFullScreen();
    virtual void ReSize(int cx, int cy);
#if defined(TARGET_DARWIN_OSX)
    virtual void ReSizeOSX(int cx, int cy);
    virtual void FadeToBlack(void);
    virtual void FadeFromBlack(void);
    virtual void ReflushRenderer(void);
#endif
    virtual void SetDisplayRatio(bool bRatio);
    virtual bool SetDisplayRatio(int nstyle);

    virtual void Move();

    virtual void RenderVideoStream();
    virtual void SetMute(bool bmute);

    virtual int  GetSubtitleCount();
    virtual int  GetSubtitleCurrentStream();
    virtual void GetSubtitleName(int iStream, char* strStreamName);
    virtual bool SelectSubtitle(int iStream);
    virtual bool SetSubtitleVisible(bool bvisable, char* strsubfile);
    virtual bool GetSubtitleVisible();

#if defined (TARGET_WINDOWS)
    virtual bool SetExternalSubtitlePath(const char* szsubpath);
    virtual bool SetExternalSubtitlePosition(int mode, int x = 0, int y = 0);
#endif

    virtual int  GetAudioStreamCount();
    virtual int  GetAudioCurrentStream();
    virtual void GetAudioStreamName(int iStream, char* strStreamName);
    virtual void GetAudioStreamLanguage(int iStream, char* strStreamLanguage);
    virtual bool SetAudioStream(int iStream);

    virtual void SetPlaySpeed(int iSpeed);
    virtual int GetPlaySpeed();

    virtual int  GetChapterCount();
    virtual int  GetChapter();
    virtual void GetChapterName(char* strChapterName);
    virtual int  SeekChapter(int iChapter);

    virtual void SetDisplayBrightness(float fBrightness);
    virtual float GetDisplayBrightness();
    virtual void SetDisplayContrast(float fContrast);
    virtual float GetDisplayContrast();
    //__LIBPLAYCORE_3D_SUPPORT__
    virtual bool Get3DGlobalEn();
    virtual void Set3DSourceType(int type);
    virtual int  Get3DSourceType();
    virtual int  GetSupportedPlayModes();
    virtual bool Set3DPlayMode(int mode);
    virtual int  Get3DPlayMode();
    //virtual void Get3DMode(C3DContext* ctx);
    //++ xubin added
    // Only when you have a reason, else don't use virtual function
    void SetZoomAmount(float Value);
    void SetPixelRatio(float Value);
    void SetVerticalShift(float Value);
    //-- xubin added

    virtual void SetAVDelay(float fdelay);
    virtual float GetAVDelay();

    virtual void SetSubTitleDelay(float fdelay);
    virtual float GetSubTitleDelay();

    virtual void SetVolumeAmplification(float va);
    virtual float GetVolumeAmplification();

    virtual float GetVideoAspectRatio();

    enum DEINTERLACEMODE
    {
        DEINTERLACEMODE_OFF = 0,
        DEINTERLACEMODE_AUTO = 1,
        DEINTERLACEMODE_FORCE = 2
    };
    virtual void SetDeinterlace(int nmode);

    virtual void EnableThreadRenderVideo(bool benable);

    virtual std::vector<MediaStreamInfo> GetStreamInfo(int type , bool bcurrent = false);
    virtual MediaStreamInfo* GetCurrentStreamInfo( int type);
    virtual void GetAllStreamInfo();

    //TODO, more data for action, please check CDVDPlayer::OnAction()
    //current only add action for bdj-bluray
    enum
    {
        PLAYERCORE_ACTION_NONE,
        PLAYERCORE_ACTION_MOVE_LEFT,
        PLAYERCORE_ACTION_MOVE_RIGHT,
        PLAYERCORE_ACTION_MOVE_UP,
        PLAYERCORE_ACTION_MOVE_DOWN,
        PLAYERCORE_ACTION_ENTER,
        PLAYERCORE_ACTION_MOUSE_MOVE,
        PLAYERCORE_ACTION_MOUSE_CLICK,
        PLAYERCORE_ACTION_NEXT_ITEM,
        PLAYERCORE_ACTION_PREV_ITEM,
        PLAYERCORE_ACTION_CHANNEL_UP,
        PLAYERCORE_ACTION_CHANNEL_DOWN,
        PLAYERCORE_ACTION_SHOWVIDEOMENU,
        //TODO, more action
    };

    virtual bool OnAction(int id, float x = 0, float y = 0);
    virtual bool ShowVideoMenu(bool bPop = false);
    virtual void AddWaittingImgInfo(WaittingImgItems* imgitem);

#if defined(__TARGET_ANDROID_ALLWINNER__)
    //for source type
    enum
    {
        //* for 2D pictures.
        PLAYCORE_CEDARV_3D_MODE_NONE                             = 0,

        //* for double stream video like MVC and MJPEG.
        PLAYCORE_CEDARV_3D_MODE_DOUBLE_STREAM,

        //* for single stream video.
        PLAYCORE_CEDARV_3D_MODE_SIDE_BY_SIDE,
        PLAYCORE_CEDARV_3D_MODE_TOP_TO_BOTTOM,
        PLAYCORE_CEDARV_3D_MODE_LINE_INTERLEAVE,
        PLAYCORE_CEDARV_3D_MODE_COLUME_INTERLEAVE
    };

    //for output type
    enum
    {
        CEDARX_DISPLAY_3D_MODE_2D,
        CEDARX_DISPLAY_3D_MODE_3D,
        CEDARX_DISPLAY_3D_MODE_HALF_PICTURE,
        CEDARX_DISPLAY_3D_MODE_ANAGLAGH,
    };

    virtual bool SetInputDimensionType(int type);
    virtual bool SetOutputDimensionType(int type);
    virtual int  SetAllWinnerDisplayMode (int mode);
#endif //__TARGET_ANDROID_ALLWINNER__

#if defined (TARGET_WINDOWS) || defined(__TARGET_ANDROID_ALLWINNER__)
    virtual bool IsInMenu();
#endif

#if defined (TARGET_WINDOWS) || defined(TARGET_DARWIN_OSX)
    virtual void ShowHint(int x, int y, const char* smsg,int nfontszie, unsigned int color, unsigned int bordercolor);
#endif

#if defined (TARGET_WINDOWS)
    enum
    {
        DEVICE_INTEL,
        DEVICE_CUDA,
    };
    virtual bool GetVideoAccelerate(int nstype);

    virtual int  GetRotationDegree();
    virtual void SetRotationDegree(int ndegree);

    virtual int  GetMirrorStyle();
    virtual void SetGetMirrorStyle(int nstyle);
#endif

    virtual void GetAudioInfo(char * strAudioInfo,int &nlength);
    virtual void GetVideoInfo(char * strVideoInfo,int &nlength);
    virtual void GetGeneralInfo( char * strGeneralInfo,int &nlength);
    virtual void Seek(bool bPlus, bool bLargeStep);
    virtual void GetPlayerState(char * strState,int &nlength);
    virtual bool SetPlayerState(char* strState);
    virtual const char * GetCurrentSubtitle();
    virtual bool IsCaching();
    virtual int GetCacheLevel();

    virtual int AddSubtitle(const char * strSubPath);

    virtual int GetPlayingPlaylist();                                       //Get current playing title
    virtual const char* GetSubtitlePath(int iStream);

public:

    bool										      m_bManualClose;

protected:

    void ParsePropertys();

protected:

    int											      m_playing_mode;
    IPlayerCallbackBridge*	      m_callbackBridge;
    bool										      m_bPlaying;
    CCriticalSection*				      m_ObjSection;
    int                                         m_iPlaySpeed;

    std::vector<MediaStreamInfo>                m_videostreams;
    std::vector<MediaStreamInfo>                m_audiostreams;
    std::vector<MediaStreamInfo>                m_subtitlestreams;

    std::string				                    m_cursubtitlecontent;
    std::string				                    m_propertys;
    std::map<std::string, std::string>          m_mapproperty;

    int                                         m_PlayerType;

    // static function

public:

    static void SetDefaultAudioLanguage(const char* lang);
    static void SetDefaultSutitleLanguage(const char* lang);
    static void SetSubtitleSearchPath(const char* pszPath);


};

#endif //__LIBPLAYERCORE_IPLAYER_H__
