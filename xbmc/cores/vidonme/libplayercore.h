#ifndef __LIBPLAYERCORE_H__
#define __LIBPLAYERCORE_H__

#if defined(__TARGET_ANDROID_ALLWINNER__)
#include <android/native_window_jni.h>
#include <android/native_activity.h>
#endif

#include "libplayercore_iplayer.h"

#ifdef DLL_LIBPLAYERCORE_EXPORTS
#define	DLL_LIBPLAYERCORE_API __declspec(dllexport)
#else
#define DLL_LIBPLAYERCORE_API
#endif


#if !defined(TARGET_DARWIN_IOS)
#ifdef __cplusplus
extern "C" {
#endif
#endif

//==========================================================================================================================//
// Indexs
// A.Basic datastruct
// B.Basic Library API 
// C.Audio settings API
// D.Disc playing API
// E.XSource support API
// F.Video settings API
// G.3D playing API
// H.Common settings API
// I.Register information API


#define PLAYERCORE_VERSION "1.3"

// changelog
//
// version  1.3 
// time:    20140215
// author:  zhang, sp ...
// changelog:
// 
// 
// version  1.2
// time:    20130911
// author:  zhang
// changelog:
//
// * add ExternalFileSystem support, user need set PlayercoreConfig->externalFilesystem,
//   using outer filesystem to support smb / upnp ...
// * add api LibPlayercore_RemovePlaySourceAll , which support remove all add source at once
//
//playercore support two working mode
//1. default: it's full playing engine
//2. plugin:  it's the plugin of xbmc, xbmc using specail code to call this player, and provide some resource


//==========================================================================================================================//
//A.Basic datastruct

enum
{
    PLAYERCORE_MODE_DEFAULT,
    PLAYERCORE_MODE_PLUGIN_PLAYER,
};

enum
{
    PLAYERCORE_FONT_STYLE_NORMAL = 0,
    PLAYERCORE_FONT_STYLE_BOLD,
    PLAYERCORE_FONT_STYLE_ITALICS,
};

enum
{
    PLAYERCORE_ALIGN_MANUAL = 0,
    PLAYERCORE_ALIGN_BOTTOM_INSIDE,
    PLAYERCORE_ALIGN_BOTTOM_OUTSIDE,
    PLAYERCORE_ALIGN_TOP_INSIDE,
    PLAYERCORE_ALIGN_TOP_OUTSIDE
};

enum
{
    PLAYERCORE_LOG_APP_DISABLE    = -1,         // all log output disable
    PLAYERCORE_LOG_APP_NOLIMITED  = 0,          // all log will be output
    PLAYERCORE_LOG_APP_COMMON     = 1,          // only log will be output
    PLAYERCORE_LOG_APP_VIDONME    = 3,          // only VIDONME log will be output
};

#if defined(TARGET_WINDOWS)
//windows have the define in windef.h
#else
typedef unsigned int COLORREF;
#endif

#define RGBA(r,g,b,a)          ((COLORREF)((((unsigned char)(b)|((unsigned short)((unsigned char)(g))<<8))|(((unsigned int)(unsigned char)(r))<<16)))) | ((unsigned int)((unsigned int)(a) << 24))





//log callback, loglevel = xbmc loglevel
typedef void (*PFN_LOG_CALLBACK)(int loglevel, const char* pstr, int console_id);

//real_log, using for show realtime log
typedef void (*PFN_REAL_LOG_CALLBACK)(int loglevel, const char* pstr);

#if defined(__TARGET_ANDROID_ALLWINNER__)
typedef int GLint;
typedef GLint(*PFN_OUTER_GUISHADER_CALLBACK)(int param1, int param2);
typedef int (*PFN_OUTER_GUISETTING_GETINT_CALLBACK)(const char* strSetting);
typedef int (*PFN_OUTER_SETDISPLAY3DMODE_CALLBACK)(int type);
#endif

//set mouse status callback, coming from bluray-hdmv menu
typedef void (*PFN_MOUSE_STATUS_CALLBACK)(bool);


//outer filesystem interface

#ifndef int64_t
typedef long long int64_t;
#endif

struct ExternalFileHandle;
struct ExternalDirectoryHandle;

typedef struct stExternalFilesystem
{

    //file interface

    ExternalFileHandle* (*Open)(const char* path);
    void (*Close)(ExternalFileHandle* h);

    int64_t (*Seek)(ExternalFileHandle* h, int64_t iFilePosition, int iWhence);
    unsigned int (*Read)(ExternalFileHandle* h, void* lpBuf, int64_t uiBufSize, int flags);
    int (*Stat)(ExternalFileHandle* h, struct __stat64* buffer);
    int (*Truncate)(ExternalFileHandle* h, int64_t size);
    int64_t (*GetLength)(ExternalFileHandle* h);
    int64_t (*GetPosition)(ExternalFileHandle* h);
    int (*GetChunkSize)(ExternalFileHandle* h);

    bool (*Exists)(const char* path);
    int (*StatEx)(const char* path, struct __stat64* buffer);


    //directory interface

    ExternalDirectoryHandle* (*GetDirectory)(const char* path);
    int (*GetDirectoryItemCount)(ExternalDirectoryHandle* handle);
    const char* (*GetDirectoryItemLabel)(ExternalDirectoryHandle* handle, int index);
    const char* (*GetDirectoryItemAttr)(ExternalDirectoryHandle* handle, int index, bool* bIsFolder);
    void (*CloseGetDirectory)(ExternalDirectoryHandle* handle);


    bool (*CreateDirectory)(const char* path);
    bool (*ExistsDirectory)(const char* path);
    bool (*RemoveDirectory)(const char* path);
    bool (*IsAllowedDirectory)(const char* filePath);

} ExternalFilesystem;


//playercore config

typedef struct stPlayercoreConfig
{
    stPlayercoreConfig()
    {
        pfn_log_callback = NULL;
        pfn_real_log_callback = NULL;

        fontsize = 30;
        fontdistance = 5;
        bfontborder = true;
        fontstyle = PLAYERCORE_FONT_STYLE_NORMAL;
        fontcolor = RGBA(255, 255, 0, 255);
        BorderColor = RGBA(255, 255, 255, 255);
        fontposition = PLAYERCORE_ALIGN_BOTTOM_OUTSIDE;
        strcpy(charset, "GB18030");
        strcpy(fontpath, "default");
#if defined (_USRDLL)
        bEnableConsoleLogger = false;
        log_app = PLAYERCORE_LOG_APP_NOLIMITED;
#else
        bEnableConsoleLogger = false;
        log_app = PLAYERCORE_LOG_APP_DISABLE;
#endif

        pfn_mouse_status = NULL;
        externalFilesystem = NULL;
        
		//user information 
        pszUserName       = NULL;
        pszUserPassword   = NULL;
        pszAppVersion     = NULL;

    }
    //playercore using which working mode
    int working_mode;

    //specify path
    //MAX_PATH
    char pszHomePath[255];				//root folder, is the same with .exe, using this path to resolve the bin-path
    char pszUserFolderPath[255];		//log, other userdata will be put this file, for windows : common is ApplicationData / App
    char pszTempFolderPath[255];		//temp file folder

    //outer window handle, for windows is HWND, TODO: mac,ios,android
    void* outerWindowHandle;

    //start window position. size
    int width;
    int height;

    //log callback , check __LOG_CALLBACK__ will find all changed code
    PFN_LOG_CALLBACK		pfn_log_callback;
    PFN_REAL_LOG_CALLBACK	pfn_real_log_callback;

#if defined(__TARGET_ANDROID_ALLWINNER__)
    PFN_OUTER_GUISHADER_CALLBACK pfn_guishader_callback;
    PFN_OUTER_GUISETTING_GETINT_CALLBACK pfn_guisetting_getint_callback;
#endif

    //hardware accelerate mask, check __HARDWARE_ACCELERATE_CONTROL__ will find changed code
#define DISABLE_HARDWARE_ACCELERATE             0x0000
#define ENABLE_HARDWARE_ACCELERATE_DXVA         0x0001
#define ENABLE_HARDWARE_ACCELEARTE_VDA          0x0002

    int	 hardwareAccelerateMask;

    //using win32, maybe some client not define TARGET_WINDOWS
#if defined (_WIN32)
    void* d3d9;
    void* d3d9device;
#endif

    int   fontsize;
    int   fontstyle;
    int   fontdistance;
    unsigned int fontcolor;
    unsigned int BorderColor;
    int   fontposition;
    char  charset[255];
    char  fontpath[255];
    bool  bfontborder;

    //option for enable/disable console logger, only valid in windows/debug
    bool	bEnableConsoleLogger;
    int  log_app;

    //mouse status callback
    PFN_MOUSE_STATUS_CALLBACK pfn_mouse_status;

    //ExternalFile filesystem support
    ExternalFilesystem* externalFilesystem;

    //user information 

    const char* pszUserName;
    const char* pszUserPassword;
    const char* pszAppVersion;


} PlayercoreConfig;

















//==========================================================================================================================//
//B.Basic library API


//Get Dll version
DLL_LIBPLAYERCORE_API void Libplayercore_GetVersion(int& major, int& minor);
DLL_LIBPLAYERCORE_API const char* Libplayercore_GetVersionString();


#define INIT_FLAG_PATH		0x0010
#define INIT_FLAG_UI		0x0011
#define INIT_FLAG_ALL		0x0012

#if defined(TARGET_WINDOWS)
DLL_LIBPLAYERCORE_API bool Libplayercore_InitEx(PlayercoreConfig* pConfig, int flag);
#endif

//Init function, only need be called once, call it before any other function

DLL_LIBPLAYERCORE_API bool Libplayercore_Init(PlayercoreConfig* pConfig);

//Release function, only call once, call it before app exit

DLL_LIBPLAYERCORE_API void Libplayercore_DeInit();

//Get Player object

DLL_LIBPLAYERCORE_API CCorePlayer* Libplayercore_GetCorePlayer(ICorePlayerCallback* callback);

//Release Object

DLL_LIBPLAYERCORE_API void Libplayercore_ReleaseCorePlayer(CCorePlayer*& pPlayer);

#if defined (TARGET_DARWIN_IOS) || defined(TARGET_WINDOWS)
DLL_LIBPLAYERCORE_API bool Libplayercore_ExtractThumb(const char* strPath, const char* strTarget, IPlayerOptions* pOptions, int nwidth, int ntime);
#endif

#if defined(TARGET_WINDOWS)
DLL_LIBPLAYERCORE_API HBITMAP Libplayercore_GetThumb(char* src, int Width, int Height, int* MediaLength, int Time, int playlist = -1);
#endif

//API for get playlist from DVD / Bluray
//iso/folder/disc param input to get bluray/dvd playlist

//param request: is the same as CCorePlayer::OpenFile

DLL_LIBPLAYERCORE_API bool Libplayercore_RefreshFontSetting(PlayercoreConfig* pConfig);
DLL_LIBPLAYERCORE_API void Libplayercore_RefreshHardwareAccelSetting(int nmode);

#if defined (__TARGET_ANDROID_ALLWINNER__)

//ANDROID_ALLWINNER API

DLL_LIBPLAYERCORE_API bool Libplayercore_SetANativeActivity_Internal(ANativeActivity* nativeActivity);
DLL_LIBPLAYERCORE_API bool Libplayercore_SetANativeWindow_Internal(ANativeWindow* nativeWindow);
DLL_LIBPLAYERCORE_API bool Libplayercore_SetExternalRenderManager_Internal(void* renderManager);
DLL_LIBPLAYERCORE_API bool Libplayercore_Init_Internal(PlayercoreConfig* pConfig);
DLL_LIBPLAYERCORE_API void Libplayercore_SetExternalMatrices_Internal (void* matrices);

//Release function, only call once, call it before app exit

DLL_LIBPLAYERCORE_API void Libplayercore_DeInit_Internal();

//Get Player object

DLL_LIBPLAYERCORE_API CCorePlayer* Libplayercore_GetCorePlayer_Internal(ICorePlayerCallback* callback);

//Release Object

DLL_LIBPLAYERCORE_API void Libplayercore_ReleaseCorePlayer_Internal(CCorePlayer*& pPlayer);
DLL_LIBPLAYERCORE_API void Libplayercore_GetVersion_Internal(int& major, int& minor);

#endif //__TARGET_ANDROID_ALLWINNER__





















//==========================================================================================================================//
//C.Audio settings API

//__LIBPLAYERCORE_AUDIO_SETTINGS__
//zhang: 20130527
//add different settings function for playercore

enum
{
    AUDIO_OUT_TYPE_DECIED_BY_DEVICE     = 0x0000,
    AUDIO_OUT_TYPE_ANALOG		        = 0x0001,							//	Analog Output
    AUDIO_OUT_TYPE_DIGITAL		        = 0x0002,							//	Spdif / coax
    AUDIO_OUT_TYPE_HDMI			        = 0x0004,							//  hdmi
    AUDIO_OUT_TYPE_DISPLAYPORT	        = 0x0008,							//	displayport
};

enum
{
    AUDIO_CH_LAYOUT_INVALID = -1,

    AUDIO_CH_LAYOUT_1_0 = 0,
    AUDIO_CH_LAYOUT_2_0,
    AUDIO_CH_LAYOUT_2_1,
    AUDIO_CH_LAYOUT_3_0,
    AUDIO_CH_LAYOUT_3_1,
    AUDIO_CH_LAYOUT_4_0,
    AUDIO_CH_LAYOUT_4_1,
    AUDIO_CH_LAYOUT_5_0,
    AUDIO_CH_LAYOUT_5_1,
    AUDIO_CH_LAYOUT_7_0,
    AUDIO_CH_LAYOUT_7_1,

    AUDIO_CH_LAYOUT_MAX
};

enum
{
    PASSTHROUGH_CODEC_NONE			= 0x0000,
    PASSTHROUGH_CODEC_AC3				= 0x0001,
    PASSTHROUGH_CODEC_DTS				= 0x0002,
    PASSTHROUGH_CODEC_AAC				= 0x0004,
    PASSTHROUGH_CODEC_LPCM			= 0x0008,
    PASSTHROUGH_CODEC_TRUEHD		= 0x0010,
    PASSTHROUGH_CODEC_DTSHD			= 0x0020,
};

enum
{
    AUDIO_SINK_NONE,
    AUDIO_SINK_DIRECTSOUND,
    AUDIO_SINK_WASAPI,
};

struct AudioDevice
{
    int			outType;
    char*		pszDeviceName;				//device name : using deviceName can sure which device is our selected and using
    char*		pszDisplayDeviceName;
};

//change to big value to support more device
#define MAX_DEVICES 40

struct AudioSetting
{
    AudioSetting()
    {
        supportOutTypes = AUDIO_OUT_TYPE_ANALOG;
        defaultDeviceIndex = -1;
        deviceCounts = 0;

        stereoToAll = false;
        speakers = AUDIO_CH_LAYOUT_INVALID;
        supportPassCodec = PASSTHROUGH_CODEC_NONE;
    };

    //basic information

    int	supportOutTypes;
    int defaultDeviceIndex;
    int deviceCounts;
    AudioDevice* pDevices[MAX_DEVICES];

    //global setting

    bool	stereoToAll;						// stereo to all speaker
    int		speakers;								// specify the speaker

    int		supportPassCodec;				// outer receiver support which type codec to passthrough

};

//Get AudioSetting data, which need be called after Libplayercore_Init()
//the AudioSetting data is only can be used, cannot change it
//the pointer for AudioSetting will be released when call Libplayercore_Deinit()
DLL_LIBPLAYERCORE_API AudioSetting* LibPlayercore_GetAudioSettings();

//set specify audio output device, the index is coming from AudioSetting
//param: deviceIndex = index of AudioDevice in AudioSetting

//param: outType, in macosx, one device maybe support different outType, so also need specify it
#if defined(TARGET_DARWIN_OSX)
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioOutputDevice(int deviceIndex, int outType);
#else
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioOutputDevice(int deviceIndex);
#endif

//set history device name, this function can be called before GetAudioSettings
//param: pszDeviceName, history saved default device name
//param: types: windows/macosx others always = DEVICE_TYPE_ALL,
//							box can using this set common/passthrough device with different device

enum
{
    DEVICE_TYPE_COMMON,
    DEVICE_TYPE_PASSTHROUGH,
    DEVICE_TYPE_ALL,
};

#if defined(TARGET_DARWIN_OSX)
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioDefaultOutputDevice(const char* pszDeviceName, int outType, int types = DEVICE_TYPE_ALL);
#else
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioDefaultOutputDevice(const char* pszDeviceName, int types = DEVICE_TYPE_ALL);
#endif

DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioOutputUpmixStereo(bool bUpmix);
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioOutputSpeakers(int speaker);
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioPassthroughCodec(int supportCodec);

#if defined (__TARGET_ANDROID_ALLWINNER__)^M
DLL_LIBPLAYERCORE_API bool LibPlayercore_SetAudioHdmiPassthroughType(int hdmitype);
#endif









//==========================================================================================================================//
//D.playing API

//zhang: 20131225
//add dynamic change region interface
DLL_LIBPLAYERCORE_API void Libplayercore_SetBDRegion(int BDRegion);
DLL_LIBPLAYERCORE_API void Libplayercore_SetDVDRegion(int DVDRegion);










//==========================================================================================================================//
//E.XSource support API

//zhang: 20130602
//manager for playing source, add, remove, get playing source inforamtion()
//when enable __LIBPLAYERCORE_XSOURCE__, enable this function
// 

namespace XSource
{
    class CPlaylist;
    typedef std::vector<CPlaylist*> PlaylistPtrs;
}


//all function need the param: path,type, internal will using this param to query source

//add play source to manager,
//some information about this source will be saved, until the RemovePlaySource function be called
//default OpenFile() will call this automatically
//when add source to playlist and not playing, also can call this function
DLL_LIBPLAYERCORE_API	bool LibPlayercore_AddPlaySource(const char* pszFile, int type);

//when removePlaySource be called, all data about this source will be removed
//include disc-handle, iso-handle, thumbnail information
//: disc-source has been eject, need call this
//: source has been remove from playlist, need call this
//: for vdmplayer, if just using to parser media, after finished, need call this
DLL_LIBPLAYERCORE_API void LibPlayercore_RemovePlaySource(const char* pszFile, int type);

//function for clean all holding source
DLL_LIBPLAYERCORE_API void LibPlayercore_RemovePlaySourceAll();


DLL_LIBPLAYERCORE_API XSource::PlaylistPtrs& LibPlayercore_GetPlaySourcePlaylist(const char* pszFile);

//get disc image for bluray(dvd,iso,folder)
//input param is the source path, which must be the same as
DLL_LIBPLAYERCORE_API const char* LibPlayercore_GetPlaySourceImage(const char* pszFile);

//get source information, include medai-info / protection ...
DLL_LIBPLAYERCORE_API void LibPlayercore_GetPlaySourceInformation(const char* pszFile);

//get playlist counts
DLL_LIBPLAYERCORE_API int LibPlayercore_GetPlaySourcePlaylistCount(const char* pszFile);

//get CPlaylist, param is the index: 0 -> playlist_count - 1
DLL_LIBPLAYERCORE_API XSource::CPlaylist* LibPlayercore_GetPlaySourcePlaylistByIndex(const char* pszFile, int index);

//get longest / mainmovie playlist
DLL_LIBPLAYERCORE_API XSource::CPlaylist* LibPlayercore_GetPlaySourceMainmoviePlaylist(const char* pszFile);

//get major meta information

//support [key : value:]
//  FileFormat            : dvd, bluray
//  VideoCodec            : mpeg1, mpeg2, vc1, h264
//  VideoResolution       : 480i, 576i, 480p, 1080i, 720p, 1080p, 576p
//
//  AudioChannel          : 2.0, 5.1, 7.1 ....
//  AudioCodec            : mp1, mp2, lpcm, ac3, dts, truehd, e-ac3, dtshd, dtshd-ma
//  AudioLanguage
//
//  SubtitleCodec         : pgs, igs
//  SubLanguage
DLL_LIBPLAYERCORE_API const char* LibPlayercore_GetPlaySourceMeta(const char* pszFile, const char* pszKey);

//create thumbnail
DLL_LIBPLAYERCORE_API bool Libplayercore_GetPlaySourceThumb(const char* src, const char* strTarget, int width, int height, int time, int playlist = -1);






















//==========================================================================================================================//
//F.Video settings API

//20130723: spy: video accelerate support, current only enable in windows

enum
{
    VIDEO_CODEC_NONE	= 0x0000,
    VIDEO_CODEC_H264	= 0x0001,
    VIDEO_CODEC_VC1		= 0x0002,
    VIDEO_CODEC_MPEG2	= 0x0004,
    /*
    VIDEO_CODEC_DISABLE_H264	= 0x0008,
    VIDEO_CODEC_DISABLE_VC1		= 0x0010,
    VIDEO_CODEC_DISABLE_MPEG2	= 0x0020,
    */
};

enum
{
    VIDEO_ACCELERATION_NONE	 = 0x0000,
    VIDEO_ACCELERATION_DXVA	 = 0x0001,
    VIDEO_ACCELERATION_INTEL = 0x0002,
    VIDEO_ACCELERATION_CUDA	 = 0x0004,
};


struct VideoDevice
{
    VideoDevice()
    {
        nperformance	= VIDEO_CODEC_NONE;
        nacceledevice	= VIDEO_ACCELERATION_NONE;
    };
    int			nperformance;
    int			nacceledevice;
};

struct VideoSetting
{
    VideoSetting()
    {
        devicecounts = 0;
    };

    int devicecounts;
    VideoDevice devices[MAX_DEVICES];
};

DLL_LIBPLAYERCORE_API VideoSetting* LibPlayercore_GetVideoAccelerationSettings();
DLL_LIBPLAYERCORE_API void			LibPlayercore_SetVideoAccelerationSettings(int codec, int nacceletation);
















//==========================================================================================================================//
//G.3D playing API

typedef struct C3DConfig
{
    int  sourceType;
    int  playMode;
} C3DConfig;

// 3D Device type
enum
{
    DEVICE3D_NONE,
    DEVICE3D_INTEL,
    DEVICE3D_AMD,
    DEVICE3D_NVIDIA,
#if defined(__TARGET_ANDROID_ALLWINNER__)
    DEVICE3D_ALLWINNER
#endif
};

// 3D source type
enum
{
    SOURCE_3D_TYPE_MONO,                            // video is not stereo

    SOURCE_3D_TYPE_UNKNOWN,                         // unknown 3d format, need user select it

    SOURCE_3D_TYPE_ANAGLYPH_CYAN_RED,               // All frames are in anaglyph format viewable through red-cyan filters
    SOURCE_3D_TYPE_GREEN_MAGENTA,                   // All frames are in anaglyph format viewable through green-magenta filters

    SOURCE_3D_TYPE_MVC,                             // MVC encode, include left / right frame in individual stream

    SOURCE_3D_TYPE_SBS_LEFT_RIGHT,                  // Both views are arranged side by side, Left-eye view is on the left
    SOURCE_3D_TYPE_SBS_RIGHT_LEFT,                  // Both views are arranged side by side, Right-eye view is on the left

    SOURCE_3D_TYPE_TOP_BOTTOM,                       // Both views are arranged in top-bottom orientation, Left-eye view is on top
    SOURCE_3D_TYPE_BOTTOM_TOP,                       // Both views are arranged in top-bottom orientation, Left-eye view is at bottom

    SOURCE_3D_TYPE_CHECKBOARD_LEFT_RIGHT,           // Each view is arranged in a checkerboard interleaved pattern, Left-eye view being first
    SOURCE_3D_TYPE_CHECKBOARD_RIGHT_LEFT,           // Each view is arranged in a checkerboard interleaved pattern, Right-eye view being first

    SOURCE_3D_TYPE_INTERLEAVED_ROW_LEFT_RIGHT,      // Each view is constituted by a row based interleaving, Right-eye view is first row 
    SOURCE_3D_TYPE_INTERLEAVED_ROW_RIGHT_LEFT,      // Each view is constituted by a row based interleaving, Left-eye view is first row
    SOURCE_3D_TYPE_INTERLEAVED_COL_RIGHT_LEFT,      // Both views are arranged in a column based interleaving manner, Right-eye view is first column
    SOURCE_3D_TYPE_INTERLEAVED_COL_LEFT_RIGHT,      // Both views are arranged in a column based interleaving manner, Left-eye view is first column

    SOURCE_3D_TYPE_BLOCK_LEFT_RIGHT,                // Both eyes laced in one Block, Left-eye view is first
    SOURCE_3D_TYPE_BLOCK_RIGHT_LEFT,                // Both eyes laced in one Block, Right-eye view is first

    SOURCE_3D_TYPE_INDIVIDUAL_STREAM,               // different stream in one file
    SOURCE_3D_TYPE_INDIVIDUAL_FILE,                 // two different file : unsupport it now
};

// 3D order

enum 
{
    SOURCE_3D_ORDER_LEFT_BASE,  
    SOURCE_3D_ORDER_RIGHT_BASE,
};

// 3D playing mode
// just setting information, render need using this value to decide working mode

#define PLAYING_MODE_3D_MONO                      0x1<<0// drop 3D part using 2D method to playing 

#define PLAYING_MODE_3D_STEREO                    0x1<<1// playing by left / right frame
#define PLAYING_MODE_3D_SBS                       0x1<<2// playing it side by side
#define PLAYING_MODE_3D_TOP_BOTTOM                0x1<<3// playing it over under

#define PLAYING_MODE_3D_RED_CYAN_MONOCHROME       0x1<<4// Red/cyan anaglyph monochrome method
#define PLAYING_MODE_3D_RED_CYAN_HALF_COLOR       0x1<<5// Red/cyan anaglyph half color method
#define PLAYING_MODE_3D_RED_CYAN_FULL_COLOR       0x1<<6// Red/cyan anaglyph full color method
#define PLAYING_MODE_3D_RED_CYAN_DUBOIS           0x1<<7// Red/cyan anaglyph high quality Dubois method
#define PLAYING_MODE_3D_GREEN_MAGENTA_HALF_COLOR  0x1<<8// Green/magenta anaglyph half color method
#define PLAYING_MODE_3D_GREEN_MAGENTA_FULL_COLOR  0x1<<9// Green/magenta anaglyph full color method
#define PLAYING_MODE_3D_GREEN_MAGENTA_DUBOIS      0x1<<10// Green/magenta anaglyph high quality Dubois method
#define PLAYING_MODE_3D_AMBER_BLUE_MONOCHROME     0x1<<11// Amber/blue anaglyph monochrome method
#define PLAYING_MODE_3D_AMBER_BLUE_HALF_COLOR     0x1<<12// Amber/blue anaglyph half color method
#define PLAYING_MODE_3D_AMBER_BLUE_FULL_COLOR     0x1<<13// Amber/blue anaglyph full color method
#define PLAYING_MODE_3D_AMBER_BLUE_DUBOIS         0x1<<14// Amber/blue anaglyph high quality Dubois method
#define PLAYING_MODE_3D_RED_GREEN_MONOCHROME      0x1<<15// Red/green anaglyph monochrome method
#define PLAYING_MODE_3D_RED_BLUE_MONOCHROME       0x1<<16// Red/blue anaglyph monochrome method

//////////////////////////////////////////////////////////////////////////
#if defined(__LIBPLAYCORE_3D_SUPPORT__)

//detect stereo mode by string from file name
int Libplayercore_DetectStereoModeByString( std::string &needle );

//TODO, detect the display card
//TODO, detect current screen is support 3D playing

//detect whether monitor support 3d display
bool Libplayercore_Detect3DMonitor();

//enable / disable 3D function
void Libplayercore_Set3DGlobalEn(bool bEnable);


#endif //__LIBPLAYCORE_3D_SUPPORT__














//==========================================================================================================================//
//H.Common settings API

enum SettingeKey
{
    //////////////////////////////////////////////////////////
    //basic setting 

    BASIC_SETTING_NONE                              = 0x0000,

    //dvd auto menu, skip to introduction to menu
    BA_DVD_AUTOMENU,

    //////////////////////////////////////////////////////////
    //advance setting 
    
    ADVANCED_NONE                                   = 0x1000,



    //////////////////////////////////////////////////////////
    //custom setting 
    
    CUSTOMSIZE_SETTING_NONE                         = 0x2000,

    //audio, sub logic
    CS_DEFAULT_AUDIO_LANGUAGE,
    CS_DEFAULT_SUBTITLE_LANGUAGE,   
    CS_SUBTITLE_SEARCH_PATH,

    //smb interface
	CS_SAMBA_PASSWORD,
	CS_SAMBA_USERNAME,

};

DLL_LIBPLAYERCORE_API void LibPlayercore_SetSettingBool(SettingeKey key, bool bValue);
DLL_LIBPLAYERCORE_API void Libplayercore_SetSettingString(SettingeKey key, const char* pszValue);
DLL_LIBPLAYERCORE_API void Libplayercore_SetSettingInt(SettingeKey key, int iValue);
DLL_LIBPLAYERCORE_API void Libplayercore_SetSettingFloat(SettingeKey key, float fValue);










//==========================================================================================================================//
// I.Register information API

//which type function is be allowed 
//app can use this to do some check
//not need set by app, it's set by playercore

#define FUNCTION_PLAYING_FILE         0x0001
#define FUNCTION_PLAYING_DVD          0x0002
#define FUNCTION_PLAYING_BLURAY       0x0004
#define FUNCTION_PLAYING_BLURAY_MENU  0x0008

struct RegisterFeedback
{
  int allowFunctionEnumValues;      
};


#if !defined(TARGET_DARWIN_IOS)
#ifdef __cplusplus
}
#endif
#endif

//the other player action all can pass CCorePlayer to implement it

#endif //__LIBPLAYERCORE_H__
