
#if defined(__HAS_VIDONME_PLAYER__)

#include "cores/IPlayer.h"
#include "threads/Thread.h"
#include "windowing/WinSystem.h"
#include "DllPlayercore.h"
#include "FileItem.h"
#include "settings/VideoSettings.h"
#include "cores/playercorefactory/PlayerCoreFactory.h"

class CVDMPlayer;
class CAfterStartedProcess : public CThread
{
public:
	CAfterStartedProcess(CVDMPlayer * pplayer);
	virtual ~CAfterStartedProcess(){}
protected:
	virtual void Process();
protected:
	CVDMPlayer* m_pPlayer;
};

enum
{
  ACTION_TYPE_NONE,
  ACTION_TYPE_PAUSE,					
  ACTION_TYPE_STOP,
  ACTION_TYPE_OPENFAILED,
  ACTION_TYPE_RUNING,
  ACTION_TYPE_OVER,

  RENDER_TYPE_BEGIN,
  RENDER_TYPE_END,
};

class CCorePlayer;
struct ICorePlayerCallback;

class CVDMPlayer : public IPlayer, public CThread , public IWinEventCallback
{
public:

  enum WARP_CURSOR { WARP_NONE = 0, WARP_TOP_LEFT, WARP_TOP_RIGHT, WARP_BOTTOM_RIGHT, WARP_BOTTOM_LEFT, WARP_CENTER };

  CVDMPlayer(IPlayerCallback& callback, EPLAYERCORES specifyPlayer);
  virtual ~CVDMPlayer();
  virtual bool Initialize(TiXmlElement* pConfig);
  virtual void RegisterAudioCallback(IAudioCallback* pCallback);
  virtual void UnRegisterAudioCallback();
  virtual bool OpenFile(const CFileItem& file, const CPlayerOptions &options);
  virtual bool QueueNextFile(const CFileItem& file);
  virtual void OnNothingToQueueNotify();
  virtual bool CloseFile();
  virtual bool CloseFile(bool bmanual = true);
  virtual bool IsPlaying() const;
  virtual void Pause();
  virtual bool IsPaused() const;
  virtual bool HasVideo() const;
  virtual bool HasAudio() const;
  virtual void ToggleOSD() { }; // empty
  virtual void SwitchToNextLanguage();
  virtual void ToggleSubtitles();
  virtual bool CanSeek();
  virtual void Seek(bool bPlus, bool bLargeStep);
  virtual void SeekPercentage(float iPercent);
  virtual float GetPercentage();
  virtual void SetDynamicRangeCompression(long drc);
  virtual void SetContrast(bool bPlus) {}
  virtual void SetBrightness(bool bPlus) {}
  virtual void SetHue(bool bPlus) {}
  virtual void SetSaturation(bool bPlus) {}
  virtual void GetAudioInfo(CStdString& strAudioInfo);
  virtual void GetVideoInfo(CStdString& strVideoInfo);
  virtual void GetGeneralInfo( CStdString& strVideoInfo);
  virtual void Update(bool bPauseDrawing)                       {}
  virtual void SwitchToNextAudioLanguage();
  virtual bool CanRecord() { return false; }
  virtual bool IsRecording() { return false; }
  virtual bool Record(bool bOnOff) { return false; }
  virtual void SetAVDelay(float fValue = 0.0f);
  virtual float GetAVDelay();

  virtual void SetSubTitleDelay(float fValue = 0.0f);
  virtual float GetSubTitleDelay();

  virtual void SeekTime(int64_t iTime);
  virtual int64_t GetTime();
  virtual int64_t GetTotalTime();
  virtual void ToFFRW(int iSpeed);
  virtual void ShowOSD(bool bOnoff);
  virtual void DoAudioWork()                                    {}

  virtual CStdString GetPlayerState();
  virtual bool SetPlayerState(CStdString state);

  virtual bool IsSelfPresent(){return true;};
  virtual void Present();

  virtual int  GetChapterCount();
  virtual int  GetChapter();
  virtual void GetChapterName(CStdString& strChapterName);
  virtual int  SeekChapter(int iChapter);

  virtual int GetSubtitleCount();
  virtual int GetSubtitle();
  virtual void GetSubtitleName(int iStream, CStdString &strStreamName);
  virtual void GetSubtitleLanguage(int iStream, CStdString &strStreamLang);
  virtual void SetSubtitle(int iStream);
  virtual bool GetSubtitleVisible();
  virtual void SetSubtitleVisible(bool bVisible);

  virtual int GetAudioStreamCount();
  virtual int GetAudioStream();
  virtual void GetAudioStreamName(int iStream, CStdString &strStreamName);
  virtual void SetAudioStream(int iStream);
  virtual void GetAudioStreamLanguage(int iStream, CStdString &strLanguage);

  virtual void SetMute(bool bOnOff);
  virtual void SetVolume(float nVolume);
  virtual bool ControlsVolume(){ return true;}

  virtual void SetDisplayBrightness( float fBrightness );
  virtual void SetDisplayRatio( int nstyle );
#if defined(TARGET_WINDOWS)
  virtual BOOL ExecuteAppW32(const char* strPath, const char* strSwitches);
  //static void CALLBACK AppFinished(void* closure, BOOLEAN TimerOrWaitFired);
#elif defined(TARGET_ANDROID)
  virtual BOOL ExecuteAppAndroid(const char* strSwitches,const char* strPath){ return TRUE; }
#elif defined(_LINUX)
  virtual BOOL ExecuteAppLinux(const char* strSwitches){ return TRUE; }
#endif

  virtual void OnReSize(int newWidth, int newHeight, int newLeft, int newTop);
  virtual bool OnAction(const CAction &action);

  virtual CStdString GetAudioCodecName();
  virtual CStdString GetVideoCodecName();
  virtual int GetChannels();

  virtual int GetPictureWidth();
  virtual int GetPictureHeight();

  virtual void GetVideoAspectRatio(float& fAR);
  virtual bool HasMenu();

  virtual bool GetCurrentSubtitle(CStdString& strSubtitle);

  virtual bool IsCaching() const;
  virtual int GetCacheLevel() const;
  virtual int  AddSubtitle(const CStdString& strSubPath);
  bool IsInMenu() const;

  virtual const char* GetSubtitlePath(int iStream);

#if defined(__ANDROID_ALLWINNER__)
  virtual bool SetInputDimensionType (int type);
  virtual bool SetOutputDimensionType (int type);
#endif

  void PrepareStream();
  void OnStarted();
  void SetPlayMode(DIMENSIONMODE mode);

protected:

  virtual void Process();

  void SetVideoPlayingOptions(CStdString& strFile, int& type, IPlayerOptions& options, PlayercoreConfig& config);
  void SetAudioOutputOptions();
  void SetSourceItem(const CFileItem& fileItem, SourceFileItem& sourceItem);

public:

  bool m_bOpenFiled;
  int  m_actiontype;
  int  m_actionrender;

private:

  CEvent m_ready;
  bool m_bIsPlaying;
  bool m_paused;
  int64_t m_playbackStartTime;
  int m_speed;
  int m_totalTime;
  int m_time;

  CStdString m_mimetype;
  CStdString m_strProperties;

  CCorePlayer	*m_pPlayer;
  ICorePlayerCallback *m_playercallback;
  CPlayerOptions m_PlayerOptions;

  int	m_nSpeed;
  CFileItem m_item;
  EPLAYERCORES m_specifyPlayer;

  
//////////////////////////////////////////////////////////////////////////
//static method and member

//get playlist and meta info.

public:

  static void InitializePlayerCore();
  static void UninitializePlayerCore();
  
  static bool AddPlaySource(const std::string& strFilePath);
  static void RemovePlaySource(const std::string& strFilePath);
  static void CleanPlaySource();

  static void LoadExternalFileSystem( PlayercoreConfig* pConfig );

  static void SetSMBPassword(const std::string& strPassword);
  static void SetSMBUserName(const std::string& strUserName);

  struct Playlist
  {
    int nDurationMs;				//duration in millsecond
    int	nPlaylist;					//playlist value
    int	nIndex;						//index in all playlist
    int nAngles;						//angle count of this playlist

    bool b3D;							//3D playlist
    bool bMainMovie;			//Mainmovie
    bool bMenu;						//it's menu playlist

    int nChapterCount;

    std::string strPlayPath;
  };

  typedef boost::shared_ptr<Playlist> PlaylistPtr;
  typedef std::vector<PlaylistPtr> Playlists;

  static int GetPlaySourcePlaylistCount(const std::string& strFilePath);
  static Playlists GetPlaySourcePlaylists(const std::string& strFilePath);
  static PlaylistPtr GetPlaySourcePlaylistByIndex(const std::string& strFilePath, int index);
  static PlaylistPtr GetPlaySourceMainMoviePlaylist(const std::string& strFilePath);

  struct PlaySourceMetaInfo
  {
    std::string strFileFormat; //dvd, bluray
    std::string strVideoCodec; //mpeg1, mpeg2, vc1, h264
    std::string strVideoResolution; //480i, 576i, 480p, 1080i, 720p, 1080p, 576p
    std::string strAudioChannel; //2.0, 5.1, 7.1 ....
    std::string strAudioCodec; //mp1, mp2, lpcm, ac3, dts, truehd, e-ac3, dtshd, dtshd-ma
    std::string strAudioLanguage; //pgs, igs
    std::string strSubtitleCodec;
    std::string strSubtitleLanguage;
    std::string strMenuMode;          // none, hdmv, bdj
  };

  static void GetPlaySourceMetaInfo(const std::string& strFilePath, PlaySourceMetaInfo& metaInfo);

  //play source thumbnail
  static std::string GetPlaySourceThumbnail(const std::string& strFilePath, int playlist = -1, int width = 640, int height = 360, int time = 30000);

private:

  static void GetPlaySourceMetaKeyValue(const std::string& strFilePath, const std::string& strKey, std::string& strValue);
  static std::string GetISOlanguage( const std::string& strLanguage );
  
private:

  static bool             s_bPlayerCoreInited;
  static CCriticalSection s_sourcesMutex;
  static DllPlayercore    s_dllPlayercore;	

};

#endif //__HAS_VIDONME_PLAYER__
