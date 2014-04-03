#pragma once
/*
*      Copyright (C) 2005-2012 Team XBMC
*      http://www.xbmc.org
*
*  This Program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This Program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/

#if defined(__VIDONME_MEDIACENTER__)

#include <vector>
#include <string>
#include "utils/Variant.h"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"
#include "network/ProxyTCPClient.h"
#include "utils/StdString.h"

#if !defined(TARGET_ANDROID)
#include "SResult.h"
#endif

#include "addons/Scraper.h"

using namespace ADDON;

namespace JSONRPC
{
	typedef unsigned int JSONRPCID;
	JSONRPCID get_jsonrpc_request_id( void );

	class IProxyTransportLayerEvent
	{
	public:
		virtual void OnNotification(
			const CVariant& notification) = 0;
	};
	class IProxyTransportLayer
	{
	protected:
		virtual const char* host() const = 0;
		virtual int port( void ) const = 0 ;
	public:
		virtual bool Service(
			const CVariant& request,
			OUT CVariant& response) = 0;
	};

	class CProxyTransportLayerCurl : public IProxyTransportLayer
	{
	public:
		CProxyTransportLayerCurl(
			const CStdString& host,
			const int port)
			: m_host(host)
			, m_port(port)
		{

		}

		/*** OVERRIDE ***/
		virtual bool Service(
			const CVariant& request,
			OUT CVariant& response);
	protected:
		virtual const char* host() const;
		virtual int port( void ) const;
	private:
		CStdString m_host;
		int m_port;
	};

	class CProxyTransportLayerClient : public IProxyTransportLayer, public IProxyClientEvent
	{
	private:
		IProxyClient* m_client;
		IProxyTransportLayerEvent* m_transportLayerEvent;
	private:
		class WaitForResponse
		{
		public:
			WaitForResponse();
			bool finished( void )const {return m_finished;}
			bool disconnected( void )const{return m_disconnected;}
			const CVariant& response( void ){return m_response;}

			void SetResponse(const CVariant& response);
			void disconnect( void );

		private:
			bool m_finished;
			bool m_disconnected;
			CVariant m_response;
		};

		typedef std::map<JSONRPCID, WaitForResponse*> WaitForResponseList;
		WaitForResponseList m_waitingList;
	public:
		CProxyTransportLayerClient(
			IProxyClient* const client)
			: m_client(client)
			, m_transportLayerEvent(NULL)
		{
		}
		IProxyTransportLayerEvent* transportLayerEvent()const {return m_transportLayerEvent;}
		void SetTransportLayerEvent( IProxyTransportLayerEvent* const transportLayerEvent) {  m_transportLayerEvent = transportLayerEvent;}

		/*** OVERRIDE ***/
		virtual bool Service(
			const CVariant& request,
			OUT CVariant& response);

		/*** OVERRIDE ***/
		virtual void OnPushBuffer(const CStdString& data);
		virtual void OnDisconnect();
	protected:
		virtual const char* host() const;
		virtual int port( void ) const;
	};

	typedef unsigned int SessionActionID;
	typedef std::set<SessionActionID> SessionActionIDs;

	enum SessionActionState
	{
		sasUnknown,
		sasNone,
		sasRunning,
		sasCancelling,
		sasCancelled,
		sasOk,
		sasFailed,
	};

	CStdString TranslateSessionActionState(const SessionActionState state);
	SessionActionState TranslateSessionActionState(const CStdString& strState);


	enum SessionActionEvent
	{
		saeUnknown,
		saeRunning,
		saeCancelling,
		saeCancelled,
		saeOk,
		saeFailed,
	};

	CStdString TranslateSessionActionEvent(const SessionActionEvent event);
	SessionActionEvent TranslateSessionActionEvent(const CStdString& strEvent);


	typedef unsigned int SessionClientID;
	typedef std::set<SessionClientID> SessionClientIDs;

	enum SessionResult
	{
		srUnknown,
		srOk,

		srOverMaxSize,
		srInvalidSessionAction,
		srInvalidSessionClient,
		srDuplicateConnect,
		srNotConnected,
		srDuplicateSubscribe,
		srNotSubscribe,
		srInvalidClientConnection,
		srMistakenNotifyKind,
		srMistakenNotifyEvent,
	};
	CStdString TranslateSessionResult(SessionResult result);
	SessionResult TranslateSessionResult(const CStdString& strResult);

	enum NotifyKind
	{
		nkUnknown,
		nkTest1,
		nkTest2,
		nkUpdateVideoLibrary,		// DB updated.(useless)
		nkScanDirectory,				// Begin and end scan directory.
		nkAnalysisFile,					// Meta/PlayList completed.
		nkScraperFile,					// Movie/Video/TVshow DB change.(in use)
	};
	CStdString TranslateNotifyKind(NotifyKind kind);
	NotifyKind TranslateNotifyKind(const CStdString& strKind);

	enum NotifyEvent
	{
		neUnknown,
		neTest1,
		neTest2,
	};
	CStdString TranslateNotifyEvent(NotifyEvent event);
	NotifyEvent TranslateNotifyEvent(const CStdString& strEvent);

	class IProxyJSONRPCEvent
	{
	public:
		virtual void OnNotification(
			const std::string& typeName,
			const std::string& method, 
			const std::string& sender,
			const CVariant& data) {};
		virtual void OnNotify(
			const SessionClientID sessionClientId,
			const NotifyKind kind, 
			const NotifyEvent event,
			const CStdString& arg1,
			const CStdString& arg2,
			const CStdString& arg3) {};
		virtual void OnCallback(
			const SessionClientID sessionClientId,
			const SessionActionID sessionActionId, 
			const SessionActionEvent event,
			const CStdString& arg1,
			const CStdString& arg2,
			const CStdString& arg3 ) {};
	};


	//VidOnMeProxy
	class CProxyJSONRPC : public IProxyTransportLayerEvent
	{
		IProxyJSONRPCEvent* m_JSONRPCEvent;
		IProxyTransportLayer* m_transportLayer;
	private:
		bool MethodCallImpl(
			const std::string& method, 
			const CVariant& params, 
			CVariant& result);
	public:

		CProxyJSONRPC(
			IProxyTransportLayer* const transportLayer)
			: m_JSONRPCEvent(NULL)
			, m_transportLayer(transportLayer)
		{
			assert(transportLayer);
		}

		IProxyJSONRPCEvent* JSONRPCEvent( void ) const {return m_JSONRPCEvent;}
		void SetJSONRPCEvent( IProxyJSONRPCEvent* const JSONRPCEvent) {m_JSONRPCEvent = JSONRPCEvent;}

		/*** OVERRIDE ***/
		virtual void OnNotification(
			const CVariant& notification);

		static bool MethodCall(
			const std::string& method, 
			const CVariant& params, 
			CVariant& result);

		static CStdString GetThumbUrl(const CStdString& thumb);
		static CStdString GetFileUrl(const CStdString& thumb2);
	public:
		struct FileNode
		{
			CStdString strTitle;
			CStdString strPath;
			bool isFolder;
			int64_t dwSize;
			CStdString strDateTime;
		};
		typedef std::vector<FileNode> FileNodes;
		static bool GetDirectory(
			const CStdString& directory,
			const CStdString& mask,
			OUT bool& ret,
			OUT FileNodes& fileNodes);
	public:
		typedef CStdString Path;
		typedef std::vector<Path> Paths;

		struct Share
		{
			CStdString strType;
			CStdString type;
			CStdString name;
			CStdString sharePath;
		};
		typedef std::vector<Share> Shares;

		static bool GetShares(
			OUT bool& ret,
			OUT Shares& shares);

		static bool AddShare(
			const CStdString& type,
			const CStdString& name,
			const CStdString& path,
			OUT bool& ret);

		static bool UpdateShare(
			const CStdString& oldname,
			const CStdString& oldpath,
			const CStdString& oldtype,
			const CStdString& name,
			const CStdString& path,
			const CStdString& type,
			OUT bool& ret);

		static bool DeleteShare(
			const CStdString& type,
			const CStdString& name,
			const CStdString& path,
			OUT bool& ret);

	public:
		struct Scraper
		{
			CStdString scraperId;
			CStdString icon;
		};
		typedef std::vector<Scraper> Scrapers;

		struct VideoScanSettings
		{
			bool parent_name;       /* use the parent dirname as name of lookup */
			bool parent_name_root;  /* use the name of directory where scan started as name for files in that dir */
			int  recurse;           /* recurse into sub folders (indicate levels) */
			bool noupdate;          /* exclude from update library function */
			bool exclude;           /* exclude this path from scraping */
		};

		static bool SetVideoTypeForPath(
			const CStdString& directory,
			const CStdString& type,
			const VideoScanSettings& scanSettings,
			OUT bool& ret);
		static bool GetVideoTypeForPath(
			const CStdString& directory,
			OUT bool& ret,
			OUT CStdString& type,
			OUT VideoScanSettings& scanSettings);
		static bool EraseVideoTypeForPath(
			const CStdString& directory,
			OUT bool& ret);
		static bool GetScrapers(
			const CONTENT_TYPE content,
			OUT bool& ret,
			OUT Scrapers& scrapers);
		static bool SetVideoScraperForPath(
			const CStdString& directory,
			const CStdString& scraperId, 
			const CStdString& scraperSettings,
			const VideoScanSettings& scanSettings,
			OUT bool& ret);
		static bool SetMusicScraperForPath(
			const CStdString& directory,
			const CStdString& scraperId, 
			const CStdString& scraperSettings,
			OUT bool& ret);
		static bool GetVideoScraperForPath(
			const CStdString& directory, 
			OUT CStdString& content,
			OUT CStdString& scraperId, 
			OUT CStdString& scraperSettings,
			OUT VideoScanSettings& scanSettings,
			OUT bool& foundDirectly);
		static bool GetMusicScraperForPath(
			const CStdString& directory, 
			const CONTENT_TYPE content,
			OUT bool& ret,
			OUT CStdString& scraperId, 
			OUT CStdString& scraperSettings);
		static bool StartVideoScan(
			const CStdString &directory, 
			const bool scanAll);
		static bool StopVideoScan();
		static bool IsVideoScanning(OUT bool& ret);
		static bool StartVideoCleanup();
		static bool StartMusicScan(
			const CStdString &directory, 
			const int flags = 0);
		static bool StartMusicAlbumScan(
			const CStdString& directory, 
			const bool refresh);
		static bool StartMusicArtistScan(
			const CStdString& directory, 
			const bool refresh);
		static bool StopMusicScan();
		static bool IsMusicScanning(OUT bool& ret);
	public:
		static bool ping();

	public:
		static bool IsServerDeviceAvailable(CStdString strFilePath, bool& bExist);
		static bool IsServerFilesExists(CStdString strFilePath, bool& bExist);

		//////////////////////////////////////////////////////////////////////////
		//Movies
		struct ServerMovieInfo
		{
			std::string			strMovieID;
			std::string			strKeyID;
			std::string			strFilePath;
			std::string			strTitle;
			std::string			strThumbnail;
			std::string			strYear;
			std::vector<std::string>	vecCountries;
			std::vector<std::string>	vecGenres;
		};

		struct Actor
		{
			std::string			strName;
			std::string			strRole;
		};

		struct Resume
		{
			int		nPosition;
			int		nTotal;
		};

		struct Limits
		{
			int		nStart;
			int		nEnd;

			Limits()
			{
				nStart	= 0;
				nEnd	= -1;
			}
		};

		struct MovieSort
		{
			bool		bDescending;
			bool		bIgnoreArticle;
			std::string	strSortItem;

			MovieSort()
			{
				bIgnoreArticle	= true;
				bDescending		= false;
				strSortItem = "none";
			}
		};

		struct MovieFilter
		{
			int				nYear;
			std::string		strTitle;
			std::string		strGenre;
			std::string		strActor;
			std::string		strDirector;
			std::string		strWriter;
			std::string		strStudio;
			std::string		strCountry;
			std::string		strSet;
			std::string		strTag;

			MovieFilter()
			{
				nYear	= 0;
			}
		};

		struct ServerMovieDetail
		{
			std::string			strMovieID;
			std::string			strTitle;
			std::string			strPlot;
			std::string			strRunTime;
			std::string			strMPAA;
			Resume					stResume;
			std::vector<Actor>			vecActors;
			std::vector<std::string>	vecWriters;
			std::vector<std::string>	vecDirectors;
			std::vector<std::string>	vecCountries;
			std::vector<std::string>	vecGenres;
			std::vector<std::string>	vecStudios;
		};

		struct ServerMetaInfo
		{
			CStdString type;
			CStdString fileFormat;
			CStdString videoCodec;
			CStdString videoResolution;
			CStdString audioCodec;
			CStdString audioChannel;
			CStdString subtitleCodec;
		};

		struct ServerPlaylist
		{
			std::string			strPath;
			std::string			strFileType;
			std::string			strName;
			std::string			strRunTime; //seconds
			std::string			strThumbnail;
			bool						bIsMainTitle;
			bool            b3D;
			int             nAngles;
			ServerPlaylist()
			{
				bIsMainTitle	= false;
				b3D = false;
				nAngles = 0;
			}
		};
		typedef std::vector<ServerPlaylist> ServerPlaylists;

		static bool GetServerMovies(std::vector<ServerMovieInfo>& vecServerMoviesInfo);
		static bool GetServerMovies(const Limits& limits, const MovieSort& sort, const MovieFilter& filter, std::vector<ServerMovieInfo>& vecServerMoviesInfo);
		static bool GetServerMovieDetails(int nMovieID, ServerMovieDetail& stMovieDetails);
		static bool SetServerMovieDetails(int nMovieID, const CStdString& strLastPlayTime, double resumePosition = 0);
		static bool GetMetaFromPath(const CStdString& strPath, ServerMetaInfo& meta);
		static bool GetFileMetaFromPath(const CStdString& strPath, ServerMetaInfo& meta);
		static bool GetPlayListsFromPath(const CStdString& strPath, ServerPlaylists& playlists);
		static bool GetFilePlayListsFromPath(const CStdString& strPath, ServerPlaylists& serverPlaylists);
		static bool GetExtractThumb(const CStdString& strPath, CStdString& strThumb);
		static bool GetFileExtractThumb(const CStdString& strPath, CStdString& strThumb);
		static bool RemoveServerMovie(int nMovieID);  

		//////////////////////////////////////////////////////////////////////////
		//Music
		struct ServerMusicInfo
		{
			int				nDuration;
			int				nRating;
			int				nYear;
			int				nPlayCount;
			std::string		strTitle;
			std::string		strThumbnail;
			std::string		strFilePath;
			std::string		strLyrics;
			std::string		strLastPlayed;
			std::vector<std::string>	vecArtists;
			std::vector<std::string>	vecGenres;

			ServerMusicInfo()
			{
				nDuration	= -1;
				nRating		= -1;
				nYear		= -1;
				nPlayCount	= -1;
			}
		};

		struct MusicSort
		{
			bool		bDescending;
			bool		bIgnoreArticle;
			std::string	strSortItem;

			MusicSort()
			{
				bIgnoreArticle	= true;
				bDescending		= false;
			}
		};

		struct MusicFilter
		{
			int				nYear;
			std::string		strTitle;
			std::string		strGenre;
			std::string		strArtist;

			MusicFilter()
			{
				nYear	= 0;
			}
		};

		static bool GetServerMusics(std::vector<ServerMusicInfo>& vecServerMusicsInfo);
		static bool GetServerMusics(const Limits& limits, const MusicSort& sort, const MusicFilter& filter, std::vector<ServerMusicInfo>& vecServerMusicsInfo);

		//////////////////////////////////////////////////////////////////////////
		//TV Show
		struct TVShowEpisode 
		{
			std::string     strTVShowID;
			std::string			strEpisodeID;
			std::string			strSeasonID;
			std::string			strFileID;
			std::string			strKeyID;
			std::string			strSeason;
			std::string			strEpisode;
			std::string			strFilePath;
			std::string			strThumbnail;
			std::string			strTitle;
			std::string			strRunTime;
			std::string			strPlot;
			Resume					stResume;
			std::vector<Actor>			vecActors;
			std::vector<std::string>	vecWriters;
			std::vector<std::string>	vecDirectors;
		};

		struct TVShowSeason
		{
			std::string			strTVShowID;
			std::string			strSeasonID;
			std::string			strShowTitle;
			std::string			strThumbnail;
			std::string			strFanart;
			std::string			strSeason;
			std::string			strEpisode;
			std::string			strWatchedEpisodes;
			std::vector<TVShowEpisode>	vecEpisodes;
		};

		struct ServerTVShowInfo
		{
			std::string			strPlot;
			std::string			strMPAA;
			std::string			strThumbnail;
			std::string			strTitle;
			std::string			strYear;
			std::string			strTVShowID;
			int							nCurSeason;
			int							nCurEpisode;
			std::vector<Actor>			vecActors;
			std::vector<std::string>	vecStudios;
			std::vector<std::string>	vecGenres;

			ServerTVShowInfo()
			{
				nCurEpisode		= 0;
				nCurSeason		= 0;
			}
		};

		struct TVShowSort
		{
			bool		bDescending;
			bool		bIgnoreArticle;
			std::string	strSortItem;

			TVShowSort()
			{
				bIgnoreArticle	= true;
				bDescending		= false;
				strSortItem = "none";
			}
		};

		struct TVShowFilter
		{
			int				nYear;
			std::string		strTitle;
			std::string		strGenre;
			std::string		strActor;
			std::string		strStudio;

			TVShowFilter()
			{
				nYear	= 0;
			}
		};

		struct EpisodeDetail
		{
			Resume resume;
		};

		static bool GetServerTVShow(std::vector<ServerTVShowInfo>& vecServerTVShowInfo);
		static bool GetServerTVShow(const Limits& limits, const TVShowSort& sort, const TVShowFilter& filter, std::vector<ServerTVShowInfo>& vecServerTVShowsInfo);
		static bool GetTVShowSeasons(int nTVShowID, std::vector<TVShowSeason>& vecTVShowSeasons);
		static bool GetTVShowEpisodes(int nTVShowID, int nSeason, std::vector<TVShowEpisode>& vecTVShowEpisodes);
		static bool GetTVShowEpisodes(int nTVShowID, std::vector<TVShowSeason>& vecTVShowSeasons);
		static bool GetEpisodeDetails(int nEpisodeID, EpisodeDetail& details);
		static bool SetEpisodeDetails(int nEpisodeID, const CStdString& strLastPlayTime, double resumePosition = 0);
		static bool RemoveEpisode(int nEpisodeID);

		//////////////////////////////////////////////////////////////////////////
		//Picture
		struct ServerPictureInfo
		{
			std::string			strTitle;
			std::string			strFilePath;
			std::string			strSize;
			std::string			strFileType;
			std::string			strLastModified;
		};

		struct ServerPictureSource
		{
			std::string			strPath;
			std::string			strName;
			std::string			strFirstPicPath;
		};

		static bool GetServerPictureSources(std::vector<ServerPictureSource>& vecPicSources);
		static bool GetServerPictureSources(const Limits& limits, std::vector<ServerPictureSource>& vecPicSources);
		static bool GetServerPictures(std::string strDirectory, std::vector<ServerPictureInfo>& vecServerPictureInfo);

		//////////////////////////////////////////////////////////////////////////
		//Video
		struct ServerVideoInfo
		{
			std::string     strVideoID;
			std::string     strKeyID;
			std::string			strFilePath;
			std::string			strThumbnail;
			std::string			strTitle;
		};

		struct VideoDetail
		{
			Resume resume;
		};

		static bool GetServerVideo(std::vector<ServerVideoInfo>& vecServerVideoInfo);
		static bool GetServerVideo(const Limits& limits, std::vector<ServerVideoInfo>& vecServerVideoInfo);    
		static bool GetVideoDetails(int nVideoID, VideoDetail& details);
		static bool SetVideoDetails(int nVideoID, const CStdString& strLastPlayTime, double resumePosition = 0);
		static bool RemoveServerVideo(int nVideoID);

#if !defined(TARGET_ANDROID)
		static bool CreateSessionClient(OUT SessionClientID& sessionClientId);
		static bool DestroySessionClient(const SessionClientID sessionClientId);
		static bool SessionClientSubscribe(const SessionClientID sessionClientId, const NotifyKind kind);
		static bool SessionClientUnsubscribe(const SessionClientID sessionClientId, const NotifyKind kind);
#endif		
	};

	//deprecated
	class CProxyVideoLibrary
	{
	public:
		CProxyVideoLibrary(
			CProxyJSONRPC* const proxy)
			: m_proxy(proxy)
		{
			assert(proxy);
		}

	public:
		struct BaseInfo
		{
			int id;
			std::string label;
			std::string title;
			std::string thumbnail;
		};
		enum BaseType
		{
			Movie,
			TvShow,
			MusicVideo,
		};
		typedef std::vector<BaseInfo> BaseInfoList;
		static void ToInfoList(const CVariant& varItems, const char* const idname, OUT BaseInfoList& items);
		static const char* TranslateType(const BaseType type);
		// only for movies
		static bool GetCountries(const BaseType type, OUT BaseInfoList& items);
		static bool GetStudios(const BaseType type, OUT BaseInfoList& items);
		static bool GetDirectors(const BaseType type, OUT BaseInfoList& items);
		static bool GetYears(const BaseType type, OUT BaseInfoList& items);
		static bool GetActors(const BaseType type, OUT BaseInfoList& items);
		static bool GetTags(const BaseType type, OUT BaseInfoList& items);
		static bool GetMusicVideoAlbums(OUT BaseInfoList& items);
	private:
		static bool MethodCall(
			const std::string& method, 
			const CVariant& params, 
			CVariant& result);
		CProxyJSONRPC* m_proxy;
	};
}

#endif