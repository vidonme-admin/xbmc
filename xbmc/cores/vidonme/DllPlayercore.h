#ifndef __DLL_PLAYERCORE__
#define __DLL_PLAYERCORE__

#include "system.h"
#include "DynamicDll.h"
#include "libplayercore.h"
#include "libplayercore_xsource.h"

class DllPlayercoreInterface
{
public:

	virtual ~DllPlayercoreInterface() {};

	virtual bool Libplayercore_Init( PlayercoreConfig *pConfig )=0;
	virtual void Libplayercore_DeInit()=0;
	virtual CCorePlayer* Libplayercore_GetCorePlayer( ICorePlayerCallback *pCallback )=0;
	virtual void Libplayercore_ReleaseCorePlayer( CCorePlayer*& pPlayer )=0;
	virtual void Libplayercore_GetVersion( int &major,int &minor )=0;
	virtual void Libplayercore_RefreshHardwareAccelSetting(int nmode) = 0;
	virtual bool LibPlayercore_SetAudioOutputDevice( int deviceIndex ) = 0;
	virtual bool LibPlayercore_SetAudioDefaultOutputDevice( const char* pszDeviceName, int types) = 0;
	virtual bool LibPlayercore_SetAudioOutputUpmixStereo( bool bUpmix ) = 0;
	virtual bool LibPlayercore_SetAudioOutputSpeakers( int speaker ) = 0;
	virtual bool LibPlayercore_SetAudioPassthroughCodec( int supportCodec ) = 0;
	virtual AudioSetting* LibPlayercore_GetAudioSettings() = 0;
#if defined(__ANDROID_ALLWINNER__)
	virtual void Libplayercore_SetANativeActivity (ANativeActivity *nativeActivity)=0;
	virtual void Libplayercore_SetANativeWindow (ANativeWindow *nativeWindow)=0;
	virtual void Libplayercore_SetExternalRenderManager (void *manager)=0;
        virtual void Libplayercore_SetExternalMatrices (void *matrices)=0;
  virtual bool LibPlayercore_SetAudioHdmiPassthroughType(int mode)=0;
#endif
  virtual bool Libplayercore_SetAppSettings(AppSettings* pSettings) = 0;
  virtual	bool LibPlayercore_AddPlaySource( const char* pszFile, int type ) = 0; 
  virtual void LibPlayercore_RemovePlaySource( const char* pszFile, int type ) = 0;
  virtual void LibPlayercore_RemovePlaySourceAll() = 0;
  virtual XSource::PlaylistPtrs& LibPlayercore_GetPlaySourcePlaylist( const char* pszFile ) = 0;
  virtual const char* LibPlayercore_GetPlaySourceImage( const char* pszFile ) = 0;
  virtual void LibPlayercore_GetPlaySourceInformation( const char* pszFile ) = 0;
  virtual int LibPlayercore_GetPlaySourcePlaylistCount( const char* pszFile ) = 0;
  virtual XSource::CPlaylist* LibPlayercore_GetPlaySourcePlaylistByIndex( const char* pszFile, int index ) = 0;
  virtual XSource::CPlaylist* LibPlayercore_GetPlaySourceMainmoviePlaylist( const char* pszFile ) = 0;
  virtual const char* LibPlayercore_GetPlaySourceMeta( const char* pszFile, const char* pszKey ) = 0;
  virtual bool Libplayercore_GetPlaySourceThumb(const char* src, const char* strTarget, int width, int height, int time, int playlist = -1) = 0;
  virtual bool Libplayercore_RefreshFontSetting(PlayercoreConfig* pConfig) = 0;
  virtual void LibPlayercore_SetSettingBool(SettingeKey key, bool bValue) = 0;
  virtual void Libplayercore_SetSettingString(SettingeKey key, const char* pszValue) = 0;
  virtual void Libplayercore_SetSettingInt(SettingeKey key, int iValue) = 0;
  virtual void Libplayercore_SetSettingFloat(SettingeKey key, float fValue) = 0;

  virtual void Libplayercore_SetBDRegion(int BDRegion) = 0;
};

class DllPlayercore : public DllDynamic, DllPlayercoreInterface
{
	DECLARE_DLL_WRAPPER(DllPlayercore, "")

		DEFINE_METHOD1(bool,					Libplayercore_Init, (PlayercoreConfig *p1))
		DEFINE_METHOD0(void,					Libplayercore_DeInit)
		DEFINE_METHOD1(CCorePlayer*,	Libplayercore_GetCorePlayer, (ICorePlayerCallback *p1))
		DEFINE_METHOD1(void,					Libplayercore_ReleaseCorePlayer, (CCorePlayer*& p1))
		DEFINE_METHOD2(void,					Libplayercore_GetVersion, (int &p1, int &p2))
		DEFINE_METHOD1(void,					Libplayercore_RefreshHardwareAccelSetting, (int p1))
		DEFINE_METHOD2(bool,					LibPlayercore_SetAudioDefaultOutputDevice, (const char* p1, int p2))
		DEFINE_METHOD1(bool,					LibPlayercore_SetAudioOutputUpmixStereo,(bool p1))
		DEFINE_METHOD1(bool,					LibPlayercore_SetAudioOutputSpeakers, (int p1))
		DEFINE_METHOD1(bool,					LibPlayercore_SetAudioPassthroughCodec, (int p1))
		DEFINE_METHOD0(AudioSetting*,	LibPlayercore_GetAudioSettings)
		DEFINE_METHOD1(bool,					LibPlayercore_SetAudioOutputDevice, (int p1))
#if defined(__ANDROID_ALLWINNER__)
		DEFINE_METHOD1(void,          Libplayercore_SetANativeActivity, (ANativeActivity *p1))
		DEFINE_METHOD1(void,          Libplayercore_SetANativeWindow, (ANativeWindow *p1))
		DEFINE_METHOD1(void,          Libplayercore_SetExternalRenderManager, (void *p1))
		DEFINE_METHOD1(void,          Libplayercore_SetExternalMatrices, (void *p1))
    DEFINE_METHOD1(bool,          LibPlayercore_SetAudioHdmiPassthroughType, (int p1))
#endif
    DEFINE_METHOD1(bool,					Libplayercore_SetAppSettings, (AppSettings* p1))
    DEFINE_METHOD2(bool,					LibPlayercore_AddPlaySource, (const char* p1, int p2))
    DEFINE_METHOD2(void,					LibPlayercore_RemovePlaySource, (const char* p1, int p2))
    DEFINE_METHOD0(void,          LibPlayercore_RemovePlaySourceAll)
    DEFINE_METHOD1(XSource::PlaylistPtrs&,	LibPlayercore_GetPlaySourcePlaylist, (const char* p1))
    DEFINE_METHOD1(const char*,		LibPlayercore_GetPlaySourceImage, (const char* p1))
    DEFINE_METHOD1(void,					LibPlayercore_GetPlaySourceInformation, (const char* p1))
    DEFINE_METHOD1(int,					  LibPlayercore_GetPlaySourcePlaylistCount, (const char* p1))
    DEFINE_METHOD2(XSource::CPlaylist*,		LibPlayercore_GetPlaySourcePlaylistByIndex, (const char* p1, int p2))
    DEFINE_METHOD1(XSource::CPlaylist*,		LibPlayercore_GetPlaySourceMainmoviePlaylist, (const char* p1))
    DEFINE_METHOD2(const char*,		LibPlayercore_GetPlaySourceMeta, (const char* p1, const char* p2))
    DEFINE_METHOD6(bool,					Libplayercore_GetPlaySourceThumb, (const char* p1, const char* p2, int p3, int p4, int p5, int p6))
    DEFINE_METHOD1(bool,					Libplayercore_RefreshFontSetting, (PlayercoreConfig* p1))

    DEFINE_METHOD2(void,		LibPlayercore_SetSettingBool, (SettingeKey p1, bool p2))
    DEFINE_METHOD2(void,		Libplayercore_SetSettingString, (SettingeKey p1, const char* p2))
    DEFINE_METHOD2(void,		Libplayercore_SetSettingInt, (SettingeKey p1, int p2))
    DEFINE_METHOD2(void,		Libplayercore_SetSettingFloat, (SettingeKey p1, float p2))

    DEFINE_METHOD1(void,					Libplayercore_SetBDRegion, (int p1))

		BEGIN_METHOD_RESOLVE()
		RESOLVE_METHOD(Libplayercore_Init)
		RESOLVE_METHOD(Libplayercore_DeInit)
		RESOLVE_METHOD(Libplayercore_GetCorePlayer)
		RESOLVE_METHOD(Libplayercore_ReleaseCorePlayer)
		RESOLVE_METHOD(Libplayercore_GetVersion)
		RESOLVE_METHOD(Libplayercore_RefreshHardwareAccelSetting)

		RESOLVE_METHOD(LibPlayercore_SetAudioOutputDevice)
		RESOLVE_METHOD(LibPlayercore_SetAudioDefaultOutputDevice)
		RESOLVE_METHOD(LibPlayercore_SetAudioOutputUpmixStereo)
		RESOLVE_METHOD(LibPlayercore_SetAudioOutputSpeakers)
		RESOLVE_METHOD(LibPlayercore_SetAudioPassthroughCodec)
		RESOLVE_METHOD(LibPlayercore_GetAudioSettings)

#if defined(__ANDROID_ALLWINNER__)
		RESOLVE_METHOD(Libplayercore_SetANativeActivity)
		RESOLVE_METHOD(Libplayercore_SetANativeWindow)
		RESOLVE_METHOD(Libplayercore_SetExternalRenderManager)
		RESOLVE_METHOD(Libplayercore_SetExternalMatrices)
    RESOLVE_METHOD(LibPlayercore_SetAudioHdmiPassthroughType)
#endif

    RESOLVE_METHOD(Libplayercore_SetAppSettings)
    RESOLVE_METHOD(LibPlayercore_AddPlaySource)
    RESOLVE_METHOD(LibPlayercore_RemovePlaySource)
    RESOLVE_METHOD(LibPlayercore_RemovePlaySourceAll)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourcePlaylist)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourceImage)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourceInformation)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourcePlaylistCount)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourcePlaylistByIndex)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourceMainmoviePlaylist)
    RESOLVE_METHOD(LibPlayercore_GetPlaySourceMeta)
    RESOLVE_METHOD(Libplayercore_GetPlaySourceThumb)
    RESOLVE_METHOD(Libplayercore_RefreshFontSetting)

    RESOLVE_METHOD(LibPlayercore_SetSettingBool)
    RESOLVE_METHOD(Libplayercore_SetSettingString)
    RESOLVE_METHOD(Libplayercore_SetSettingInt)
    RESOLVE_METHOD(Libplayercore_SetSettingFloat)

    RESOLVE_METHOD(Libplayercore_SetBDRegion)

		END_METHOD_RESOLVE()


public:    
    virtual void Unload() {}
};

#endif //__DLL_PLAYERCORE__
