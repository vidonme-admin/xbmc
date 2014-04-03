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

#include "ProxyJSONRPC.h"

namespace JSONRPC
{
    //VidOnMeProxy
    class CTheJSONRPC : public IProxyTransportLayerEvent
    {

        IProxyJSONRPCEvent* m_JSONRPCEvent;
        IProxyTransportLayer* m_transportLayer;
        CStdString m_host;
        int m_port;

    public:

        CTheJSONRPC(
            const CStdString& host,
            const int port,
            IProxyTransportLayer* const transportLayer)
            : m_host(host)
            , m_port(port)
            , m_JSONRPCEvent(NULL)
            , m_transportLayer(transportLayer)
        {
            assert(transportLayer);
        }

        IProxyJSONRPCEvent* JSONRPCEvent( void ) const { return m_JSONRPCEvent; }
        void SetJSONRPCEvent( IProxyJSONRPCEvent* const JSONRPCEvent) {m_JSONRPCEvent = JSONRPCEvent;}
       
        /*** OVERRIDE ***/
        virtual void OnNotification(
            const CVariant& notification);

        bool MethodCall(
            const std::string& method, 
            const CVariant& params, 
            CVariant& result);

        CStdString GetThumbUrl(const CStdString& thumb);
        static CStdString TransformSizeForThumb(const CStdString& thumb, const CStdString& size = "thumb");
        CStdString GetFileUrl(const CStdString& file);
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
        bool GetDirectory(
            const CStdString& directory,
            const CStdString& mask,
            OUT bool& ret,
            OUT FileNodes& fileNodes);
    public:
        typedef CStdString Path;
        typedef std::vector<Path> Paths;

        struct Share
        {
            CStdString name;
            CStdString sharePath;
            Paths paths;
        };
        typedef std::vector<Share> Shares;

        bool GetShares(
            const CStdString& type,
            OUT bool& ret,
            OUT Shares& shares);

        bool AddShare(
            const CStdString& type,
            const CStdString& name,
            const Paths& paths,
            OUT bool& ret,
            OUT CStdString& newname,
            OUT CStdString& sharePath);
        bool UpdateShare(
            const CStdString& type,
            const CStdString& oldname,
            const CStdString& name,
            const Paths& paths,
            OUT bool& ret,
            OUT CStdString& newname,
            OUT CStdString& sharePath);
        bool DeleteShare(
            const CStdString& type,
            const CStdString& name,
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

        bool GetScrapers(
            const CONTENT_TYPE content,
            OUT bool& ret,
            OUT Scrapers& scrapers);
        bool SetVideoScraperForPath(
            const CStdString& directory,
            const CStdString& scraperId, 
            const CStdString& scraperSettings,
            const VideoScanSettings& scanSettings,
            OUT bool& ret);
        bool SetMusicScraperForPath(
            const CStdString& directory,
            const CStdString& scraperId, 
            const CStdString& scraperSettings,
            OUT bool& ret);
        bool GetVideoScraperForPath(
            const CStdString& directory, 
            OUT CStdString& scraperId, 
            OUT CStdString& scraperSettings,
            OUT VideoScanSettings& scanSettings,
            OUT bool& foundDirectly);
        bool GetMusicScraperForPath(
            const CStdString& directory, 
            const CONTENT_TYPE content,
            OUT bool& ret,
            OUT CStdString& scraperId, 
            OUT CStdString& scraperSettings);
        bool StartVideoScan(
            const CStdString &directory, 
            const bool scanAll);
        bool StopVideoScan();
        bool IsVideoScanning(OUT bool& ret);
        bool StartVideoCleanup();
        bool StartMusicScan(
            const CStdString &directory, 
            const int flags = 0);
        bool StartMusicAlbumScan(
            const CStdString& directory, 
            const bool refresh);
        bool StartMusicArtistScan(
            const CStdString& directory, 
            const bool refresh);
        bool StopMusicScan();
        bool IsMusicScanning(OUT bool& ret);
    public:
        typedef CStdString SystemSettingKey;
        typedef CStdString SystemSettingVal;
        typedef std::map<SystemSettingKey, SystemSettingVal> SystemSettings;
        bool GetSystemSettingForAll(
            OUT bool& ret,
            OUT SystemSettings& systemSettings);
        bool GetSystemSetting(
            const SystemSettingKey& key,
            OUT bool & ret,
            OUT SystemSettingVal& val);
        bool SetSystemSetting(
            const SystemSettingKey& key,
            const SystemSettingVal& val,
            OUT bool & ret);
    public:
        struct Language
        {
            CStdString language;
        };
        typedef std::vector<Language> Languages;
        bool FindLanguages(
            OUT bool& ret,
            OUT Languages& languages);

    public:
        bool ping();

	public:
		struct MovieActor
		{
			std::string			strName;
			std::string			strRole;
		};

		struct MovieResume
		{
			int		nPosition;
			int		nTotal;
		};

		struct ServerMovieInfo
		{
			std::string			strFilePath;
			std::string			strPlot;
			std::string			strThumbnai;
			std::string			strTitle;
			std::string			strYear;
			MovieResume			stMovieResume;
			std::vector<MovieActor>		vecMovieActors;
			std::vector<std::string>	vecWriters;
			std::vector<std::string>	vecDirectors;
			std::vector<std::string>	vecCountries;
			std::vector<std::string>	vecStudios;
		};

		bool GetAllMovies(std::vector<ServerMovieInfo>& vecServerMoviesInfo);
    public:
        bool CreateSessionClient(OUT SessionResult& result, OUT SessionClientID& sessionClientId);
        bool DestroySessionClient(OUT SessionResult& result, const SessionClientID sessionClientId);
        bool SessionClientSubscribe(OUT SessionResult& result, const SessionClientID sessionClientId, const NotifyKind kind);
        bool SessionClientUnsubscribe(OUT SessionResult& result, const SessionClientID sessionClientId, const NotifyKind kind);
        bool SessionClientConnect(OUT SessionResult& result, const SessionClientID sessionClientId);
        bool SessionClientDisconnect(OUT SessionResult& result, const SessionClientID sessionClientId);

    };


    class CTheVideoLibrary
    {
    public:
        CTheVideoLibrary(
            CTheJSONRPC* const proxy)
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
            PrivVideo,
        };
        typedef std::vector<BaseInfo> BaseInfoList;
        void ToInfoList(const CVariant& varItems, const char* const idname, OUT BaseInfoList& items);
        const char* TranslateType(const BaseType type);
        // only for movies
        bool GetCountries(const BaseType type, OUT BaseInfoList& items);
        bool GetStudios(const BaseType type, OUT BaseInfoList& items);
        bool GetDirectors(const BaseType type, OUT BaseInfoList& items);
        bool GetYears(const BaseType type, OUT BaseInfoList& items);
        bool GetActors(const BaseType type, OUT BaseInfoList& items);
        bool GetTags(const BaseType type, OUT BaseInfoList& items);
        bool GetMusicVideoAlbums(OUT BaseInfoList& items);
  
    
        struct VideoStreams
        {
            struct Audio
            {
                CStdString codec;
                CStdString language;
                int channels;
            };
            typedef std::vector<Audio> AudioList;
            AudioList audioList;

            struct Video
            {
                CStdString codec;
                double aspect;
                int width;
                int height;
                int duration;
            };
            typedef std::vector<Video> VideoList;
            VideoList videoList;

            struct Subtitle
            {
                CStdString language;
            };
            typedef std::vector<Subtitle> SubtitleList;
            SubtitleList subtitleList;


        };
        struct VideoResume
        {
            double position;
            double total;
        };
        struct PrivVideoInfo
        {
            int privVideoId;
            CStdString title;           
            int playcount;              
            CStdString runtime;         
            CStdString lastplayed;      
            CStdString thumbnail;       
            CStdString strTag;          

            CStdString file;            
            CStdString dateadded;       
            VideoStreams streamDetails; 
            VideoResume resume;         
        };
        struct PrivVideoDetails
        {
            CStdString title;         
            int playcount;            
            CStdString runtime;       
            CStdString lastplayed;    
            CStdString thumbnail;     
            CStdString strTag;        
        };
        typedef std::vector<PrivVideoInfo> PrivVideoInfoList;
        void ToVideoInfo(const CVariant& varItem, OUT PrivVideoInfo& item);
        void ToVideoInfoList(const CVariant& varItems, OUT PrivVideoInfoList& items);
        bool GetPrivVideos(OUT PrivVideoInfoList& items);
        bool GetPrivVideoDetails(const int privVideoId, OUT PrivVideoInfo& info);
        bool GetRecentlyAddedPrivVideos(OUT PrivVideoInfoList& items);
        bool SetPrivVideoDetails(const int privVideoId, const PrivVideoDetails details);
        bool RemovePrivVideo(const int privVideoId);
private:
        bool MethodCall(
            const std::string& method, 
            const CVariant& params, 
            CVariant& result);
        CTheJSONRPC* m_proxy;
    };
}