#include "TheJSONRPC.h"
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#include "utils/log.h"
#include "settings/AdvancedSettings.h"

#include "filesystem/CurlFile.h"
#include "URL.h"

namespace JSONRPC
{

void CTheJSONRPC::OnNotification(const CVariant& notification)
{
    CStdString method = notification["method"].asString();
    if (method.Equals("callback"))
    {
        const SessionClientID sessionClientId = notification["params"]["sessionClientId"].asInteger();
        const SessionActionID sessionActionId = notification["params"]["sessionActionId"].asInteger();
        const SessionActionEvent event = TranslateSessionActionEvent(notification["params"]["event"].asString());
        const CStdString arg1 = notification["params"]["arg1"].asString();
        const CStdString arg2 = notification["params"]["arg2"].asString();
        const CStdString arg3 = notification["params"]["arg3"].asString();

        m_JSONRPCEvent->OnCallback(sessionClientId, sessionActionId, event, arg1, arg2, arg3 );
        return;
    }

    if (method.Equals("notify"))
    {
        const SessionClientID sessionClientId = notification["params"]["sessionClientId"].asInteger();
        const NotifyKind kind = TranslateNotifyKind(notification["params"]["kind"].asString());
        const NotifyEvent event = TranslateNotifyEvent(notification["params"]["event"].asString());
        const CStdString arg1 = notification["params"]["arg1"].asString();
        const CStdString arg2 = notification["params"]["arg2"].asString();
        const CStdString arg3 = notification["params"]["arg3"].asString();
        m_JSONRPCEvent->OnNotify(sessionClientId, kind, event, arg1, arg2, arg3);
        return;
    }
    std::string typeName; // response["method"].asString();

    const std::string sender = notification["params"]["sender"].asString();
    const CVariant& data = notification["params"]["data"];
    int sPos = method.find('.');
    if (method.npos != sPos)
    {
        typeName = method.substr(0, sPos-1);
        method.erase( method.begin() + sPos );
    }

    m_JSONRPCEvent->OnNotification(typeName, method, sender, data);
    return;
}


CStdString CTheJSONRPC::GetThumbUrl(const CStdString& thumb2)
{
    CStdString thumb(thumb2);
    if (thumb2.compare(0, 8, "image://") != 0)
    {
        CStdString strTemp(thumb2);
        CURL::Encode(strTemp);
        thumb.Format("image://%s", strTemp.c_str());
    }

    CURL::Encode(thumb);

    CStdString strURL;
    strURL.Format("http://%s:%d/image/%s", m_host.c_str(), m_port, thumb.c_str());
    return strURL;
}

CStdString CTheJSONRPC::TransformSizeForThumb(const CStdString& thumb, const CStdString& size/*="thumb"*/)
{
    CStdString imageRet;
    if (thumb.compare(0, 8, "image://") != 0)
    {
        CStdString thumbEncoded = CURL::Encode(thumb);
        imageRet.Format("image://%s/transform?size=%s", thumbEncoded.c_str(), size.c_str());
        return imageRet;
    }

    int iPosTransform = thumb.Find("/transform?", 8);
    if ( thumb.npos == iPosTransform )
    {
        imageRet.Format("%s/transform?size=%s", thumb.c_str(), size.c_str());
        return imageRet;
    }

    // image : image://.../transform?
    CStdString image = thumb.Mid(0, iPosTransform + 11);
    CStdString options = thumb.Mid(iPosTransform + 11);

    int iPosSize = options.Find("size=");
    if ( options.npos == iPosSize )
    {
        imageRet.Format("%s&ssize=%s", image.c_str(), size.c_str());
        return imageRet;
    }

    CStdString optionsLeft = options.Mid(0, iPosSize);
    imageRet.Format("%s%ssize=%s", image.c_str(), optionsLeft.c_str(), size.c_str());

    int iEndSize = options.Find("&", iPosSize);
    if ( iEndSize > 0 )
    {
        CStdString optionsRight = options.Mid(iEndSize);
        imageRet += optionsRight;
    }

    return imageRet;
}


CStdString CTheJSONRPC::GetFileUrl(const CStdString& file)
{
    CStdString strURL;
    strURL.Format("http://%s:%d/file/%s", m_host.c_str(), m_port, CURL::Encode(file));
    return strURL;
}


bool CTheJSONRPC::MethodCall(
    const std::string& method, 
    const CVariant& params, 
    CVariant& result)
{
    CVariant request;
    request["jsonrpc"] = "2.0";
    request["method"] = method;
    request["params"] = params;
    request["id"] = get_jsonrpc_request_id();

    CVariant response;
    assert(m_transportLayer);
    bool ret = m_transportLayer->Service(request, response);
    if (!ret) return false;

    ret = response.isMember("result");
    assert(ret);
    if (!ret) return false;

    result = response["result"];
    return true;


    /*
    //JSONRPC_STATUS errorCode;
    if (response.isMember("result"))
    {
        if ( response["result"].isString() 
            && response["result"] == CVariant("OK") )
        {
            result = CVariant::VariantTypeNull;
            //errorCode = ACK;
            return true;
        }
        result = response["result"];
        //errorCode = OK;
        return true;
    }

    if (response.isMember("error"))
    {
        errorCode = (JSONRPC::JSONRPC_STATUS)response["error"]["code"].asInteger();
        if ( errorCode == JSONRPC::InvalidParams && response["error"].isMember("data"))
        {
            result = response["error"]["data"];
        }
        return false;
    }
    return false;
    */
}


bool CTheJSONRPC::ping()
{
    const CVariant params = CVariant::VariantTypeObject;

    CVariant result;
    bool ret = MethodCall("playlist.getplaylists", params, result);
    if (!ret) return false;

    return true;
}



bool CTheJSONRPC::GetDirectory(
    const CStdString& directory,
    const CStdString& mask,
    OUT bool& ret,
    OUT FileNodes& fileNodes)
{
    CVariant params; 
    params["directory"] = directory.c_str();
    params["mask"] = mask.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetDirectory", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    const CVariant& varFileList = result["filelist"];
    for (unsigned int index = 0; index < varFileList.size(); index++ )
    {
        const CVariant& varFile = varFileList[index];
        FileNode node;
        node.strTitle       = varFile["title"].asString();
        node.strPath        = varFile["path"].asString();
        node.isFolder       = varFile["isFolder"].asBoolean();
        node.dwSize         = varFile["size"].asInteger();
        node.strDateTime    = varFile["datetime"].asString();
        fileNodes.push_back(node);
    }
    return true;
}


bool CTheJSONRPC::GetShares(
    const CStdString& type,
    OUT bool& ret,
    OUT Shares& shares)
{
    CVariant params;
    params["type"] = type.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetShares", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    const CVariant& varShares = result["shares"];
    for (unsigned int index = 0; index < varShares.size(); index++ )
    {
        const CVariant& varShare = varShares[index];

        Share share;
        share.name = varShare["name"].asString();
        share.sharePath = varShare["sharePath"].asString();
        Paths &paths = share.paths;

        const CVariant& varPaths = varShare["paths"];
        for (unsigned int i = 0; i < varPaths.size(); i++ )
        {
            paths.push_back(varPaths[index].asString());
        }

        shares.push_back(share);
    }
    return true;
}

bool CTheJSONRPC::AddShare(
    const CStdString& type,
    const CStdString& name,
    const Paths& paths,
    OUT bool& ret,
    OUT CStdString& newname,
    OUT CStdString& sharePath)
{
    CVariant params;
    params["type"] = type.c_str();
    params["name"] = name.c_str();
    params["paths"] = CVariant::VariantTypeArray;

    CVariant& varPaths = params["paths"];
    for ( unsigned int index = 0; index<paths.size(); index++ )
    {
        varPaths.push_back(paths[index].c_str());
    }


    CVariant result;
    bool bRet = MethodCall("VidOnMe.AddShare", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    newname = result["newname"].asString();
    sharePath = result["sharePath"].asString();
    return true;
}

bool CTheJSONRPC::UpdateShare(
    const CStdString& type,
    const CStdString& oldname,
    const CStdString& name,
    const Paths& paths,
    OUT bool& ret,
    OUT CStdString& newname,
    OUT CStdString& sharePath)
{
    CVariant params;
    params["type"] = type.c_str();
    params["oldname"] = oldname.c_str();
    params["name"] = name.c_str();
    params["paths"] = CVariant::VariantTypeArray;

    CVariant& varPaths = params["paths"];
    for ( unsigned int index = 0; index<paths.size(); index++ )
    {
        varPaths.push_back(paths[index].c_str());
    }


    CVariant result;
    bool bRet = MethodCall("VidOnMe.UpdateShare", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    newname = result["newname"].asString();
    sharePath = result["sharePath"].asString();
    return true;
}

bool CTheJSONRPC::DeleteShare(
    const CStdString& type,
    const CStdString& name,
    OUT bool& ret)
{
    CVariant params;
    params["type"] = type.c_str();
    params["name"] = name.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.DeleteShare", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    return true;
}

bool CTheJSONRPC::GetScrapers(
    const CONTENT_TYPE content,
    OUT bool& ret,
    OUT Scrapers& scrapers)
{
    CStdString strContent = TranslateContent(content);
    CVariant params;
    params["content"] = strContent.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetScrapers", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    const CVariant& varScrapers = result["scrapers"];
    for (unsigned int index = 0; index < varScrapers.size(); index++)
    {
        const CVariant& varScraper = varScrapers[index];

        Scraper scraper;
        scraper.scraperId = varScraper["scraperId"].asString();
        scraper.icon = varScraper["icon"].asString();

        scrapers.push_back(scraper);
    }

    return true;
}
bool CTheJSONRPC::SetVideoScraperForPath(
    const CStdString& directory,
    const CStdString& scraperId, 
    const CStdString& scraperSettings,
    const VideoScanSettings& scanSettings,
    OUT bool& ret)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["scraperId"] = scraperId.c_str();
    params["scraperSettings"] = scraperSettings.c_str();
    params["scanSettings"] = CVariant::VariantTypeObject;

    CVariant& varScanSettings = params["scanSettings"];
    varScanSettings["parent_name"]      = scanSettings.parent_name;
    varScanSettings["parent_name_root"] = scanSettings.parent_name_root;
    varScanSettings["recurse"]          = scanSettings.recurse;
    varScanSettings["noupdate"]         = scanSettings.noupdate;
    varScanSettings["exclude"]          = scanSettings.exclude;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.SetVideoScraperForPath", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    return true;
}

bool CTheJSONRPC::SetMusicScraperForPath(
    const CStdString& directory,
    const CStdString& scraperId, 
    const CStdString& scraperSettings,
    OUT bool& ret)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["scraperId"] = scraperId.c_str();
    params["scraperSettings"] = scraperSettings.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.SetMusicScraperForPath", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    return true;
}

bool CTheJSONRPC::GetVideoScraperForPath(
    const CStdString& directory, 
    OUT CStdString& scraperId, 
    OUT CStdString& scraperSettings,
    OUT VideoScanSettings& scanSettings,
    OUT bool& foundDirectly)
{
    CVariant params;
    params["directory"] = directory.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetVideoScraperForPath", params, result);
    if (!bRet) return false;

    scraperId       = result["scraperId"].asString();
    scraperSettings = result["scraperSettings"].asString();
    foundDirectly   = result["foundDirectly"].asBoolean();

    const CVariant& varScanSettings = result["scanSettings"];
    scanSettings.parent_name        = varScanSettings["parent_name"].asBoolean();
    scanSettings.parent_name_root   = varScanSettings["parent_name_root"].asBoolean();
    scanSettings.recurse            = (int)varScanSettings["recurse"].asInteger();
    scanSettings.noupdate           = varScanSettings["noupdate"].asBoolean();
    scanSettings.exclude            = varScanSettings["exclude"].asBoolean();

    return true;
}

bool CTheJSONRPC::GetMusicScraperForPath(
    const CStdString& directory, 
    const CONTENT_TYPE content,
    OUT bool& ret,
    OUT CStdString& scraperId, 
    OUT CStdString& scraperSettings)
{
    CStdString strContent = TranslateContent(content);
    CVariant params;
    params["directory"] = directory.c_str();
    params["content"] = strContent.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetMusicScraperForPath", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    scraperId       = result["scraperId"].asString();
    scraperSettings = result["scraperSettings"].asString();

    return true;
}
bool CTheJSONRPC::StartVideoScan(
    const CStdString &directory, 
    const bool scanAll)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["scanAll"] = scanAll;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StartVideoScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::StopVideoScan()
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StopVideoScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::IsVideoScanning(OUT bool& ret)
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.IsVideoScanning", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    return true;
}

bool CTheJSONRPC::StartVideoCleanup()
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StartVideoCleanup", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::StartMusicScan(
    const CStdString &directory, 
    const int flags/* = 0*/)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["flags"] = flags;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StartMusicScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::StartMusicAlbumScan(
    const CStdString& directory, 
    const bool refresh)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["refresh"] = refresh;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StartMusicAlbumScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::StartMusicArtistScan(
    const CStdString& directory, 
    const bool refresh)
{
    CVariant params;
    params["directory"] = directory.c_str();
    params["refresh"] = refresh;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StartMusicArtistScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::StopMusicScan()
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.StopMusicScan", params, result);
    if (!bRet) return false;

    bool ret = result["ret"].asBoolean();
    assert(ret);

    return true;
}

bool CTheJSONRPC::IsMusicScanning(OUT bool& ret)
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.IsMusicScanning", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    return true;
}

bool CTheJSONRPC::GetSystemSettingForAll(
    OUT bool& ret,
    OUT SystemSettings& systemSettings)
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetSystemSettingForAll", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    const CVariant& varSettings = result["settings"];
    for (unsigned int index = 0; index < varSettings.size(); index++)
    {
        const CVariant& varSetting = varSettings[index];

        SystemSettingKey key = varSetting["key"].asString();
        SystemSettingVal val = varSetting["val"].asString();
        systemSettings[key] = val;
    }
    return true;
}


bool CTheJSONRPC::GetSystemSetting(
    const SystemSettingKey& key,
    OUT bool & ret,
    OUT SystemSettingVal& val)
{
    CVariant params;
    params["key"] = key.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.GetSystemSetting", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    val = result["val"].asString();
    return true;

}
bool CTheJSONRPC::SetSystemSetting(
    const SystemSettingKey& key,
    const SystemSettingVal& val,
    OUT bool & ret)
{
    CVariant params;
    params["key"] = key.c_str();
    params["val"] = val.c_str();

    CVariant result;
    bool bRet = MethodCall("VidOnMe.SetSystemSetting", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    //assert(ret);

    return true;
}

bool CTheJSONRPC::FindLanguages(
    OUT bool& ret,
    OUT Languages& languages)
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant result;
    bool bRet = MethodCall("VidOnMe.FindLanguages", params, result);
    if (!bRet) return false;

    ret = result["ret"].asBoolean();
    if (!ret) return true;

    const CVariant& varLanguages = result["languages"];
    for (unsigned int index = 0; index < varLanguages.size(); index++)
    {
        const CVariant& varLanguage = varLanguages[index];

        Language language;
        language.language = varLanguage["language"].asString();

        languages.push_back(language);
    }

    return true;
}

bool CTheJSONRPC::GetAllMovies(std::vector<ServerMovieInfo>& vecServerMoviesInfo)
{
	CVariant	params;

	CVariant	properties;
	properties.push_back("cast");
	properties.push_back("country");
	properties.push_back("director");
	properties.push_back("file");
	properties.push_back("plot");
	properties.push_back("resume");
	properties.push_back("studio");
	properties.push_back("thumbnail");
	properties.push_back("title");
	properties.push_back("writer");
	properties.push_back("year");
	
	params["properties"]	= properties;

	CVariant result;
	bool bRet = MethodCall("VideoLibrary.GetMovies", params, result);
	if (!bRet) return false;

	const CVariant& varMovies = result["movies"];
	for (unsigned int index = 0; index < varMovies.size(); ++index)
	{
		ServerMovieInfo		stMovieInfo;

		const CVariant& varMovie	= varMovies[index];

		const CVariant& varActors	= varMovie["cast"];
		for (unsigned int Idx = 0; Idx < varActors.size(); ++Idx)
		{
			MovieActor		stActor;
			const CVariant& varActor	= varActors[Idx];
			
			stActor.strName		= varActor["name"].asString();
			stActor.strRole		= varActor["role"].asString();

			stMovieInfo.vecMovieActors.push_back(stActor);
		}
		
		const CVariant& varWriters	= varMovie["writer"];
		for (unsigned int Idx = 0; Idx < varWriters.size(); ++Idx)
		{
			const CVariant& varWriter	= varWriters[Idx];
			stMovieInfo.vecWriters.push_back(varWriter.asString());
		}

		const CVariant& varDirectors	= varMovie["director"];
		for (unsigned int Idx = 0; Idx < varDirectors.size(); ++Idx)
		{
			const CVariant& varDirector	= varDirectors[Idx];
			stMovieInfo.vecDirectors.push_back(varDirector.asString());
		}

		const CVariant& varCountries	= varMovie["country"];
		for (unsigned int Idx = 0; Idx < varCountries.size(); ++Idx)
		{
			const CVariant& varCountry	= varCountries[Idx];
			stMovieInfo.vecCountries.push_back(varCountry.asString());
		}

		const CVariant& varStudios	= varMovie["studio"];
		for (unsigned int Idx = 0; Idx < varStudios.size(); ++Idx)
		{
			const CVariant& varStudio	= varStudios[Idx];
			stMovieInfo.vecStudios.push_back(varStudio.asString());
		}

		const CVariant& varResume	= varMovie["resume"];
		stMovieInfo.stMovieResume.nPosition	= (int)varResume["position"].asInteger();
		stMovieInfo.stMovieResume.nTotal	= (int)varResume["total"].asInteger();

		const CVariant& varThumbnai	= varMovie["thumbnail"];
		stMovieInfo.strThumbnai	= GetThumbUrl(varThumbnai.c_str());

		stMovieInfo.strFilePath	= varMovie["file"].asString();
		stMovieInfo.strPlot		= varMovie["plot"].asString();
		stMovieInfo.strTitle	= varMovie["title"].asString();
		stMovieInfo.strYear		= varMovie["year"].asString();

		vecServerMoviesInfo.push_back(stMovieInfo);
	}

	return true;
}

bool CTheJSONRPC::CreateSessionClient(OUT SessionResult& result, OUT SessionClientID& sessionClientId)
{
    CVariant params = CVariant::VariantTypeArray;

    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.CreateSessionClient", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    if ( result != srOk ) return true;

    sessionClientId = vResult["sessionClientId"].asInteger();
    return true;
}

bool CTheJSONRPC::DestroySessionClient(OUT SessionResult& result, const SessionClientID sessionClientId)
{
    CVariant params = CVariant::VariantTypeObject;
    params["sessionClientId"] = sessionClientId;

    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.DestroySessionClient", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    return true;
}

bool CTheJSONRPC::SessionClientSubscribe(OUT SessionResult& result, const SessionClientID sessionClientId, const NotifyKind kind)
{
    CVariant params = CVariant::VariantTypeObject;
    params["sessionClientId"] = sessionClientId;
    params["kind"] = TranslateNotifyKind(kind);
    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.SessionClientSubscribe", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    return true;
}

bool CTheJSONRPC::SessionClientUnsubscribe(OUT SessionResult& result, const SessionClientID sessionClientId, const NotifyKind kind)
{
    CVariant params = CVariant::VariantTypeObject;
    params["sessionClientId"] = sessionClientId;
    params["kind"] = TranslateNotifyKind(kind);
    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.SessionClientUnsubscribe", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    return true;
}

bool CTheJSONRPC::SessionClientConnect(OUT SessionResult& result, const SessionClientID sessionClientId)
{
    CVariant params = CVariant::VariantTypeObject;
    params["sessionClientId"] = sessionClientId;
    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.SessionClientConnect", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    return true;
}

bool CTheJSONRPC::SessionClientDisconnect(OUT SessionResult& result, const SessionClientID sessionClientId)
{
    CVariant params = CVariant::VariantTypeObject;
    params["sessionClientId"] = sessionClientId;
    CVariant vResult;
    const bool ret = MethodCall("VidOnMe.SessionClientDisconnect", params, vResult);
    if (!ret) return false;

    const CStdString strResult = vResult["result"].asString();
    result = TranslateSessionResult(strResult);
    return true;
}

bool CTheVideoLibrary::MethodCall(const std::string& method, const CVariant& params, CVariant& result)
{
    return m_proxy->MethodCall("VideoLibrary." + method, params, result);
}

const char* CTheVideoLibrary::TranslateType(const BaseType type)
{
    switch (type)
    {
    case Movie: return "movie";
    case TvShow: return "tvshow";
    case MusicVideo: return "musicvideo";
    case PrivVideo: return "privvideo";
    }
    assert(false);
    return NULL;
}
void CTheVideoLibrary::ToInfoList(const CVariant& varItems, const char* const idname, OUT BaseInfoList& items)
{
    for (unsigned int index = 0; index < varItems.size(); index++)
    {
        const CVariant& varItem = varItems[index];
        BaseInfo item = 
        {
            (int)varItem[idname].asInteger(),
            varItem["label"].asString(),
            varItem["title"].asString(),
            varItem["thumbnail"].asString()
        };
        items.push_back(item);
    }
}

// only for movies
bool CTheVideoLibrary::GetCountries(const BaseType type, OUT BaseInfoList& items)
{
    assert ( type == Movie );
    if ( type != Movie ) return false;

    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetCountries", params, result);
    if (!ret) return false;

    ToInfoList(result["countries"], "countryid", items);
    return true;
}

bool CTheVideoLibrary::GetStudios(const BaseType type, OUT BaseInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetStudios", params, result);
    if (!ret) return false;

    ToInfoList(result["studios"], "studioid", items);
    return true;
}

bool CTheVideoLibrary::GetDirectors(const BaseType type, OUT BaseInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetDirectors", params, result);
    if (!ret) return false;

    ToInfoList(result["directors"], "directorid", items);
    return true;
}

bool CTheVideoLibrary::GetYears(const BaseType type, OUT BaseInfoList& items)
{
    assert ( type == Movie || type == TvShow);
    if ( type != Movie && type != TvShow ) return false;

    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetYears", params, result);
    if (!ret) return false;

    ToInfoList(result["years"], "year", items);
    return true;
}

bool CTheVideoLibrary::GetActors(const BaseType type, OUT BaseInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetActors", params, result);
    if (!ret) return false;

    ToInfoList(result["actors"], "actorid", items);
    return true;
}

bool CTheVideoLibrary::GetTags(const BaseType type, OUT BaseInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["type"] = TranslateType(type);
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetTags", params, result);
    if (!ret) return false;

    ToInfoList(result["tags"], "tagid", items);
    return true;
}

bool CTheVideoLibrary::GetMusicVideoAlbums(OUT BaseInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["properties"] = CJSONVariantParser::ParseStr("[\"title\", \"thumbnail\"]");
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetMusicVideoAlbums", params, result);
    if (!ret) return false;

    ToInfoList(result["albums"], "albumid", items);
    return true;
}

void CTheVideoLibrary::ToVideoInfo(const CVariant& varItem, OUT PrivVideoInfo& info)
{
    info.privVideoId    = (int)varItem["privvideoid"].asInteger();
    info.title          = varItem["title"].asString();
    info.playcount      = (int)varItem["playcount"].asInteger();
    info.runtime        = varItem["runtime"].asString();
    info.lastplayed     = varItem["lastplayed"].asString();
    info.strTag         = varItem["tag"].asString();
    info.thumbnail      = varItem["thumbnail"].asString();
    info.file           = varItem["file"].asString();
    info.dateadded      = varItem["dateadded"].asString();
    //info.streamDetails;
    //info.resume;
}

void CTheVideoLibrary::ToVideoInfoList(const CVariant& varItems, OUT PrivVideoInfoList& items)
{
    for (unsigned int index = 0; index < varItems.size(); index++)
    {
        const CVariant& varItem = varItems[index];
        PrivVideoInfo info;  
        ToVideoInfo(varItem, info);
        items.push_back(info);
    }
}

bool CTheVideoLibrary::GetPrivVideos(OUT PrivVideoInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["properties"] = CJSONVariantParser::ParseStr(
        "[\"title\", \"playcount\", \"runtime\","
        "\"streamdetails\", \"lastplayed\","
        "\"thumbnail\", \"file\", \"resume\", \"dateadded\", \"tag\"]" );
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetPrivVideos", params, result);
    if (!ret) return false;

    ToVideoInfoList(result["privvideos"], items);
    return true;
}

bool CTheVideoLibrary::GetPrivVideoDetails(const int privVideoId, OUT PrivVideoInfo& info)
{
    CVariant params = CVariant::VariantTypeObject;
    params["privvideoid"] = privVideoId;
    params["properties"] = CJSONVariantParser::ParseStr(
        "[\"title\", \"playcount\", \"runtime\","
        "\"streamdetails\", \"lastplayed\","
        "\"thumbnail\", \"file\", \"resume\", \"dateadded\", \"tag\"]" );

    CVariant result;
    bool ret = MethodCall("GetPrivVideoDetails", params, result);
    if (!ret) return false;

    ToVideoInfo(result["privvideodetails"], info);
    return true;
}

bool CTheVideoLibrary::GetRecentlyAddedPrivVideos(OUT PrivVideoInfoList& items)
{
    CVariant params = CVariant::VariantTypeObject;
    params["properties"] = CJSONVariantParser::ParseStr(
        "[\"title\", \"playcount\", \"runtime\","
        "\"streamdetails\", \"lastplayed\","
        "\"thumbnail\", \"file\", \"resume\", \"dateadded\", \"tag\"]" );
    //params["limits"]
    //params["sort"]

    CVariant result;
    bool ret = MethodCall("GetRecentlyAddedPrivVideos", params, result);
    if (!ret) return false;

    ToVideoInfoList(result["privvideos"], items);
    return true;

}

bool CTheVideoLibrary::SetPrivVideoDetails(const int privVideoId, const PrivVideoDetails details)
{
    CVariant params = CVariant::VariantTypeObject;
    params["privvideoid"] = privVideoId;
    params["title"] = details.title.c_str();
    params["playcount"] = details.playcount;
    params["runtime"] = details.runtime.c_str();
    params["lastplayed"] = details.lastplayed.c_str();
    params["thumbnail"] = details.thumbnail.c_str();
    params["tag"] = details.strTag.c_str();

    CVariant result;
    bool ret = MethodCall("SetPrivVideoDetails", params, result);
    if (!ret) return false;

    return true;    
}

bool CTheVideoLibrary::RemovePrivVideo(const int privVideoId)
{
    CVariant params = CVariant::VariantTypeObject;
    params["privvideoid"] = privVideoId;

    CVariant result;    
    bool ret = MethodCall("RemovePrivVideo", params, result);
    if (!ret) return false;

    return true;    
}

}