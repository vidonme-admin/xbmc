#include "TheJSONRPCTest.h"
#include "network/ProxyTCPClient.h"

#include "settings/AdvancedSettings.h"
#include "utils/JSONVariantWriter.h"
#include "utils/log.h"
#include "URL.h"

using namespace JSONRPC;

class CTheJSONRPCEventTest : public IProxyJSONRPCEvent
{
public:
    virtual void OnNotification(
        const std::string& typeName,
        const std::string& method, 
        const std::string& sender,
        const CVariant& data)
    {
        std::string strData = CJSONVariantWriter::Write(data, g_advancedSettings.m_jsonOutputCompact);
        CLog::Log(LOGERROR, "%s: typename(%s), method(%s), sender(%s), data(%s)", __FUNCTION__, 
            typeName.c_str(), method.c_str(), sender.c_str(), strData.c_str());
    }

    virtual void OnNotify(
        const SessionClientID sessionClientId,
        const NotifyKind kind, 
        const NotifyEvent event,
        const CStdString& arg1,
        const CStdString& arg2,
        const CStdString& arg3)
    {
        CLog::Log(999, "%s: sessionClientId(%d), kind(%s), event(%s), arg1(%s), arg2(%s), arg3(%s)", 
            __FUNCTION__, 
            sessionClientId,
            TranslateNotifyKind(kind).c_str(),
            TranslateNotifyEvent(event).c_str(),
            arg1.c_str(), 
            arg2.c_str(), 
            arg3.c_str());
    }
    virtual void OnCallback(
        const SessionClientID sessionClientId,
        const SessionActionID sessionActionId, 
        const SessionActionEvent event,
        const CStdString& arg1,
        const CStdString& arg2,
        const CStdString& arg3)
    {
        CLog::Log(999, "%s: sessionClientId(%d), sessionActionId(%d), event(%s), arg1(%s), arg2(%s), arg3(%s)", 
            __FUNCTION__, 
            sessionClientId,
            sessionActionId, 
            TranslateSessionActionEvent(event).c_str(),
            arg1.c_str(), 
            arg2.c_str(), 
            arg3.c_str());
    }
};

CTheJSONRPCTest *CTheJSONRPCTest::ServerInstance = NULL;

bool CTheJSONRPCTest::StartServer( void )
{
    StopServer(true);

    ServerInstance = new CTheJSONRPCTest("CTheJSONRPCTest");
    ServerInstance->Create();
    return true;
}

void CTheJSONRPCTest::StopServer(bool bWait)
{
    if (ServerInstance)
    {
        ServerInstance->StopThread(bWait);
        if (bWait)
        {
            delete ServerInstance;
            ServerInstance = NULL;
        }
    }
}

void CTheJSONRPCTest::Process()
{
    m_bStop = false;

    //通信方式
    CProxyTCPClient client;
    client.Initialize();
    client.ContentTCP("127.0.0.1", 9090);

    CProxyTransportLayerClient transportLayer(&client);
    client.SetClientEvent(&transportLayer);

    //请求对象
    CTheJSONRPC rpc("127.0.0.1", 8050, &transportLayer);
    transportLayer.SetTransportLayerEvent(&rpc);

    CTheJSONRPCEventTest rpcEventTest;
    rpc.SetJSONRPCEvent(&rpcEventTest);

#pragma region JSONRPC
    while(false)
    {
        bool ret;
        CTheJSONRPC::FileNodes fileNodes;
        bool bRet = rpc.GetDirectory("c:\\", "", ret, fileNodes);
        assert(bRet);
        assert(ret);
        assert(fileNodes.size()>0);
    }

    while(false)
    {
        //"\"enum\": [ \"programs\", \"myprograms\", \"files\", \"music\", \"video\", \"pictures\" ]"
        bool ret;
        CTheJSONRPC::Shares shares;
        bool bRet = rpc.GetShares("files", ret, shares);
        assert(bRet);
        assert(ret);

    }

    while(false)
    {
        //"\"enum\": [ \"programs\", \"myprograms\", \"files\", \"music\", \"video\", \"pictures\" ]"
        const CStdString shareName = "shareName";
        CTheJSONRPC::Paths paths;
        paths.push_back("D:/test-dev/transformers/");

        bool ret;
        CStdString newname;
        CStdString sharePath;

        bool bRet = rpc.AddShare("video", shareName, paths, ret, newname, sharePath);
        assert(bRet);
        assert(ret);

    }

    while(false)
    {
        //"\"enum\": [ \"programs\", \"myprograms\", \"files\", \"music\", \"video\", \"pictures\" ]"
        const CStdString shareOldName = "shareName";
        const CStdString shareName = "shareName_99";
        CTheJSONRPC::Paths paths;
        paths.push_back("D:/test-dev/transformers");

        bool ret;
        CStdString newname;
        CStdString sharePath;

        bool bRet = rpc.UpdateShare("video", shareOldName, shareName, paths, ret, newname, sharePath);
        assert(bRet);
        assert(ret);

    }

    while(false)
    {
        //"\"enum\": [ \"programs\", \"myprograms\", \"files\", \"music\", \"video\", \"pictures\" ]"
        const CStdString shareName = "shareName_99";

        bool ret;
        bool bRet = rpc.DeleteShare("video", shareName, ret);
        assert(bRet);
        assert(ret);

    }

    while(false)
    {
        bool ret;
        CTheJSONRPC::Scrapers scrapers;
        bool bRet = rpc.GetScrapers(CONTENT_MOVIES, ret, scrapers);
        assert(bRet);
        assert(ret);
        assert(scrapers.size()>0);
    }


    while(false)
    {
        const CStdString directory = "d:\\videotest\\";
        const CStdString scraperId = "metadata.themoviedb.org";
        CTheJSONRPC::VideoScanSettings scanSettings={};
        //scanSettings.exclude = true;
        //scanSettings.noupdate = true;
        //scanSettings.parent_name = true;
        //scanSettings.parent_name_root = true;
        scanSettings.recurse = 2147483647;


        bool ret;
        bool bRet = rpc.SetVideoScraperForPath(directory, scraperId, "", scanSettings, ret);
        assert(bRet);

        assert(ret);
    }

    while (false)
    {
        const CStdString directory = "D:/test-dev/transformers/";

        CONTENT_TYPE content;
        CStdString scraperId;
        CStdString scraperSettings;
        CTheJSONRPC::VideoScanSettings scanSettings;
        bool foundDirectly;
        bool bRet = rpc.GetVideoScraperForPath(directory,
            scraperId, scraperSettings, 
            scanSettings, foundDirectly);
        assert(bRet);
    }

    while(false)
    {
        const CStdString directory = "d:\\videotest\\";
        bool bRet = rpc.StartVideoScan(directory, true);
        assert(bRet);
    }

    while(false)
    {
        bool bRet = rpc.StopVideoScan();
        assert(bRet);
    }

    while(false)
    {
        bool ret;
        bool bRet = rpc.IsVideoScanning(ret);
        assert(bRet);
    }

    while(false)
    {
        bool bRet = rpc.StartVideoCleanup();
        assert(bRet);
    }

    while (false)
    {
        bool ret;
        CTheJSONRPC::SystemSettings settings;
        bool bRet = rpc.GetSystemSettingForAll(ret, settings);
        assert(bRet);
    }

    while (false)
    {
        const CTheJSONRPC::SystemSettingKey key = "locale.language";

        bool ret;
        CTheJSONRPC::SystemSettingVal val;
        bool bRet = rpc.GetSystemSetting( key, ret, val);
        assert(bRet);
    }


    while (false)
    {
        bool ret;
        const CTheJSONRPC::SystemSettingKey key = "locale.language";
        const CTheJSONRPC::SystemSettingVal val = "English (US)";
        bool bRet = rpc.SetSystemSetting( key, val, ret);
        assert(bRet);
    }

    while (false)
    {
        bool ret;
        CTheJSONRPC::Languages languages;
        const CTheJSONRPC::SystemSettingVal val = "English (US)";
        bool bRet = rpc.FindLanguages(ret, languages);
        assert(bRet);
        assert(ret);
    }

    while (false)
    {
        CStdString strFileName = "D:\\IMG_0661.JPG";
        CStdString strFileNameThumb = CTheJSONRPC::TransformSizeForThumb(strFileName, "thumb&sss=cc");
        CStdString strFileNameXXX = CTheJSONRPC::TransformSizeForThumb(strFileNameThumb, "XXX");
        strFileName = strFileNameThumb + strFileNameXXX;
    }


#pragma endregion JSONRPC


#pragma region ProxyVideoLibrary
    CTheVideoLibrary videolibrary(&rpc);
    while (false)
    {
        CTheVideoLibrary::BaseInfoList countries;
        const bool bRet = videolibrary.GetCountries(CTheVideoLibrary::Movie, countries);
        assert(bRet);
    }

    while (false)
    {
        CTheVideoLibrary::BaseInfoList studios;
        const bool bRet = videolibrary.GetStudios(CTheVideoLibrary::Movie, studios);
        assert(bRet);
    }
    while (false)
    {
        CTheVideoLibrary::BaseInfoList directors;
        const bool bRet = videolibrary.GetDirectors(CTheVideoLibrary::Movie, directors);
        assert(bRet);
    }
    while (false)
    {
        CTheVideoLibrary::BaseInfoList years;
        const bool bRet = videolibrary.GetYears(CTheVideoLibrary::Movie, years);
        assert(bRet);
    }
    while (false)
    {
        CTheVideoLibrary::BaseInfoList actors;
        const bool bRet = videolibrary.GetActors(CTheVideoLibrary::Movie, actors);
        assert(bRet);
    }
    while (false)
    {
        CTheVideoLibrary::BaseInfoList tags;
        const bool bRet = videolibrary.GetTags(CTheVideoLibrary::Movie, tags);
        assert(bRet);
    }
    while (false)
    {
        CTheVideoLibrary::BaseInfoList albums;
        const bool bRet = videolibrary.GetMusicVideoAlbums(albums);
        assert(bRet);
    }

#pragma endregion ProxyVideoLibrary

#pragma region OTHER

    while(false)
    {
        const CStdString directory = "D:\\test-dev\\movies\\";
        const CStdString scraperId = "metadata.themoviedb.org";
        CTheJSONRPC::VideoScanSettings scanSettings={};
        scanSettings.recurse = INT_MAX;

        bool bRet;
        bool ret;
        bRet = rpc.SetVideoScraperForPath(directory, scraperId, "", scanSettings, ret);        
        bRet = rpc.StartVideoScan(directory, true);
    }

    while(false)
    {
        const CStdString directory = "D:\\test-dev\\tvshows\\";
        const CStdString scraperId = "metadata.tvdb.com";
        CTheJSONRPC::VideoScanSettings scanSettings={};
        scanSettings.recurse = INT_MAX;

        bool bRet;
        bool ret;
        bRet = rpc.SetVideoScraperForPath(directory, scraperId, "", scanSettings, ret);        
        bRet = rpc.StartVideoScan(directory, true);
    }


    while(false)
    {
        bool bRet;
        bool ret;

        CTheJSONRPC::Paths paths;
        paths.push_back("D:\\test-dev\\privvideos\\");

        CStdString newname;
        CStdString sharepath;
        bRet = rpc.AddShare("video", "privvideo", paths, ret, newname, sharepath);

        const CStdString scraperId = "metadata.privvideos.goland";
        CTheJSONRPC::VideoScanSettings scanSettings={};
        scanSettings.recurse = INT_MAX;
        bRet = rpc.SetVideoScraperForPath(sharepath, scraperId, "", scanSettings, ret);        
        bRet = rpc.StartVideoScan(sharepath, true);


        rpc.DeleteShare("video", newname, ret);
    }

    while(false)
    {
        CTheVideoLibrary::PrivVideoInfoList items;
        videolibrary.GetPrivVideos(items);
        for (unsigned int index = 0; index < items.size(); index ++)
        {
            const CTheVideoLibrary::PrivVideoInfo&  item = items[index];
            CLog::Log(LOGDEBUG, "id:%i\t file:%s", item.privVideoId, item.file.c_str());
        }
    }

    while (false)
    {
        /* "Y:\\BD\\H264\\2012.iso" 

        "Play main title: 1"
        bluray://Y%3a%5cBD%5cH264%5c2012.iso/BDMV/PLAYLIST/00001.mpls

        "Select from all titles ..."
        bluray://Y%3a%5cBD%5cH264%5c2012.iso/titles/

        "Show bluray menus"
        bluray://Y%3a%5cBD%5cH264%5c2012.iso/BDMV/MovieObject.bdmv

        */
        const CStdString file = "Y:\\BD\\H264\\2012.iso";
        CURL url("bluray://");
        url.SetHostName(file);

        bool ret;
        CTheJSONRPC::FileNodes fileNodes;
        bool bRet = rpc.GetDirectory(url.Get(), "", ret, fileNodes);
        assert(bRet);

        url.SetFileName("titles/");
        fileNodes.clear();
        bRet = rpc.GetDirectory(url.Get(), "", ret, fileNodes);
        assert(bRet);
    }


    while (false)
    {
        CTheJSONRPC::Paths paths;
        paths.push_back("d:\\test-dev\\movies\\");

        CStdString newname;
        CStdString sharepath;
        bool ret;
        rpc.AddShare("video", "movies", paths, ret, newname,  sharepath);

        CTheJSONRPC::VideoScanSettings scanSettings;
        scanSettings.exclude = false;
        scanSettings.noupdate = false;
        scanSettings.parent_name = false;
        scanSettings.parent_name_root = false;
        scanSettings.recurse = INT_MAX;
        rpc.SetVideoScraperForPath(sharepath, "metadata.themoviedb.org", "", scanSettings, ret);

        rpc.StartVideoScan(sharepath, true);
    }

    while (false)
    {
        CTheJSONRPC::Paths paths;
        paths.push_back("D:\\pics\\");

        CStdString newname;
        CStdString sharepath;
        bool ret;
        rpc.AddShare("pictures", "pictures", paths, ret, newname,  sharepath);



        
    }


#pragma endregion OTHER


#pragma region SessionClient
    //while(false)
    {
        const CStdString directory = "D:\\test-dev\\movies\\";
        const CStdString scraperId = "metadata.themoviedb.org";
        CTheJSONRPC::VideoScanSettings scanSettings={};
        scanSettings.recurse = INT_MAX;

        bool ret;
        SessionResult result;
        SessionClientID sessionClientId;
        ret = rpc.CreateSessionClient(result, sessionClientId);
        assert(ret);
        assert(result == srOk);
        
        rpc.SessionClientSubscribe(result, sessionClientId, nkTest1);
        assert(result == srOk);

        rpc.SessionClientConnect(result, sessionClientId);
        assert(result == srOk);


        while (true)
        {
            Sleep(500);
        }

        rpc.SessionClientSubscribe(result, sessionClientId, nkTest2);
        assert(result == srOk);

        while (true)
        {
            Sleep(500);
        }

        rpc.SessionClientUnsubscribe(result, sessionClientId, nkTest1);
        assert(result == srOk);

        while (true)
        {
            Sleep(500);
        }

        rpc.SessionClientDisconnect(result, sessionClientId);
        assert(result == srOk);

        while (true)
        {
            Sleep(500);
        }

        ret = rpc.DestroySessionClient(result, sessionClientId);
        assert(ret);
        assert(result == srOk);

        while (true)
        {
            Sleep(500);
        }
    }



#pragma endregion SessionClient

    /*
        rpc.GetMusicScraperForPath();
        rpc.SetMusicScraperForPath();
        rpc.StartMusicScan();
        rpc.StartMusicArtistScan();
        rpc.StartMusicAlbumScan();
        rpc.StopMusicScan();
        rpc.IsMusicScanning();
    */

    rpc.SetJSONRPCEvent(NULL);
    transportLayer.SetTransportLayerEvent(NULL);
    client.SetClientEvent(NULL);

    if (!client.connected())
        client.Disconnect();

    client.Deinitialize();
}

