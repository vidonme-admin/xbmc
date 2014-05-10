#if defined(__HAS_VIDONME_PLAYER__)

#include "utils/StringUtils.h"
#include "utils/log.h"
#include "FileItem.h"
#include "Application.h"
#ifdef HAS_VIDEO_PLAYBACK
#include "cores/VideoRenderers/RenderManager.h"
#endif
#include "windowing/WindowingFactory.h"
#include "cores/AudioEngine/AEFactory.h"
#include "settings/Settings.h"
#include "Util.h"

#if defined(TARGET_ANDROID)
#include "utils/URIUtils.h"
#include "android/activity/XBMCApp.h"
#else
#include "Utils/URIUtils.h"
#endif

#include "guilib/GraphicContext.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GUIWindowManager.h"
#include "settings/GUISettings.h"

#include "VDMPlayer.h"

#include "settings/GUISettings.h"
#include "TextureCache.h"

#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "filesystem/FileFactory.h"
#include "filesystem/DirectoryFactory.h"
#include "URL.h"

#include "vidonme/VDMUtils.h"
#include "PasswordManager.h"

#if defined(__ANDROID_ALLWINNER__)
#include "guilib/MatrixGLES.h"
#endif

#include "LangInfo.h"
//////////////////////////////////////////////////////////////////////////


static CCriticalSection s_renderlock;     
static bool             s_bManualClose = false;     //flag for sure it's closed by user, not auto playing finished            
static CStdString       s_strPlayerCoreFilePath;

#define PLAYERCORE_PATH_WIN "special://xbmcbin/system/players/vdmplayer/libplayercore.dll";
#define PLAYERCORE_PATH_A20 "libcoreplayer2.so"
#define PLAYERCORE_PATH_A31 "libcoreplayer.so"

bool                    CVDMPlayer::s_bPlayerCoreInited = false;
DllPlayercore           CVDMPlayer::s_dllPlayercore;
CCriticalSection        CVDMPlayer::s_sourcesMutex;

#if defined(__ANDROID_ALLWINNER__)
extern CMatrixGLES g_matrices;
#endif

using namespace VidOnMe;

#if defined(__ANDROID_ALLWINNER__)

GLint on_guishader_callback (int type, int value)
{
  switch (type)
  {
  case 0: //for enable guishader
    g_Windowing.EnableGUIShader ((ESHADERMETHOD)value);
    return 0;

  case 1: //for disable guishader
    g_Windowing.DisableGUIShader();
    return 0;

  case 2: //for posLoc
    return g_Windowing.GUIShaderGetPos();

  case 3: //for colLoc
    return g_Windowing.GUIShaderGetCol();

  case 4: //for tex0Loc
    return g_Windowing.GUIShaderGetCoord0();
  }

  return 0;
}

int on_guisetting_getint_callback(const char *strSetting)
{
  return g_guiSettings.GetInt(strSetting);
}

#endif //__ANDROID_ALLWINNER__

namespace VidOnMe
{

void OnPlaybackEnd( void* owner )
{
  if (!owner)
    return;

  if (!s_bManualClose)
  {
    CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
    coreplayer->CloseFile(false);

    if (coreplayer->m_bOpenFiled)
      g_windowManager.PreviousWindow();
  }

  g_application.OnPlayBackEnded();
  //CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
  //coreplayer->CloseFile();
}

void OnPlaybackStarted(int type, void* owner )
{
  if (!owner)
    return;

  CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
  coreplayer->m_actionrender = RENDER_TYPE_BEGIN;
  CAfterStartedProcess procsser(coreplayer);
  procsser.StopThread();

  CSingleLock lock(s_renderlock);

  coreplayer->PrepareStream();

  //coreplayer->OnStarted();

  g_application.OnPlayBackStarted();

  //CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
  //coreplayer->m_bAbortRequest = true;
  //g_application.SwitchToFullScreen();
  g_settings.m_currentVideoSettings.m_SubtitleOn = coreplayer->GetSubtitleVisible();
  g_settings.m_currentVideoSettings.m_AudioStream = coreplayer->GetAudioStream();
  g_settings.m_currentVideoSettings.m_SubtitleStream = coreplayer->GetSubtitle();
  CLog::Log(LOGINFO, "********* callback  ******** m_SubtitleOn = %d *************************", g_settings.m_currentVideoSettings.m_SubtitleOn);
  CLog::Log(LOGINFO, "********* callback  ******** m_AudioStream = %d *************************", g_settings.m_currentVideoSettings.m_AudioStream);
  CLog::Log(LOGINFO, "********* callback  ******** m_SubtitleStream = %d *************************", g_settings.m_currentVideoSettings.m_SubtitleStream);
}

void OnNewVideoFrame( void* owner )
{
  g_application.NewFrame();
}

void OnPlaybackFailed( void* owner )
{
  if (!owner)
    return;

  if (!s_bManualClose)
  {
    CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
    coreplayer->CloseFile(false);

    if (coreplayer->m_bOpenFiled)
      g_windowManager.PreviousWindow();
  }

  g_application.OnPlayBackStopped();

  //CVDMPlayer * coreplayer = (CVDMPlayer *)owner;
  //coreplayer->CloseFile();

}

void OnQueueNextItem( void* owner )
{
  g_application.OnQueueNextItem();
}

void OnPlayBackSpeedChanged( int iSpeed, void* owner )
{
  g_application.OnPlayBackSpeedChanged(iSpeed);
}

void OnPlayBackResumed( void* owner )
{
  g_application.OnPlayBackResumed();
}

void OnPlayBackPaused( void* owner )
{
  g_application.OnPlayBackPaused();
}

void OnPlayBackSeek( int iTime, int seekOffset, void* owner )
{
  g_application.OnPlayBackSeek(iTime, seekOffset);
}


} //VidOnMe


static void OnAudioCallbackInitialize(void* owner, int iChannels, int iSamplesPerSec, int iBitsPerSample)
{
  IAudioCallback* iacallback = (IAudioCallback*)owner;

  if( iacallback )
    iacallback->OnInitialize(iChannels, iSamplesPerSec, iBitsPerSample);
}

static void OnAudioCallbackAudioData(void* owner, const float* pAudioData, int iAudioDataLength)
{
  IAudioCallback* iacallback = (IAudioCallback*)owner;

  if( iacallback )
    iacallback->OnAudioData(pAudioData, iAudioDataLength);
}

static void VDMPlayerLogCallback( int loglevel, const char* pstr, int console_id )
{
  CStdString strLog;
  strLog.Format(" PLAYERCORE:%s", pstr);
  CLog::Log(loglevel, strLog.c_str());
}

CAfterStartedProcess::CAfterStartedProcess(CVDMPlayer * pplayer)
  :CThread("CAfterStartedProcess")
{
  m_pPlayer = pplayer;
  Create();
}

void CAfterStartedProcess::Process()
{
  CLog::Log(LOGINFO, "-------------------------------------CAfterStartedProcess::Process()-------------------Start");
  if (m_pPlayer)
    m_pPlayer->OnStarted();
  CLog::Log(LOGINFO, "-------------------------------------CAfterStartedProcess::Process()-------------------End");
}



//////////////////////////////////////////////////////////////////////////
//class CVDMPlayer 

CVDMPlayer::CVDMPlayer(IPlayerCallback& callback, EPLAYERCORES specifyPlayer)
  : IPlayer(callback),
  CThread("CVDMPlayer"),
  m_ready(true),
  m_specifyPlayer(specifyPlayer)
{
  m_bIsPlaying = false;
  m_paused = false;
  m_playbackStartTime = 0;
  m_speed = 1;
  m_totalTime = 1;
  m_time = 0;

  m_pPlayer = NULL;
  m_playercallback = NULL;
  m_actiontype = ACTION_TYPE_NONE;
  m_actionrender = ACTION_TYPE_NONE;
  m_bOpenFiled = false;
  m_nSpeed = 1;

  s_dllPlayercore.Unload();
}

CVDMPlayer::~CVDMPlayer()
{
  if (m_bIsPlaying)
    CloseFile(true);
  else
    StopThread();

  if( m_playercallback )
    delete m_playercallback;

  // FIXME: urgly implement, current remove all source and avoid memory leap
  // zhang: 20131231, not need remove here, in vdm player, will only keep one playing source
  // the old source will be removed before new source add
  // s_dllPlayercore.LibPlayercore_RemovePlaySourceAll();

  s_dllPlayercore.Unload();
}

bool CVDMPlayer::Initialize(TiXmlElement* pConfig)
{
  return true;
}

void CVDMPlayer::RegisterAudioCallback(IAudioCallback* pCallback)
{
    if( m_pPlayer )
    {
        ICorePlayerAudioCallback callback;

        callback.cbowner = pCallback;
        callback.pfn_initialize = OnAudioCallbackInitialize;
        callback.pfn_audiodata = OnAudioCallbackAudioData;

        m_pPlayer->RegisterAudioCallback(&callback);
    }
}

void CVDMPlayer::UnRegisterAudioCallback()
{
    if( m_pPlayer )
        m_pPlayer->UnRegisterAudioCallback();
}

bool CVDMPlayer::OpenFile(const CFileItem& file, const CPlayerOptions &options)
{
    try
    {
        if (m_bIsPlaying)
        {
            CloseFile(true);
            StopThread();
        }

        m_ready.Reset();
        m_item = file;
        m_bIsPlaying = true;

        m_mimetype = file.GetMimeType();
        m_strProperties = file.GetPropertiesAsString();

        m_PlayerOptions = options;

        if (!s_dllPlayercore.IsLoaded())
        {
            if( !s_dllPlayercore.Load() )
            {
                CLog::Log(LOGERROR, "Load %s failed", s_strPlayerCoreFilePath.c_str());
                return false;
            }
        }

#if defined(HAS_VIDEO_PLAYBACK)
        //g_renderManager.PreInit();
#endif

        // Suspend AE temporarily so exclusive or hog-mode sinks 
        // don't block external player's access to audio device  
        CAEFactory::Suspend();

        if (!s_bPlayerCoreInited)
        {
            InitializePlayerCore();
            if (!s_bPlayerCoreInited)
            {
                //need resume xbmc-audioengine
                if( CAEFactory::IsSuspended() )
                    CAEFactory::Resume();

                //TODO, log it, need the detail information
                return false;
            }
        }

        CLog::Log(LOGNOTICE, "%s: %s : %lf", __FUNCTION__, m_item.GetPath().c_str(), options.starttime);

        //reset manual close flag
        s_bManualClose = false;

        if (g_guiSettings.GetBool("videoplayer.usedxva2"))
            s_dllPlayercore.Libplayercore_RefreshHardwareAccelSetting(ENABLE_HARDWARE_ACCELERATE_DXVA);
        else
            s_dllPlayercore.Libplayercore_RefreshHardwareAccelSetting(DISABLE_HARDWARE_ACCELERATE);

        Create();

        if(!m_ready.WaitMSec(100))
        {
            while(!m_ready.WaitMSec(1))
                ::Sleep(1);
        }

        if (m_actiontype == ACTION_TYPE_OPENFAILED)
        {
            if( CAEFactory::IsSuspended() )
                CAEFactory::Resume();

            return false;
        }

        return true;
    }
    catch(...)
    {
        m_bIsPlaying = false;
        CLog::Log(LOGERROR,"%s - Exception thrown", __FUNCTION__);
        return false;
    }

}

bool CVDMPlayer::QueueNextFile(const CFileItem& file)
{
  if( m_pPlayer )
  {
    SourceFileItem item;
    memset(&item, 0, sizeof(SourceFileItem));

    item.pszFile = file.GetPath();
    item.pszMimeType = file.GetMimeType();

    SetSourceItem(file, item);

    return m_pPlayer->QueueNextFile(&item);
  }

  return false;
}

void CVDMPlayer::OnNothingToQueueNotify()
{
  if( m_pPlayer )
    m_pPlayer->OnNothingToQueueNotify();
}

void CVDMPlayer::OnStarted()
{
  CLog::Log(LOGINFO, "-------------------------------------CVDMPlayer::OnStarted()-------------------Start");
  if (!m_pPlayer)
    return;

  //zhang 20140312:  starttime seek will be done by player internal
  //if (m_PlayerOptions.starttime > 0)
  //  SeekTime(m_PlayerOptions.starttime * 1000);
  
  CLog::Log(LOGINFO, "-------------------------------------CVDMPlayer::OnStarted()-------------------End");
}

bool CVDMPlayer::CloseFile()
{
  return CloseFile(true);
}

bool CVDMPlayer::CloseFile(bool bmanual) 
{
  if (!m_bIsPlaying)
    return true;

  if (m_actiontype == ACTION_TYPE_STOP)
    return true;

  {
    CSingleLock lock(s_renderlock);
    s_bManualClose = true;
    m_actiontype = ACTION_TYPE_STOP;
    m_actionrender = RENDER_TYPE_END;
  }

  m_ready.Reset();

  if (bmanual)
    StopThread();

  m_bIsPlaying = false;

  return true;
}

bool CVDMPlayer::IsPlaying() const
{
  CSingleLock lock(s_renderlock);

  if (!m_pPlayer)
    return false;

  return m_pPlayer->IsPlaying();
}

void CVDMPlayer::Pause()
{
  CSingleLock lock(s_renderlock);

  if (!m_pPlayer)
    return;

  if (m_actiontype != ACTION_TYPE_PAUSE && !IsCaching())
    m_actiontype = ACTION_TYPE_PAUSE;
  else
    m_actiontype = ACTION_TYPE_RUNING;

  m_pPlayer->Pause();
}

bool CVDMPlayer::IsPaused() const
{
  CSingleLock lock(s_renderlock);

  if (!m_pPlayer)
    return true;

  return m_pPlayer->IsPaused();
}

bool CVDMPlayer::HasVideo() const
{
    if( m_pPlayer )
        return m_pPlayer->HasVideo();

    return false;
}

bool CVDMPlayer::HasAudio() const
{
    if( m_pPlayer )
        return m_pPlayer->HasAudio();

    return false;
}

void CVDMPlayer::SwitchToNextLanguage()
{

}

void CVDMPlayer::ToggleSubtitles()
{

}

bool CVDMPlayer::CanSeek()
{
  return true;
}

void CVDMPlayer::Seek(bool bPlus, bool bLargeStep)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  m_pPlayer->Seek(bPlus,bLargeStep);
}

void CVDMPlayer::SeekPercentage(float iPercent)
{
  int64_t iTotalTime = GetTotalTime();

  if (iTotalTime != 0)
  {
    float fpercent = iPercent / (float)100;
    int64_t iTime = (int64_t)((float)iTotalTime * fpercent);
    //CLog::Log(LOGDEBUG, "SeekPercentage is %f", iTime);
    //return iTime * 100 / (float)iTotalTime;
    SeekTime(iTime);
  }
}

float CVDMPlayer::GetPercentage()
{

  int64_t iTime = GetTime();
  int64_t iTotalTime = GetTotalTime();

  if (iTotalTime != 0)
  {
    CLog::Log(LOGDEBUG, "Percentage is %f", (iTime * 100 / (float)iTotalTime));
    return iTime * 100 / (float)iTotalTime;
  }

  return 0.0f;
}

void CVDMPlayer::GetAudioInfo(CStdString& strAudioInfo)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  int nlength = 0;
  m_pPlayer->GetAudioInfo(NULL,nlength);
  if (!nlength)
    return;

  char* sztemp = strAudioInfo.GetBufferSetLength(nlength);
  m_pPlayer->GetAudioInfo(sztemp,nlength);
}

void CVDMPlayer::GetVideoInfo(CStdString& strVideoInfo)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  int nlength = 0;
  m_pPlayer->GetVideoInfo(NULL,nlength);
  if (!nlength)
    return;
  char* sztemp = strVideoInfo.GetBufferSetLength(nlength);
  m_pPlayer->GetVideoInfo(sztemp,nlength);
}

void CVDMPlayer::GetGeneralInfo( CStdString& strVideoInfo)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  int nlength = 0;
  m_pPlayer->GetGeneralInfo(NULL,nlength);
  if (!nlength)
    return;
  char* sztemp = strVideoInfo.GetBufferSetLength(nlength);
  m_pPlayer->GetGeneralInfo(sztemp,nlength);
}

void CVDMPlayer::SwitchToNextAudioLanguage()
{

}

void CVDMPlayer::SetAVDelay(float fValue)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetAVDelay(fValue);	
}

float CVDMPlayer::GetAVDelay()
{
  if (!m_pPlayer)
    return 0;

  return m_pPlayer->GetAVDelay();	
}

void CVDMPlayer::SetSubTitleDelay(float fValue)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetSubTitleDelay(fValue);	
}

float CVDMPlayer::GetSubTitleDelay()
{
  if (!m_pPlayer)
    return 0;

  return m_pPlayer->GetSubTitleDelay();	
}


void CVDMPlayer::SeekTime(int64_t iTime)
{
  CLog::Log(LOGINFO, "-------------------------------------CVDMPlayer::SeekTime(int64_t iTime)-------------------Start");
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return ;

  CLog::Log(LOGNOTICE, "This is Test for SeekTime : %s: %lld", __FUNCTION__, iTime);

  m_pPlayer->SeektoTime(iTime / 1000);
  CLog::Log(LOGINFO, "-------------------------------------CVDMPlayer::SeekTime(int64_t iTime)-------------------End");
}

int64_t CVDMPlayer::GetTime()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;

  return (int64_t)(m_pPlayer->GetCurrentTime() * 1000);
}

int64_t CVDMPlayer::GetTotalTime()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;

  return (int64_t)(m_pPlayer->GetDuration() * 1000);
}

void CVDMPlayer::ToFFRW(int iSpeed)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  m_pPlayer->SetPlaySpeed(iSpeed);
}

void CVDMPlayer::ShowOSD(bool bOnoff)
{

}

CStdString CVDMPlayer::GetPlayerState()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return "";

  int nlength = 0;
  m_pPlayer->GetPlayerState(NULL,nlength);
  if (!nlength)
    return "";

  CStdString sresult;
  char * sztemp = sresult.GetBufferSetLength(nlength);
  m_pPlayer->GetPlayerState(sztemp,nlength);

  return sztemp;
}

bool CVDMPlayer::SetPlayerState(CStdString state)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return false;

  return m_pPlayer->SetPlayerState((char *)state.c_str());
}

#if defined(TARGET_WINDOWS)

BOOL CVDMPlayer::ExecuteAppW32(const char* strPath, const char* strSwitches)
{
  return TRUE;
}

#endif

void CVDMPlayer::Present()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  //if (m_actionrender == RENDER_TYPE_BEGIN)
  {
    m_pPlayer->RenderVideoStream();
  }
}

std::string CVDMPlayer::GetISOlanguage(const std::string& strLanguage )
{
  CStdString srcLanguage;
  if(strLanguage == "Chinese (Simple)")
  {
    srcLanguage = "chi";
  }
  else if(strLanguage == "Chinese (Traditional)")
  {
    srcLanguage = "chi";
  }
  else if(strLanguage == "English")
  {
    srcLanguage = "eng";
  }
  else if(strLanguage == "French")
  {
    srcLanguage = "fre";
  }
  else if(strLanguage == "German")
  {
    srcLanguage = "ger";
  }
  else if(strLanguage == "Japanese")
  {
    srcLanguage = "jpn";
  }
  else if(strLanguage == "Korean")
  {
    srcLanguage = "kor";
  }
  else if(strLanguage == "Portuguese (Brazil)")
  {
    srcLanguage = "por";
  }
  else if(strLanguage == "Spanish")
  {
    srcLanguage = "spa";
  }
  else
  {
    srcLanguage = strLanguage;
  }
  return srcLanguage;
}

void CVDMPlayer::Process()
{
  m_playercallback = new ICorePlayerCallback;

  m_playercallback->cbowner = (void *)this;
  m_playercallback->pfn_OnPlayBackEnded = OnPlaybackEnd;
  m_playercallback->pfn_OnPlayBackStopped = OnPlaybackFailed;
  m_playercallback->pfn_OnPlayBackStarted = OnPlaybackStarted;
  m_playercallback->pfn_OnNewVideoFrame = OnNewVideoFrame;
  m_playercallback->pfn_OnQueueNextItem = OnQueueNextItem;
  m_playercallback->pfn_OnPlayBackResumed = OnPlayBackResumed;
  m_playercallback->pfn_OnPlayBackSpeedChanged = OnPlayBackSpeedChanged;
  m_playercallback->pfn_OnPlayBackPaused = OnPlayBackPaused;
  m_playercallback->pfn_OnPlayBackSeek = OnPlayBackSeek;


  m_pPlayer = s_dllPlayercore.Libplayercore_GetCorePlayer(m_playercallback);

  if (m_pPlayer)
  {
    RESOLUTION res = g_graphicsContext.GetVideoResolution();
    int nx = g_settings.m_ResInfo[res].iWidth;
    int ny = g_settings.m_ResInfo[res].iHeight;

    m_pPlayer->ReSize(nx,ny);
  }

  /*
  {
  CSingleLock lock(s_renderlock);
  m_bOpenSuccessfully = true;
  }
  return;
  */

  SourceFileItem sourceItem;
  memset(&sourceItem, 0, sizeof(SourceFileItem));
  SetSourceItem(m_item, sourceItem);

  m_bOpenFiled = false;

  bool bAudioOnly = false;
  CStdString strFile = m_item.GetPath();
  CLog::Log(LOGDEBUG, "%s: receive playing source path = %s", __FUNCTION__, strFile.c_str());

  IPlayerOptions options;
  sourceItem.pOptions = &options;
  sourceItem.sourceType = PLAYER_SOURCE_TYPE_FILE;

  options.spropertys = m_strProperties.c_str();

#if defined(__ANDROID_ALLWINNER__)
  options.displayMode = g_settings.m_currentVideoSettings.m_DimensionMode;
  if(VidOnMe::VDMUtils::GetCPUType() != VidOnMe::CT_ALLWINNER_A20)
  {
    options.displayMode = g_guiSettings.GetInt("bd.playmode");
  }
#endif

  //playercore config
  PlayercoreConfig config;
  memset(&config, 0, sizeof(PlayercoreConfig));

  if( EPC_PAPLAYER == m_specifyPlayer )
  {
    CLog::Log(LOGDEBUG, "%s: specify using EPC_PAPLAYER", __FUNCTION__);
    sourceItem.playerType = PLAYER_TYPE_AUDIO;
    bAudioOnly = true;
  }
  else
  {
    sourceItem.playerType = PLAYER_TYPE_VIDEO;
    SetVideoPlayingOptions(strFile, sourceItem.sourceType, options, config);
  }

  //set playing source path, this path has been processed
  sourceItem.pszFile = strFile.c_str();

  sourceItem.pszMimeType = m_mimetype.c_str();

  //zhang 20140312:  starttime seek will be done by player internal
  //set start time, and pass to player, let player doing the seek
  options.starttime = m_PlayerOptions.starttime;

  m_bOpenFiled = m_pPlayer->OpenFile(&sourceItem);

  if (m_bOpenFiled)
  {

    float fbrightness = g_settings.m_currentVideoSettings.m_Brightness;
    float fcontrast = g_settings.m_currentVideoSettings.m_Contrast;

    SetAudioOutputOptions();

    m_ready.Set();

    g_application.SwitchToFullScreen();
    g_Windowing.SetEventCallback(this);

    if (m_actiontype == ACTION_TYPE_NONE)
    {
      m_actiontype = ACTION_TYPE_RUNING;
    }

      for(;;)
      {
        {
          CSingleLock lock(s_renderlock);
          if( m_actiontype == ACTION_TYPE_STOP )
          {
            m_ready.Set();
            break;
          }
        }

        if (fbrightness != g_settings.m_currentVideoSettings.m_Brightness)
        {
          m_pPlayer->SetDisplayBrightness(g_settings.m_currentVideoSettings.m_Brightness);
          fbrightness = g_settings.m_currentVideoSettings.m_Brightness;
        }

        if (fcontrast != g_settings.m_currentVideoSettings.m_Contrast)
        {
          m_pPlayer->SetDisplayContrast(g_settings.m_currentVideoSettings.m_Contrast);
          fcontrast = g_settings.m_currentVideoSettings.m_Contrast;
        }

        Sleep(1);
      }
  }
  else
  {
    m_ready.Set();
    m_actiontype = ACTION_TYPE_OPENFAILED;
  }

  if (m_pPlayer)
    m_pPlayer->CloseFile();

  s_dllPlayercore.Libplayercore_ReleaseCorePlayer(m_pPlayer);

  {
    CSingleLock lock(s_renderlock);
    m_pPlayer = NULL;
  }

  /* Resume AE processing of XBMC native audio */
  if( CAEFactory::IsSuspended() )
  {
    if (!CAEFactory::Resume())
    {
      CLog::Log(LOGFATAL, "%s: Failed to restart AudioEngine after return from external player",__FUNCTION__);
    }
  }

  m_actiontype = ACTION_TYPE_OVER;

  g_Windowing.SetEventCallback(NULL);
}

void CVDMPlayer::SetVideoPlayingOptions(CStdString& strFile, int& type, IPlayerOptions& options, PlayercoreConfig& config)
{
  //video playing setting

  CStdString audiolanguage = g_guiSettings.GetString("locale.audiolanguage");
  CStdString subtitlelanguage = g_guiSettings.GetString("locale.subtitlelanguage");

  if(audiolanguage.Equals("default"))
  {
    audiolanguage = g_guiSettings.GetString("locale.language");
  }
  if(subtitlelanguage.Equals("default"))
  {
    subtitlelanguage = g_guiSettings.GetString("locale.language");
  }

  CLog::Log(LOGINFO, "***************** audiolanguage = %s *************************", audiolanguage.c_str());
  CLog::Log(LOGINFO, "***************** subtitlelanguage = %s *************************", subtitlelanguage.c_str());

  s_dllPlayercore.Libplayercore_SetSettingString( CS_DEFAULT_AUDIO_LANGUAGE, GetISOlanguage(audiolanguage).c_str() );
  s_dllPlayercore.Libplayercore_SetSettingString( CS_DEFAULT_SUBTITLE_LANGUAGE, GetISOlanguage(subtitlelanguage).c_str() );

  CStdString custompath = g_guiSettings.GetString("subtitles.custompath");
  s_dllPlayercore.Libplayercore_SetSettingString( CS_SUBTITLE_SEARCH_PATH, g_guiSettings.GetString("subtitles.custompath"));

  s_dllPlayercore.LibPlayercore_SetSettingBool(BA_DVD_AUTOMENU, g_guiSettings.GetBool("dvds.automenu"));

  CLog::Log(LOGINFO, "***************** custompath = %s *************************", custompath.c_str());
  CLog::Log(LOGINFO, "***************** dvds.automenu = %d *************************", g_guiSettings.GetBool("dvds.automenu"));


  //TODO, need test connect remote server
  //input path style example
  //VDM-mode:
  // 
  // bluray/folder/menu:
  // bluray/folder/playlist:
  //  
  // bluray/iso/menu:
  // bluray/iso/playlist:
  // 
  //XBMC-mode:
  // bluray/folder/menu:      bluray://Y%3a%5csever_source%5cbluray%5cMission%20Impossible%20-%20Ghost%20Protocol%202011%201080p%20Blu-ray%20AVC%20TrueHD%207.1-DIY-HDChina%5c/BDMV/MovieObject.bdmv
  // bluray/folder/playlist:  Y:\sever_source\bluray\Mission Impossible - Ghost Protocol 2011 1080p Blu-ray AVC TrueHD 7.1-DIY-HDChina\/BDMV/PLAYLIST/00800.mpls
  //  
  // bluray/iso/menu:      bluray://udf%3a%2f%2fY%253a%255csever_source%255cbluray%255c2012.iso%2f/BDMV/MovieObject.bdmv
  // bluray/iso/playlist:  Y:\sever_source\bluray\2012.iso/BDMV/PLAYLIST/00001.mpls
  //  
  // 


  //in xbmc-mode, need extract the bluray:// udf:// first 
  if( strFile.Left(7).Equals("bluray:") )
  {
    //bluray, not need mimetype
    m_mimetype = "";

    CURL url(strFile);
    strFile = url.GetHostName();

    if( strFile.Left(4).Equals("udf:") )
    {
      CURL url(strFile);
      strFile = url.GetHostName();
    }
    else
    {
      //not udf wrap, must be folder
      type = PLAYER_SOURCE_TYPE_FOLDER;
    }
  }

  //parser the path, make sure it's playlist / menu mode 
  //parser the source path, make sure it's iso or fake-folder iso

  //first check it's title mode

  CStdString sfileext = URIUtils::GetExtension(strFile);

  if( sfileext.Equals(".title") )
  {
    CURL::Decode(strFile);

    CStdString strFileName, strPath;

    //directly to get title.name 
    strFileName = strFile.Right(11);						//00000.title
    strPath = strFile.Left(strFile.size()-12);	// /00000.title = 12

    strFileName.MakeLower();

    int playlist = -1;
    sscanf(strFileName.c_str(), "%05d.title", &playlist);

    options.playlist = playlist;

    URIUtils::RemoveSlashAtEnd(strPath);
    strFile = strPath;
  }
  else 
  {
    //old working logic 

    if( sfileext != ".iso" )
    {
      CStdString strExtension, strFileName, strPath;

      URIUtils::GetExtension(strFile,strExtension);
      strExtension.MakeLower();

      //extract the playlist 
      strFileName = strFile;
      URIUtils::Split(strFile, strPath, strFileName);

      if( ".mpls" == strExtension )
      {
        //bluray, not need mimetype
        m_mimetype = "";

        strFileName.MakeLower();

        int playlist = -1;
        sscanf(strFileName.c_str(), "%05d.mpls", &playlist);

        options.playlist = playlist;

        URIUtils::GetParentPath(strPath, strFile);		//remove PLAYLIST
        URIUtils::GetParentPath(strFile, strPath);		//remove BDMV
        URIUtils::RemoveSlashAtEnd(strPath);
        strFile = strPath;

        //the BDMV/PLAYLIST/BDMV has been removed, I need check it's file or folder
        URIUtils::GetExtension(strFile,strExtension);

        if( strExtension.Equals(".iso") )
          type = PLAYER_SOURCE_TYPE_FILE;
        else
          type = PLAYER_SOURCE_TYPE_FOLDER;
      }

      if( strFileName.Equals("MovieObject.bdmv") || strFileName.Equals("index.bdmv") )
      {
        //no specify the playlist, menu mode or longest 
        URIUtils::GetParentPath(strPath, strFile);		//remove PLAYLIST
        strFile = strPath;
        type = PLAYER_SOURCE_TYPE_FOLDER;
        m_mimetype = "";
      }
    }
  }


  s_dllPlayercore.Libplayercore_SetBDRegion(g_guiSettings.GetInt("bd.regincode"));


  CStdString strCharset = g_langInfo.GetSubtitleCharSet();
  memcpy(config.charset, strCharset.c_str(), strCharset.GetLength());

  unsigned int colormap[8] = { 0xFFFFFF00, 0xFFFFFFFF, 0xFF0099FF, 0xFF00FF00, 0xFFCCFF00, 0xFF00FFFF, 0xFFE5E5E5, 0xFFC0C0C0 };
  config.fontcolor = colormap[g_guiSettings.GetInt("subtitles.color")];
  config.fontstyle = g_guiSettings.GetInt("subtitles.style");
  config.fontsize = g_guiSettings.GetInt("subtitles.height");
  s_dllPlayercore.Libplayercore_RefreshFontSetting(&config);

  options.restoreAudioId = g_settings.m_currentVideoSettings.m_AudioStream;
  options.restoreSubtitleId = g_settings.m_currentVideoSettings.m_SubtitleStream;
  if(g_settings.m_currentVideoSettings.m_SubtitlePath != "")
  {
    strcpy(options.subtitlepath, g_settings.m_currentVideoSettings.m_SubtitlePath.c_str());
  }
  else
  {
    memset(options.subtitlepath,NULL,sizeof(options.subtitlepath));
  }

  CLog::Log(LOGINFO, "*****************options.restoreAudioId = %d *************************", options.restoreAudioId );
  CLog::Log(LOGINFO, "*****************options.restoreSubtitleId = %d *************************", options.restoreSubtitleId );
  CLog::Log(LOGINFO, "*****************g_settings.m_currentVideoSettings.m_SubtitlePath = %s *************************", g_settings.m_currentVideoSettings.m_SubtitlePath.c_str() );
  CLog::Log(LOGINFO, "*****************options.subtitlepath = %s *************************", options.subtitlepath );
}

void CVDMPlayer::SetAudioOutputOptions()
{
  //m_pPlayer->SetDisplayRatio(VIEW_MODE_STRETCH_4x3);
  CStdString currentPassthroughDevice =  g_guiSettings.GetString("audiooutput.passthroughdevice");
  CStdString currentDevice = g_guiSettings.GetString("audiooutput.audiodevice");
  int aomode = g_guiSettings.GetInt("audiooutput.mode");
  int nchannel = g_guiSettings.GetInt("audiooutput.channels");

  AudioSetting * paudiosettings = s_dllPlayercore.LibPlayercore_GetAudioSettings();

  if (aomode == AUDIO_ANALOG)
  {
#if defined(__TARGET_ANDROID_ALLWINNER__)
    s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice("audiotrack",DEVICE_TYPE_COMMON);
#else
    //s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice(currentDevice.c_str(),DEVICE_TYPE_COMMON);
    int ncount = paudiosettings->deviceCounts;
    bool bfind =false;
    for (int i=0; i<ncount; i++)
    {
      if (!strcmp(paudiosettings->pDevices[i]->pszDeviceName, currentDevice.c_str()))
      {

        if (paudiosettings->pDevices[i]->outType == AUDIO_OUT_TYPE_ANALOG)
        {
          s_dllPlayercore.LibPlayercore_SetAudioOutputDevice(i);
          bfind = true;
          break;
        }
      }
    }

    if (!bfind)
    {
      s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice(currentDevice.c_str(),DEVICE_TYPE_COMMON);
    }
#endif
  }
  else 
  {
    int ncount = paudiosettings->deviceCounts;
    bool bfind =false;
    int type = 0;

    if (aomode == AUDIO_IEC958)
      type = AUDIO_OUT_TYPE_DIGITAL;
    else if (aomode == AUDIO_HDMI)
      type = AUDIO_OUT_TYPE_DISPLAYPORT;

#if defined(__TARGET_ANDROID_ALLWINNER__)
    if (aomode == AUDIO_IEC958)
      s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice("aw-spdif",DEVICE_TYPE_PASSTHROUGH);
    else if (aomode == AUDIO_HDMI)
      s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice("aw-hdmi",DEVICE_TYPE_PASSTHROUGH);

    s_dllPlayercore.LibPlayercore_SetAudioHdmiPassthroughType(g_guiSettings.GetInt("audiooutput.passthroughmode"));
#else
    for (int i=0; i<ncount; i++)
    {
      if (!strcmp(paudiosettings->pDevices[i]->pszDeviceName, currentPassthroughDevice.c_str()))
      {

        if (paudiosettings->pDevices[i]->outType == type)
        {
          s_dllPlayercore.LibPlayercore_SetAudioOutputDevice(i);
          bfind = true;
          break;
        }
      }
    }

    if (!bfind)
    {
      s_dllPlayercore.LibPlayercore_SetAudioDefaultOutputDevice(currentPassthroughDevice.c_str(),DEVICE_TYPE_PASSTHROUGH);
    }
#endif
    int supportCodec = 0;
    if (g_guiSettings.GetBool("audiooutput.ac3passthrough"))
      supportCodec |= PASSTHROUGH_CODEC_AC3;
    if (g_guiSettings.GetBool("audiooutput.dtspassthrough"))
      supportCodec |= PASSTHROUGH_CODEC_DTS;
    if (g_guiSettings.GetBool("audiooutput.passthroughaac"))
      supportCodec |= PASSTHROUGH_CODEC_AAC;
    if (g_guiSettings.GetBool("audiooutput.multichannellpcm"))
      supportCodec |= PASSTHROUGH_CODEC_LPCM;
    if (g_guiSettings.GetBool("audiooutput.truehdpassthrough"))
      supportCodec |= PASSTHROUGH_CODEC_TRUEHD;
    if (g_guiSettings.GetBool("audiooutput.dtshdpassthrough"))
      supportCodec |= PASSTHROUGH_CODEC_DTSHD;

    s_dllPlayercore.LibPlayercore_SetAudioPassthroughCodec(supportCodec);
  }

  s_dllPlayercore.LibPlayercore_SetAudioOutputSpeakers(nchannel);
}

void CVDMPlayer::SetSourceItem(const CFileItem& fileItem, SourceFileItem& sourceItem)
{
  //set file item other attribute to source item, to make sure not losing other information

  sourceItem.startOffset = m_item.m_lStartOffset;
  sourceItem.endOffset = m_item.m_lEndOffset;
  sourceItem.startPartNumber = m_item.m_lStartPartNumber;
}

void CVDMPlayer::OnReSize(int newWidth, int newHeight, int newLeft, int newTop)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  m_pPlayer->ReSize(newWidth, newHeight);
}

bool CVDMPlayer::OnAction(const CAction &action)
{
  if (!m_pPlayer)
    return false;

  bool bret = false;

  switch (action.GetID())
  {
  case ACTION_STEP_FORWARD:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOVE_RIGHT);
    }
    break;
  case ACTION_STEP_BACK:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOVE_LEFT);
    }
    break;
  case ACTION_BIG_STEP_FORWARD:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOVE_UP);
    }
    break;
  case ACTION_BIG_STEP_BACK:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOVE_DOWN);
    }
    break;
  case ACTION_SHOW_OSD:
    {
      if (action.GetButtonCode() == 0xF00D)
        bret =m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_ENTER);
    }
    break;
  case ACTION_MOUSE_MOVE:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOUSE_MOVE,action.GetAmount(),action.GetAmount(1));
    }
    break;
  case ACTION_MOUSE_LEFT_CLICK:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_MOUSE_CLICK,action.GetAmount(),action.GetAmount(1));
    }
    break;
  case ACTION_NEXT_ITEM:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_NEXT_ITEM);
    }
    break;
  case ACTION_PREV_ITEM:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_PREV_ITEM);
    }
    break;
  case ACTION_SHOW_VIDEOMENU:
    {
      bret = m_pPlayer->OnAction(CCorePlayer::PLAYERCORE_ACTION_SHOWVIDEOMENU,action.GetAmount());
    }
    break;
  default:
    break;
  }
  return bret;
}

int  CVDMPlayer::GetChapterCount()
{
  //CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->GetChapterCount();
}

int  CVDMPlayer::GetChapter()
{
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->GetChapter();
}

void CVDMPlayer::GetChapterName(CStdString& strChapterName)
{
  if (!m_pPlayer)
    return;

  char szchapter[255] = {'\0'};
  m_pPlayer->GetChapterName(szchapter);

  strChapterName = szchapter;
}

int  CVDMPlayer::SeekChapter(int iChapter)
{
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->SeekChapter(iChapter);
}

int CVDMPlayer::GetSubtitleCount()
{
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->GetSubtitleCount();
}

int CVDMPlayer::GetSubtitle()
{
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->GetSubtitleCurrentStream();
}

void CVDMPlayer::GetSubtitleName(int iStream, CStdString &strStreamName)
{
  if (!m_pPlayer)
    return ;

  char szsubname[255] = {'\0'};

  m_pPlayer->GetSubtitleName(iStream, szsubname);
  strStreamName = szsubname;
}

void CVDMPlayer::GetSubtitleLanguage(int iStream, CStdString &strStreamLang)
{

}

void CVDMPlayer::SetSubtitle(int iStream)
{
  if (!m_pPlayer)
    return ;

  CLog::Log(LOGINFO, "*****************SetSubtitle's iStream = %d *************************", iStream );
  m_pPlayer->SelectSubtitle(iStream);
}

bool CVDMPlayer::GetSubtitleVisible()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return false;

  return m_pPlayer->GetSubtitleVisible();
}

void CVDMPlayer::SetSubtitleVisible(bool bVisible)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetSubtitleVisible(bVisible);
}

void CVDMPlayer::SetDisplayBrightness( float fBrightness )
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetDisplayBrightness(fBrightness);
}

void CVDMPlayer::SetDisplayRatio( int nstyle )
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetDisplayRatio(nstyle);
}

int CVDMPlayer::GetAudioStreamCount()
{
  if (!m_pPlayer)
    return 0;

  return m_pPlayer->GetAudioStreamCount();
}

int CVDMPlayer::GetAudioStream()
{
  if (!m_pPlayer)
    return -1;

  return m_pPlayer->GetAudioCurrentStream();
}

void CVDMPlayer::GetAudioStreamName(int iStream, CStdString &strStreamName)
{
  if (!m_pPlayer)
    return ;

  char szAudioName[255] = {'\0'};
  m_pPlayer->GetAudioStreamName(iStream, szAudioName);
  strStreamName = szAudioName; 
}

void CVDMPlayer::SetAudioStream(int iStream)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetAudioStream(iStream);

#if defined(__VIDONME_MEDIACENTER__)

  g_application.SetAudioStreamChange();

#endif
}

void CVDMPlayer::GetAudioStreamLanguage(int iStream, CStdString &strLanguage)
{
  if (!m_pPlayer)
    return ;

  /*
  //TODO
  char szAudioLanguage[255] = {'\0'};
  m_pPlayer->GetAudioStreamLanguage(iStream, szAudioLanguage);
  strLanguage = szAudioLanguage; 
  */
}

void CVDMPlayer::SetVolume(float nVolume)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetAudioVolume(nVolume,false);
}

void CVDMPlayer::SetMute(bool bOnOff)
{
  if (!m_pPlayer)
    return ;

  m_pPlayer->SetMute(bOnOff);
}

bool CVDMPlayer::HasMenu()
{
  if (!m_pPlayer)
    return false;

  return m_pPlayer->IsInMenu();
}

bool CVDMPlayer::IsInMenu() const
{
  if (!m_pPlayer)
    return false;

  return m_pPlayer->IsInMenu();
}

void CVDMPlayer::SetDynamicRangeCompression(long drc)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  m_pPlayer->SetVolumeAmplification(drc);
}

#if defined(__ANDROID_ALLWINNER__)

bool CVDMPlayer::SetInputDimensionType (int type)
{
  if (!m_pPlayer)
    return false;

  return m_pPlayer->SetInputDimensionType (type);
}

bool CVDMPlayer::SetOutputDimensionType (int type)
{
  if (!m_pPlayer)
    return false;

  return m_pPlayer->SetOutputDimensionType (type);
}

#endif //__ANDROID_ALLWINNER__

void CVDMPlayer::InitializePlayerCore()
{
  if( s_bPlayerCoreInited )
    return;

  if( !s_dllPlayercore.IsLoaded() )
  {
#if defined(TARGET_ANDROID)
#if defined(__ANDROID_ALLWINNER__)
    if (VDMUtils::GetCPUType() == CT_ALLWINNER_A20)
    {
      s_strPlayerCoreFilePath = PLAYERCORE_PATH_A20;
    }
    else if (VDMUtils::GetCPUType() == CT_ALLWINNER_A31)
    {
      s_strPlayerCoreFilePath = PLAYERCORE_PATH_A31;
    }
#endif
#elif defined(TARGET_WINDOWS)
    s_strPlayerCoreFilePath = PLAYERCORE_PATH_WIN;
#endif

    s_dllPlayercore.SetFile(s_strPlayerCoreFilePath);

    if( !s_dllPlayercore.Load() )
    {
      CLog::Log(LOGERROR, "playercore: load playercore failed !");
      return;
    }
  }

  //basic init, just for can using playercore api

  PlayercoreConfig config;
  memset(&config, 0, sizeof(PlayercoreConfig));

  config.working_mode = PLAYERCORE_MODE_PLUGIN_PLAYER;
#if defined(__ANDROID_ALLWINNER__)
  s_dllPlayercore.Libplayercore_SetANativeActivity (CXBMCApp::GetCurrentActivity ());
  s_dllPlayercore.Libplayercore_SetANativeWindow (CXBMCApp::GetNativeWindow ());
  s_dllPlayercore.Libplayercore_SetExternalRenderManager (&g_renderManager);
  s_dllPlayercore.Libplayercore_SetExternalMatrices (&g_matrices);
#endif

#if defined(__ANDROID_ALLWINNER__)
  config.pfn_guishader_callback = on_guishader_callback;
  CXBMCApp::GetScreenDimension (config.width, config.height);
  CLog::Log (LOGDEBUG, "### screen width:%d / %d ##", config.width, config.height);
  CLog::Log(LOGDEBUG, "regidit pfn_guisetting_getint_callback");
  config.pfn_guisetting_getint_callback = on_guisetting_getint_callback;
#endif

  //set log callback function, current log all using xbmc log system to output
#if defined(TARGET_WINDOWS) && defined(_DEBUG)
  //let playercore output log itself, in windows, output to console
  config.pfn_log_callback = NULL;
  config.bEnableConsoleLogger = true;
#else
  config.pfn_log_callback = VDMPlayerLogCallback;
#endif
  //playercore will be put into system\player\dvdplayer\ 
  //get xbmc-bin correct folder

  CStdString strExePath = CUtil::ResolveExecutablePath();
  CStdString strHomePath;

  CStdString basePath = CSpecialProtocol::TranslatePath("special://xbmcbin/");
  if (!basePath.IsEmpty())
    strHomePath = basePath;

  //URIUtils::GetParentPath(strExePath, strHomePath);

  strcpy(config.pszHomePath, strHomePath.c_str());

  //TODO, using xbmc userfolder and tempfolder 

  strcpy(config.pszUserFolderPath, strHomePath.c_str());

  strcpy(config.pszTempFolderPath, strHomePath.c_str());

  CLog::Log(LOGDEBUG, "##TEMP: %s ###", strHomePath.c_str());

#if defined(_WIN32)
  config.outerWindowHandle = g_Windowing.GetHwnd();

  RECT rc;
  ::GetClientRect(g_Windowing.GetHwnd() , &rc);
  config.width = rc.right - rc.left;
  config.height = rc.bottom - rc.top;

  config.d3d9 = g_Windowing.GetD3D();
  config.d3d9device = g_Windowing.Get3DDevice();
#endif

  //specify the which app to call playercore
  config.log_app = PLAYERCORE_LOG_APP_VIDONME;

  //external filesystem support, support playecore to playing smb / upnp file
  LoadExternalFileSystem(&config);

  s_bPlayerCoreInited = s_dllPlayercore.Libplayercore_Init(&config);
}

void CVDMPlayer::UninitializePlayerCore()
{
  if (s_bPlayerCoreInited)
  {
    CleanPlaySource();
    s_dllPlayercore.Libplayercore_DeInit();
    s_bPlayerCoreInited = false;
  }

  if (s_dllPlayercore.IsLoaded())
  {
    // Do not Unload is for UnableExitError while playing.
    //s_dllPlayercore.Unload();
  }
}

void CVDMPlayer::SetSMBPassword(const std::string& strPassword)
{
  if( !s_bPlayerCoreInited )
    return ;
  s_dllPlayercore.Libplayercore_SetSettingString( CS_SAMBA_PASSWORD, strPassword.c_str() );
}
void CVDMPlayer::SetSMBUserName(const std::string& strUserName)
{
  if( !s_bPlayerCoreInited )
    return ;
  s_dllPlayercore.Libplayercore_SetSettingString( CS_SAMBA_USERNAME, strUserName.c_str() );
}

bool CVDMPlayer::AddPlaySource(const std::string& strFilePath)
{
  CSingleLock lock(s_sourcesMutex);

  if( !s_bPlayerCoreInited )
    return false;

  //default set as file

  int type = PLAYER_SOURCE_TYPE_FILE;

  if (XFILE::CDirectory::Exists(strFilePath))
  {
    type = PLAYER_SOURCE_TYPE_FOLDER;
  }

  return s_dllPlayercore.LibPlayercore_AddPlaySource(strFilePath.c_str(), type);
}

void CVDMPlayer::RemovePlaySource(const std::string& strFilePath)
{
  CSingleLock lock(s_sourcesMutex);

  if( !s_bPlayerCoreInited )
    return;

  int type = PLAYER_SOURCE_TYPE_FILE;

  s_dllPlayercore.LibPlayercore_RemovePlaySource(strFilePath.c_str(), type);
}

void CVDMPlayer::CleanPlaySource()
{
  CSingleLock lock(s_sourcesMutex);

  if( !s_bPlayerCoreInited )
    return;

  s_dllPlayercore.LibPlayercore_RemovePlaySourceAll();
}

int CVDMPlayer::GetPlaySourcePlaylistCount(const std::string& strFilePath)
{
  CSingleLock lock(s_sourcesMutex);

  if( !s_bPlayerCoreInited )
    return 0;

  return s_dllPlayercore.LibPlayercore_GetPlaySourcePlaylistCount(strFilePath.c_str());
}

CVDMPlayer::Playlists CVDMPlayer::GetPlaySourcePlaylists(const std::string& strFilePath)
{
  CSingleLock lock(s_sourcesMutex);

  CVDMPlayer::Playlists playlists;

  if( !s_bPlayerCoreInited )
    return playlists;

  CStdString filePath = strFilePath;
  int nPlaylistCount = GetPlaySourcePlaylistCount(strFilePath);

  if( nPlaylistCount > 0 )
  {
    bool bMainMovieSet = false;
    int nMaxDurationIndex = 0;

    for (int i = 0; i < nPlaylistCount; i++)
    {
      CVDMPlayer::PlaylistPtr playlist = GetPlaySourcePlaylistByIndex(strFilePath, i);
      if (playlist)
      {
        playlists.push_back(playlist);
        if (playlist->bMainMovie)
        {
          bMainMovieSet = true;
        }
        else
        {
          if (playlist->nDurationMs > playlists[nMaxDurationIndex]->nDurationMs)
          {
            nMaxDurationIndex = playlists.size() - 1;
          }
        }
      }
    }

    if ((!bMainMovieSet) && (playlists.size() > 0))
    {
      playlists[nMaxDurationIndex]->bMainMovie = true;
    }
  }

  return playlists;
}

static CVDMPlayer::PlaylistPtr ConvertPlaylistObject( const char* pszFilePath, XSource::CPlaylist* pRawPlaylist )
{
  CVDMPlayer::PlaylistPtr playlist = CVDMPlayer::PlaylistPtr(new CVDMPlayer::Playlist());

  playlist->nDurationMs = pRawPlaylist->m_durationMs;
  playlist->nPlaylist = pRawPlaylist->m_Playlist;
  playlist->nIndex = pRawPlaylist->m_index;
  playlist->nAngles = pRawPlaylist->m_angles;

  playlist->b3D = pRawPlaylist->m_b3D;
  playlist->bMainMovie = pRawPlaylist->m_bMainMovie;
  playlist->bMenu = pRawPlaylist->m_bMenu;

  playlist->nChapterCount = pRawPlaylist->m_chapterCount;

  CStdString strPlayPath = pszFilePath;

  if (!URIUtils::HasSlashAtEnd(strPlayPath))
  {
    URIUtils::AddSlashAtEnd(strPlayPath);
  }

  strPlayPath.Format("%s%05d.title", strPlayPath.c_str(), playlist->nPlaylist);

  playlist->strPlayPath = strPlayPath;

  return playlist;
}

CVDMPlayer::PlaylistPtr CVDMPlayer::GetPlaySourcePlaylistByIndex(const std::string& strFilePath, int index)
{
  CSingleLock lock(s_sourcesMutex);

  CVDMPlayer::PlaylistPtr playlist;

  if( !s_bPlayerCoreInited )
    return playlist;

  XSource::CPlaylist* pRawPlaylist = s_dllPlayercore.LibPlayercore_GetPlaySourcePlaylistByIndex(strFilePath.c_str(), index);

  if (pRawPlaylist && pRawPlaylist->m_durationMs >= 60 * 1000 )
    playlist = ConvertPlaylistObject(strFilePath.c_str(), pRawPlaylist);

  return playlist;
}

CVDMPlayer::PlaylistPtr CVDMPlayer::GetPlaySourceMainMoviePlaylist(const std::string& strFilePath)
{
  CSingleLock lock(s_sourcesMutex);

  CVDMPlayer::PlaylistPtr playlist;

  if( !s_bPlayerCoreInited )
    return playlist;

  CStdString filePath = strFilePath;
  XSource::CPlaylist* pRawPlaylist = s_dllPlayercore.LibPlayercore_GetPlaySourceMainmoviePlaylist(strFilePath.c_str());

  if (pRawPlaylist)
    playlist = ConvertPlaylistObject(strFilePath.c_str(), pRawPlaylist);

  return playlist;
}

void CVDMPlayer::GetPlaySourceMetaKeyValue(const std::string& strFilePath, const std::string& strKey, std::string& strValue)
{
  if( !s_bPlayerCoreInited )
    return;

  const char* szValue = s_dllPlayercore.LibPlayercore_GetPlaySourceMeta(strFilePath.c_str(), strKey.c_str());

  if (szValue)
    strValue = szValue;
}

void CVDMPlayer::GetPlaySourceMetaInfo(const std::string& strFilePath, PlaySourceMetaInfo& metaInfo)
{
  CSingleLock lock(s_sourcesMutex);

  if( !s_bPlayerCoreInited )
    return;

  //need call parser it first, if not any playlist, still have any meta information ?

  int playlist = GetPlaySourcePlaylistCount(strFilePath);

  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "FileFormat", metaInfo.strFileFormat);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "VideoCodec", metaInfo.strVideoCodec);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "VideoResolution", metaInfo.strVideoResolution);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "AudioChannel", metaInfo.strAudioChannel);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "AudioCodec", metaInfo.strAudioCodec);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "AudioLanguage", metaInfo.strAudioLanguage);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "SubtitleCodec", metaInfo.strSubtitleCodec);
  GetPlaySourceMetaKeyValue(strFilePath.c_str(), "SubLanguage", metaInfo.strSubtitleLanguage);

  //menu mode support value: none, hdmv, bdj

  if( metaInfo.strFileFormat == "bluray" )
    GetPlaySourceMetaKeyValue(strFilePath.c_str(), "MenuMode", metaInfo.strMenuMode);
}

std::string CVDMPlayer::GetPlaySourceThumbnail(const std::string& strFilePath, int playlist, int width, int height, int time)
{
  CSingleLock lock(s_sourcesMutex);

  std::string strThumbnail;

  if( !s_bPlayerCoreInited )
    return strThumbnail;

#if defined(TARGET_ANDROID)
  //cache on sd card
  CStdString strDestPath = CSpecialProtocol::TranslatePath("special://home/temp/");
#else
  CStdString strDestPath = CSpecialProtocol::TranslatePath("special://temp/");
#endif

  CStdString strFileName = URIUtils::GetFileName(strFilePath.c_str());
  strFileName = strFileName.Left(strFileName.ReverseFind('.'));
  strDestPath += strFileName;

  if (!URIUtils::HasSlashAtEnd(strDestPath))
  {
    URIUtils::AddSlashAtEnd(strDestPath);
  }

  CStdString strDestFolder = strDestPath;
  strDestPath.Format("%s%05d.jpg", strDestPath.c_str(), playlist);

  bool bNeedsRecaching = false;
  CStdString strLoadPath = CTextureCache::Get().CheckCachedImage(strDestPath, true, bNeedsRecaching);
  if (!bNeedsRecaching && !strLoadPath.IsEmpty())
  {
    return strLoadPath.c_str();
  }

  if (!XFILE::CDirectory::Exists(strDestFolder))
  {
    XFILE::CDirectory::Create(strDestFolder);
  }

  bool bRet = s_dllPlayercore.Libplayercore_GetPlaySourceThumb(strFilePath.c_str(), strDestPath.c_str(), width, height, time, playlist);

  if (bRet)
  {
    CLog::Log(LOGINFO, "Extract image from %s to %s successfully!", strFilePath.c_str(), strDestPath.c_str());

    CTextureDetails details;
    strThumbnail = CTextureCache::Get().CacheImage(strDestPath, NULL, &details).c_str();
    if (!strThumbnail.empty())
    {
      CLog::Log(LOGINFO, "Cache image from %s to %s successfully!", strDestPath.c_str(), strThumbnail.c_str());
    }
    else
    {
      CLog::Log(LOGINFO, "Cache image from %s to %s failed!", strDestPath.c_str(), strThumbnail.c_str());
    }

    XFILE::CFile::Delete(strDestPath);
  }
  else
  {
    CLog::Log(LOGINFO, "Extract image from %s to %s failed!", strFilePath.c_str(), strDestPath.c_str());
  }  

  return strThumbnail;
}

void CVDMPlayer::PrepareStream()
{
  if (!m_pPlayer)
    return ;
  m_pPlayer->GetAllStreamInfo();
}

CStdString CVDMPlayer::GetAudioCodecName()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return "";
  PrepareStream();

  MediaStreamInfo * pinfo = m_pPlayer->GetCurrentStreamInfo(1);
  if (!pinfo)
    return "";

  return pinfo->codec;
}

CStdString CVDMPlayer::GetVideoCodecName()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return "";
  PrepareStream();
  MediaStreamInfo * pinfo = m_pPlayer->GetCurrentStreamInfo(2);
  if (!pinfo)
    return "";

  return pinfo->codec;
}

int CVDMPlayer::GetChannels()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;
  PrepareStream();
  MediaStreamInfo * pinfo = m_pPlayer->GetCurrentStreamInfo(1);
  if (!pinfo)
    return 0;

  return pinfo->channels;
}

int CVDMPlayer::GetPictureWidth()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;
  PrepareStream();
  MediaStreamInfo * pinfo = m_pPlayer->GetCurrentStreamInfo(2);
  if (!pinfo)
    return 0;

  return pinfo->width;
}

int CVDMPlayer::GetPictureHeight()
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;
  PrepareStream();
  MediaStreamInfo * pinfo = m_pPlayer->GetCurrentStreamInfo(2);
  if (!pinfo)
    return 0;

  return pinfo->height;
}

void CVDMPlayer::GetVideoAspectRatio(float& fAR)
{
  fAR = 0.0f;
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return;

  fAR = m_pPlayer->GetVideoAspectRatio();
}

bool CVDMPlayer::GetCurrentSubtitle(CStdString& strSubtitle)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return false;

  char * stemp;
  stemp = (char *)m_pPlayer->GetCurrentSubtitle();
  if (strcmp(stemp,"") == 0)
    return false;

  strSubtitle = stemp;
  return true;
}

bool CVDMPlayer::IsCaching() const
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return false;

  return m_pPlayer->IsCaching();
}

int CVDMPlayer::GetCacheLevel() const
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return 0;

  return m_pPlayer->GetCacheLevel();
}

int CVDMPlayer::AddSubtitle(const CStdString& strSubPath)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return -1;

  CLog::Log(LOGINFO, "*****************AddSubtitle's strSubPath = %s *************************", strSubPath.c_str());
  return m_pPlayer->AddSubtitle(strSubPath.c_str());
}

const char*  CVDMPlayer::GetSubtitlePath(int iStream)
{
  CSingleLock lock(s_renderlock);
  if (!m_pPlayer)
    return NULL;

  return m_pPlayer->GetSubtitlePath(iStream);
}

//////////////////////////////////////////////////////////////////////////
//local filesystem support 

using namespace XFILE;

struct ExternalFileHandle
{
  IFile* pFile;

  //TODO, other member

};

static ExternalFileHandle* Exfsys_Open(const char* path)
{
  ExternalFileHandle* exhandle;

  CURL url(path);

  IFile* file = XFILE::CFileFactory::CreateLoader(url);

  if( file )
  {
    if( file->Open(url) )
    {
      exhandle = new ExternalFileHandle;
      exhandle->pFile = file;
    }
    else
    {
      CLog::Log(LOGERROR, "Open file(%s) failed", path);
    }
  }

  return exhandle;
}

static void Exfsys_Close(ExternalFileHandle* h)
{
  if( h && h->pFile )
  {
    h->pFile->Close();
    delete h->pFile;
    delete h;
  }
}

static int64_t Exfsys_Seek(ExternalFileHandle* h, int64_t iFilePosition, int iWhence)
{
  if( h && h->pFile )
    return h->pFile->Seek(iFilePosition, iWhence);

  return 0;
}

static unsigned int Exfsys_Read(ExternalFileHandle* h, void* lpBuf, int64_t uiBufSize, int flags)
{
  if( h && h->pFile )
    return h->pFile->Read(lpBuf, uiBufSize);

  return 0;
}

static int Exfsys_Stat(ExternalFileHandle* h, struct __stat64* buffer)
{
  if( h && h->pFile )
    return h->pFile->Stat(buffer);

  return 0;
}

static int Exfsys_Truncate(ExternalFileHandle* h, int64_t size)
{
  if( h && h->pFile )
    return h->pFile->Truncate(size);

  return 0;
}

static int64_t Exfsys_GetLength(ExternalFileHandle* h)
{
  if( h && h->pFile )
    return h->pFile->GetLength();

  return 0;
}

static int64_t Exfsys_GetPosition(ExternalFileHandle* h)
{
  if( h && h->pFile )
    return h->pFile->GetPosition();

  return 0;
}

static int  Exfsys_GetChunkSize(ExternalFileHandle* h)
{
  if( h && h->pFile )
    return h->pFile->GetChunkSize();

  return 0;
}

static bool Exfsys_Exists(const char* path)
{
  CURL url(path);

  IFile* file = XFILE::CFileFactory::CreateLoader(url);

  if( file )
    return file->Exists(url);

  return false;
}

static int Exfsys_StatEx(const char* path, struct __stat64* buffer)
{
  CURL url(path);

  IFile* file = XFILE::CFileFactory::CreateLoader(url);

  if( file )
    return file->Stat(url, buffer);

  return 0;
}

//Directory support 

using namespace XFILE;

struct ExternalDirectoryHandle
{
  CFileItemList itemList;
  XFILE::IDirectory* pDirectory;
};

static ExternalDirectoryHandle* Exfsys_GetDirectory(const char* path)
{
  ExternalDirectoryHandle* handle = new ExternalDirectoryHandle;

  CStdString strPath = path;

  handle->pDirectory = CDirectoryFactory::Create(strPath);

  if( !handle->pDirectory )
  {
    delete handle;
    return NULL;
  }

  if( !handle->pDirectory->GetDirectory(strPath, handle->itemList) )
  {
    delete handle->pDirectory;
    delete handle;
    return NULL;
  }

  return handle;
}

static int Exfsys_GetDirectoryItemCount(ExternalDirectoryHandle* handle)
{
  if( handle )
    return handle->itemList.Size();

  return 0;
}

static const char* Exfsys_GetDirectoryItemLabel(ExternalDirectoryHandle* handle, int index)
{
  static CStdString strEmpty;

  if( handle )
  {
    CFileItemPtr item = handle->itemList.Get(index);

    if( item )
      return item->GetLabel().c_str();
  }

  return strEmpty.c_str();
}

static const char* Exfsys_GetDirectoryItemAttr(ExternalDirectoryHandle* handle, int index, bool* bIsFolder)
{
  static CStdString strEmpty;

  *bIsFolder = false;

  if( handle )
  {
    CFileItemPtr item = handle->itemList.Get(index);

    if( item )
    {
      *bIsFolder = item->m_bIsFolder;
      return item->GetPath().c_str();
    }
  }

  return strEmpty.c_str();
}

static void Exfsys_CloseGetDirectory(ExternalDirectoryHandle* handle)
{
  if( handle )
  {
    if( handle->pDirectory )
      delete handle->pDirectory;

    delete handle;
  }
}

static bool Exfsys_CreateDirectory(const char* path)
{
  CStdString strPath = path;
  CStdString realPath = URIUtils::SubstitutePath(strPath);

  std::auto_ptr<IDirectory> pDirectory(CDirectoryFactory::Create(realPath));

  if (pDirectory.get())
    return pDirectory->Create(realPath.c_str());

  return false;
}

static bool Exfsys_ExistsDirectory(const char* path)
{
  CStdString strPath = path;
  CStdString realPath = URIUtils::SubstitutePath(strPath);

  std::auto_ptr<IDirectory> pDirectory(CDirectoryFactory::Create(realPath));

  if (pDirectory.get())
    return pDirectory->Exists(realPath.c_str());

  return false;
}

static bool Exfsys_RemoveDirectory(const char* path)
{
  CStdString strPath = path;
  CStdString realPath = URIUtils::SubstitutePath(strPath);

  std::auto_ptr<IDirectory> pDirectory(CDirectoryFactory::Create(realPath));

  if (pDirectory.get())
    return pDirectory->Exists(realPath.c_str());

  return false;
}

static bool Exfsys_IsAllowedDirectory(const char* filePath)
{
  return true;
}

void CVDMPlayer::LoadExternalFileSystem( PlayercoreConfig* pConfig )
{
#if defined(__ANDROID_ALLWINNER__) || defined(TARGET_WINDOWS)

  static ExternalFilesystem exfsys;

  exfsys.Open = Exfsys_Open;
  exfsys.Close = Exfsys_Close;
  exfsys.Seek = Exfsys_Seek;
  exfsys.Read = Exfsys_Read;
  exfsys.Stat = Exfsys_Stat;
  exfsys.Truncate = Exfsys_Truncate;
  exfsys.GetLength = Exfsys_GetLength;
  exfsys.GetPosition = Exfsys_GetPosition;
  exfsys.GetChunkSize = Exfsys_GetChunkSize;
  exfsys.Exists = Exfsys_Exists;
  exfsys.StatEx = Exfsys_StatEx;

  exfsys.GetDirectory = Exfsys_GetDirectory;
  exfsys.GetDirectoryItemCount = Exfsys_GetDirectoryItemCount;
  exfsys.GetDirectoryItemLabel = Exfsys_GetDirectoryItemLabel;
  exfsys.GetDirectoryItemAttr = Exfsys_GetDirectoryItemAttr;
  exfsys.CloseGetDirectory = Exfsys_CloseGetDirectory;

  exfsys.CreateDirectory = Exfsys_CreateDirectory;
  exfsys.ExistsDirectory = Exfsys_ExistsDirectory;
  exfsys.RemoveDirectory = Exfsys_RemoveDirectory;
  exfsys.IsAllowedDirectory = Exfsys_IsAllowedDirectory;

  pConfig->externalFilesystem = &exfsys;

#endif
}

/*
VS_2D_DISPLAY_THE_ORIGINAL=0,
VS_2D_DISPLAY_THE_LEFT_HALF=1,
VS_2D_DISPLAY_THE_UPPER_HALF=2,
VS_3D_LEFT_RIGHT=3,
VS_3D_TOM_BOTTOM=4,
VS_3D_INTERLACING=5,
VS_3D_LINE_INTERLACING=6,
VS_3D_COLUMN_INTERLACING=7,
VS_2D3D_MAX

*/
void CVDMPlayer::SetPlayMode(DIMENSIONMODE mode)
{
#if defined(__ANDROID_ALLWINNER__)
  if(VidOnMe::VDMUtils::GetCPUType() != VidOnMe::CT_ALLWINNER_A20)
  {
  switch (mode)
  {
  case VS_2D_DISPLAY_THE_ORIGINAL:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_2D_FULL);
    break;

  case VS_2D_DISPLAY_THE_LEFT_HALF:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_2D_LEFT);
    break;

  case VS_2D_DISPLAY_THE_UPPER_HALF:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_2D_RIGHT);
    break;

  case VS_3D_LEFT_RIGHT:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_3D_LEFT_RIGHT);
    break;

  case VS_3D_TOM_BOTTOM:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_3D_TOP_BOTTOM);
    break;

  case VS_3D_INTERLACING:
    m_pPlayer->SetAllWinnerDisplayMode (ALLWINNER_DISPLAYMODE_3D_DUAL_STREAM);
    break;

    default:
      break; 
    }
  }
#endif
}

#endif //__HAS_VIDONME_PLAYER__
