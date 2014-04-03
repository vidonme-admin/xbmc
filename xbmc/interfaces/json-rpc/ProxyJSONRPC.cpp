#if defined(__VIDONME_MEDIACENTER__)

#include <map>
#include "ProxyJSONRPC.h"
#include "utils/JSONVariantParser.h"
#include "utils/JSONVariantWriter.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "settings/AdvancedSettings.h"
#include "filesystem/CurlFile.h"
#include "URL.h"
#include "JSONRPC.h"
#include "interfaces/legacy/aojsonrpc.h"
#include "vidonme/VDMUtils.h"
#include "settings/Settings.h"
#include "video/VideoInfoTag.h"
#include "Application.h"
#include "storage/MediaManager.h"
#include "filesystem/VirtualDirectory.h"
#include "filesystem/SourcesDirectory.h"
#include "cores/dvdplayer/DVDFileInfo.h"
#include "TextureCacheJob.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "DllLibbluray.h"
#include "filesystem/File.h"
#include "filesystem/Directory.h"
#include "cores/vidonme/VDMPlayer.h"
#include "Util.h"

using namespace VidOnMe;

namespace JSONRPC
{

struct SessionActionStateMapping
{
	const char* const szState;
	SessionActionState state;
};

static const SessionActionStateMapping MapSessionActionState[] ={
	{"sasUnknown",      sasUnknown},
	{"sasNone",         sasNone},
	{"sasRunning",      sasRunning},
	{"sasCancelling",   sasCancelling},
	{"sasCancelled",    sasCancelled},
	{"sasOk",           sasOk},
	{"sasFailed",       sasFailed},
};

CStdString TranslateSessionActionState(const SessionActionState state)
{
	for (unsigned int index=0; index < sizeof(MapSessionActionState)/sizeof(MapSessionActionState[0]); ++index)
	{
		const SessionActionStateMapping &map = MapSessionActionState[index];
		if (state == map.state)
		{
			return map.szState;
		}
	}
	return "";
}

SessionActionState TranslateSessionActionState(const CStdString& strState)
{
	for (unsigned int index=0; index < sizeof(MapSessionActionState)/sizeof(MapSessionActionState[0]); ++index)
	{
		const SessionActionStateMapping &map = MapSessionActionState[index];
		if (strState.Equals(map.szState))
			return map.state;
	}
	return sasUnknown;
}


struct SessionActionEventMapping
{
	const char* const szEvent;
	SessionActionEvent event;
};

static const SessionActionEventMapping MapSessionActionEvent[] ={
	{"saeUnknown",      saeUnknown},
	{"saeRunning",      saeRunning},
	{"saeCancelling",   saeCancelling},
	{"saeCancelled",    saeCancelled},
	{"saeOk",           saeOk},
	{"saeFailed",       saeFailed},
};

CStdString TranslateSessionActionEvent(const SessionActionEvent event)
{
	for (unsigned int index=0; index < sizeof(MapSessionActionEvent)/sizeof(MapSessionActionEvent[0]); ++index)
	{
		const SessionActionEventMapping &map = MapSessionActionEvent[index];
		if (event == map.event)
		{
			return map.szEvent;
		}
	}
	return "";
}

SessionActionEvent TranslateSessionActionEvent(const CStdString& strEvent)
{
	for (unsigned int index=0; index < sizeof(MapSessionActionEvent)/sizeof(MapSessionActionEvent[0]); ++index)
	{
		const SessionActionEventMapping &map = MapSessionActionEvent[index];
		if (strEvent.Equals(map.szEvent))
			return map.event;
	}
	return saeUnknown;
}


struct SessionResultMapping
{
	const char* const szResult;
	SessionResult result;
};

static const SessionResultMapping MapSessionResult[] ={
	{"srUnknown",                   srUnknown},
	{"srOk",                        srOk},
	{"srOverMaxSize",               srOverMaxSize},
	{"srInvalidSessionAction",      srInvalidSessionAction},
	{"srInvalidSessionClient",      srInvalidSessionClient},
	{"srDuplicateConnect",          srDuplicateConnect},
	{"srNotConnected",              srNotConnected},
	{"srDuplicateSubscribe",        srDuplicateSubscribe},
	{"srNotSubscribe",              srNotSubscribe},
	{"srInvalidClientConnection",   srInvalidClientConnection},
};

CStdString TranslateSessionResult(const SessionResult result)
{
	for (unsigned int index=0; index < sizeof(MapSessionResult)/sizeof(MapSessionResult[0]); ++index)
	{
		const SessionResultMapping &map = MapSessionResult[index];
		if (result == map.result)
		{
			return map.szResult;
		}
	}
	return "";
}

SessionResult TranslateSessionResult(const CStdString& strResult)
{
	for (unsigned int index=0; index < sizeof(MapSessionResult)/sizeof(MapSessionResult[0]); ++index)
	{
		const SessionResultMapping &map = MapSessionResult[index];
		if (strResult.Equals(map.szResult))
			return map.result;
	}
	return srUnknown;
}


struct NotifyKindMapping
{
	const char* const szKind;
	NotifyKind kind;
};

static const NotifyKindMapping MapNotifyKind[] ={
	{"nkUnknown",                   nkUnknown},
	{"nkTest1",                     nkTest1},
	{"nkTest2",                     nkTest2},
	{"update video library",        nkUpdateVideoLibrary},
	{"scan director",               nkScanDirectory},
	{"file analysis",               nkAnalysisFile},
	{"file scraper",								nkScraperFile},
};

CStdString TranslateNotifyKind(const NotifyKind kind)
{
	for (unsigned int index=0; index < sizeof(MapNotifyKind)/sizeof(MapNotifyKind[0]); ++index)
	{
		const NotifyKindMapping &map = MapNotifyKind[index];
		if (kind == map.kind)
		{
			return map.szKind;
		}
	}
	return "";
}

NotifyKind TranslateNotifyKind(const CStdString& strKind)
{
	for (unsigned int index=0; index < sizeof(MapNotifyKind)/sizeof(MapNotifyKind[0]); ++index)
	{
		const NotifyKindMapping &map = MapNotifyKind[index];
		if (strKind.Equals(map.szKind))
			return map.kind;
	}
	return nkUnknown;
}

SessionResult TranslateNotifyKind(const CStdString& strKind, OUT NotifyKind& kind)
{
	kind = TranslateNotifyKind(strKind);
	return (kind == nkUnknown) ? srMistakenNotifyKind : srOk;
}

struct NotifyEventMapping
{
	const char* const szEvent;
	NotifyEvent event;
};

static const NotifyEventMapping MapNotifyEvent[] ={
	{"neUnknown",                   neUnknown},
	{"neTest1",                     neTest1},
	{"neTest2",                     neTest2},
};


CStdString TranslateNotifyEvent(const NotifyEvent event)
{
	for (unsigned int index=0; index < sizeof(MapNotifyEvent)/sizeof(MapNotifyEvent[0]); ++index)
	{
		const NotifyEventMapping &map = MapNotifyEvent[index];
		if (event == map.event)
		{
			return map.szEvent;
		}
	}
	return "";
}

NotifyEvent TranslateNotifyEvent(const CStdString& strEvent)
{
	for (unsigned int index=0; index < sizeof(MapNotifyEvent)/sizeof(MapNotifyEvent[0]); ++index)
	{
		const NotifyEventMapping &map = MapNotifyEvent[index];
		if (strEvent.Equals(map.szEvent))
			return map.event;
	}
	return neUnknown;
}

SessionResult TranslateNotifyEvent(const CStdString& strEvent, OUT NotifyEvent& event)
{
	event = TranslateNotifyEvent(strEvent);
	return (event == neUnknown) ? srMistakenNotifyEvent : srOk;
}


JSONRPCID get_jsonrpc_request_id( void )
{
	static JSONRPCID s_jsonrpc_request_id = 50;
	if (s_jsonrpc_request_id < 50)
	{
		s_jsonrpc_request_id = 50;
	}
	return s_jsonrpc_request_id++;
}

const char* CProxyTransportLayerCurl::host() const
{
  return m_host.c_str();
}

int CProxyTransportLayerCurl::port( void ) const
{
  return m_port;
}

bool CProxyTransportLayerCurl::Service(const CVariant& request, OUT CVariant& response)
{
  const CStdString request_string = CJSONVariantWriter::Write(request, g_advancedSettings.m_jsonOutputCompact);

  CStdString strURL;
  strURL.Format("http://%s:%d/jsonrpc", m_host.c_str(), m_port);

  CStdString response_string;
  XFILE::CCurlFile curl;
  bool ret = curl.PostJson(strURL, request_string, response_string);
  if (!ret) return false;

  response = CJSONVariantParser::Parse((unsigned char*) response_string.c_str(), response_string.length());
  return true;
}

CProxyTransportLayerClient::WaitForResponse::WaitForResponse()
  : m_finished(false)
  , m_disconnected(false)
  , m_response(CVariant::VariantTypeNull)
{

}


void CProxyTransportLayerClient::WaitForResponse::SetResponse(const CVariant& response)
{
  m_response = response;
  m_disconnected = false;
  m_finished = true;
}

void CProxyTransportLayerClient::WaitForResponse::disconnect( void )
{
  //m_response = response;
  m_disconnected = true;
  m_finished = true;
}



void CProxyTransportLayerClient::OnPushBuffer(const CStdString& data)
{
  CLog::Log(999, "jsonrpc response: %s", data.c_str());

  CVariant varData = CJSONVariantParser::Parse((unsigned char*) data.c_str(), data.length());

  if (varData.isMember("method") && !varData.isMember("id"))
  {
    if (m_transportLayerEvent)
    {
      m_transportLayerEvent->OnNotification(varData);
    }
  }
  else if (varData.isMember("result") || varData.isMember("error"))
  {
    // µÈ´ý¶ÔÏó;
    const JSONRPCID id = varData["id"].asInteger();
    assert( id > 20 );
    const WaitForResponseList::iterator found = m_waitingList.find(id);
    assert( found != m_waitingList.end() );
    WaitForResponse* waiting = found->second;
    m_waitingList.erase(found);
    waiting->SetResponse(varData);
  }
  else
  {
    CLog::Log(LOGINFO, "%s: no handler json-rpc message(%s)", __FUNCTION__, data.c_str() );
  }
}


const char* CProxyTransportLayerClient::host() const
{
  return m_client->host();
}

int CProxyTransportLayerClient::port( void ) const
{
  return m_client->port();
}

bool CProxyTransportLayerClient::Service(const CVariant& request, OUT CVariant& response)
{
  const CStdString request_string = CJSONVariantWriter::Write(request, g_advancedSettings.m_jsonOutputCompact);

  CLog::Log(999, "jsonrpc request: %s", request_string.c_str());
  assert(m_client);
  m_client->Send(request_string.c_str(), request_string.length());


  WaitForResponse waiting;
  const JSONRPCID id = request["id"].asInteger();
  assert( id > 20 );
  m_waitingList.insert(std::make_pair(id, &waiting));
  while ( !waiting.finished() )
  {
    Sleep(200);
  }        

  if ( waiting.disconnected() ) return false;

  response = waiting.response();
  return true;
}

void CProxyTransportLayerClient::OnDisconnect()
{
  for (unsigned int index = 0; index < m_waitingList.size(); ++index)
  {
    WaitForResponse* waiting = m_waitingList[index];
    waiting->disconnect();
  }
  m_waitingList.clear();
}

void CProxyJSONRPC::OnNotification(const CVariant& notification)
{

  std::string method = notification["method"].asString();
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
}

CStdString CProxyJSONRPC::GetThumbUrl(const CStdString& thumb2)
{
  if (thumb2.IsEmpty())
  {
    return thumb2;
  }

  VDMServer server;
  if (!VDMUtils::Instance().GetCurrentServer(server) || server.m_bLocal)
  {
    return thumb2;
  }

  CStdString thumb(thumb2);
  if (thumb2.compare(0, 8, "image://") != 0)
  {
    CStdString strTemp(thumb2);
    CURL::Encode(strTemp);
    thumb.Format("image://%s", strTemp.c_str());
  }

  CURL::Encode(thumb);

  CStdString strURL;
  strURL.Format("http://%s:%d/image/%s", 
    server.m_strIP,
    server.m_nJsonRPCPort,
    thumb.c_str());
  return strURL;
}

CStdString CProxyJSONRPC::GetFileUrl(const CStdString& thumb2)
{
  if (thumb2.IsEmpty())
  {
    return thumb2;
  }

  VDMServer server;
  if (!VDMUtils::Instance().GetCurrentServer(server) || server.m_bLocal)
  {
    return thumb2;
  }

  CStdString thumb(thumb2);
  CURL::Encode(thumb);

  CStdString strURL;
  strURL.Format("http://%s:%d/file/%s", 
    server.m_strIP,
    server.m_nJsonRPCPort,
    thumb.c_str());
  return strURL;
}

//bool CJsonProxy::MethodCall(const CVariant& request, CVariant& response)
bool CProxyJSONRPC::MethodCall(
  const std::string& method, 
  const CVariant& params, 
  CVariant& result)
{
  VDMServer server;
  if (!VDMUtils::Instance().GetCurrentServer(server))
  {
    return false;
  }

  if (server.m_bLocal)
  {
    CVariant request;
    request["jsonrpc"] = "2.0";
    request["method"] = method;
    request["params"] = params;
    request["id"] = get_jsonrpc_request_id();

    CAddOnTransport transport;
    CAddOnTransport::CAddOnClient client;
    CStdString strResult = CJSONRPC::MethodCall(CJSONVariantWriter::Write(request, true), &transport, &client);
    if (strResult.IsEmpty())
    {
      return false;
    }

    CVariant response = CJSONVariantParser::Parse((unsigned char *)strResult.c_str(), strResult.length());
    bool ret = response.isMember("result");
    //assert(ret);
    if (!ret) return false;

    result = response["result"];
    return true;
  }
  else
  {
    CProxyTransportLayerCurl tlURL(server.m_strIP, server.m_nJsonRPCPort);
    CProxyJSONRPC JsonRPC(&tlURL);

    return JsonRPC.MethodCallImpl(method, params, result);
  }
}

//bool CJsonProxy::MethodCallImpl(const CVariant& request, CVariant& response)
bool CProxyJSONRPC::MethodCallImpl(
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
  //assert(ret);
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

bool CProxyJSONRPC::ping()
{
  const CVariant params = CVariant::VariantTypeObject;

  CVariant result;
  bool ret = MethodCall("playlist.getplaylists", params, result);
  if (!ret) return false;

  return true;
}



bool CProxyJSONRPC::GetDirectory(
  const CStdString& directory,
  const CStdString& mask,
  OUT bool& ret,
  OUT FileNodes& fileNodes)
{
  CVariant params; 
  params["directory"] = directory.c_str();
  params["mask"] = mask.c_str();

  VDMServer server;
  VDMUtils::Instance().GetCurrentServer(server);
  if (server.m_bLocal)
  {
    CStdString strPath = directory;
    strPath.Trim();
    CFileItemList items;
    XFILE::CVirtualDirectory rootDir;

#if defined(TARGET_ANDROID) && defined(__ANDROID_ALLWINNER__)
    if (strPath.GetLength() <= 5 || strPath.Left(5) != "/mnt/")
    {
      CFileItemList rootItems;
      rootDir.GetDirectory("/mnt/", rootItems, true);
      for (int i = 0; i < rootItems.Size(); i++)
      {
        CStdString strPath = rootItems[i]->GetPath();
        CLog::Log(LOGDEBUG, "system root dir: %s", strPath.c_str());
        if (strPath.GetLength() >= 10 && strPath.Left(10).CompareNoCase("/mnt/VidTV") == 0 && rootItems[i]->m_bIsFolder)
        {
          items.Add(rootItems[i]);
          CLog::Log(LOGDEBUG, "root title: %s", rootItems[i]->GetLabel().c_str());
        }
      }
    }
#else
    if (strPath.IsEmpty())
    {
      VECSOURCES extraShares;
      g_mediaManager.GetLocalDrives(extraShares);
      g_mediaManager.GetRemovableDrives(extraShares);
      g_mediaManager.GetNetworkLocations(extraShares);
      rootDir.SetMask(mask);
      rootDir.SetSources(extraShares);
      rootDir.GetDirectory(strPath, items);
    }
#endif
    else
    {
      rootDir.SetMask(mask);
      rootDir.GetDirectory(strPath, items);
      CStdString strParentPath = URIUtils::GetParentPath(strPath);
      
      CFileItemPtr pItem(new CFileItem(".."));
      pItem->SetPath(URIUtils::GetParentPath(strPath));
      pItem->m_bIsFolder = true;
      pItem->m_bIsShareOrDrive = false;
      items.AddFront(pItem, 0);
    }

    for(unsigned int index = 0; index < items.Size(); index++)
    {
      FileNode node;
      node.strTitle       = items[index]->GetLabel();
      node.strPath        = items[index]->GetPath();
      node.isFolder       = items[index]->m_bIsFolder;
      if(node.strTitle == g_localizeStrings.Get(21440))
      {
        continue;
      }
      fileNodes.push_back(node);
    }
  }
  else
  {
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
      if(node.strTitle.IsEmpty())
      {
        continue;
      }

			if (CUtil::IsPicture(node.strPath))
			{
				node.strPath				= GetThumbUrl(node.strPath);
			}

      fileNodes.push_back(node);
    }

    CStdString strParentPath;
    if (URIUtils::GetParentPath(directory, strParentPath))
    {
      FileNode node;
      node.strTitle       = "..";
      node.strPath        = strParentPath;
      node.isFolder       = true;
      fileNodes.insert(fileNodes.begin(), 1, node);
    }
  }

  return true;
}


bool CProxyJSONRPC::GetShares(
	OUT bool& ret,
	OUT Shares& shares)
{
	CVariant params(CVariant::VariantTypeArray);

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
		share.sharePath = varShare["path"].asString();
		share.type = varShare["type"].asString();

		shares.push_back(share);
	}
	return true;
}

bool CProxyJSONRPC::AddShare(
	const CStdString& type,
	const CStdString& name,
	const CStdString& path,
	OUT bool& ret)
{
	CVariant params;
	params["type"] = type.c_str();
	params["name"] = name.c_str();
	params["path"] = path.c_str();

	CVariant result;
	bool bRet = MethodCall("VidOnMe.AddShare", params, result);
	if (!bRet) return false;

	ret = result["ret"].asBoolean();
	return true;
}

bool CProxyJSONRPC::UpdateShare(
	const CStdString& oldname,
	const CStdString& oldpath,
	const CStdString& oldtype,
	const CStdString& name,
	const CStdString& path,
	const CStdString& type,
	OUT bool& ret)
{
	CVariant params;
	params["oldname"] = oldname.c_str();
	params["oldpath"] = oldpath.c_str();
	params["oldtype"] = oldtype.c_str();
	params["name"]		= name.c_str();
	params["path"]		= path.c_str();
	params["type"]		= type.c_str();
	
	CVariant result;
	bool bRet = MethodCall("VidOnMe.UpdateShare", params, result);
	if (!bRet) return false;

	ret = result["ret"].asBoolean();
	return true;
}

bool CProxyJSONRPC::DeleteShare(
	const CStdString& type,
	const CStdString& name,
	const CStdString& path,
	OUT bool& ret)
{
	CVariant params;
	params["type"] = type.c_str();
	params["name"] = name.c_str();
	params["path"] = path.c_str();

	CVariant result;
	bool bRet = MethodCall("VidOnMe.DeleteShare", params, result);
	if (!bRet) return false;

	ret = result["ret"].asBoolean();
	return true;
}

bool CProxyJSONRPC::SetVideoTypeForPath(
  const CStdString& directory,
  const CStdString& type,
  const VideoScanSettings& scanSettings,
  OUT bool& ret)
{
  CVariant params;
  params["directory"] = directory.c_str();
  params["type"] = type.c_str();
  params["scanSettings"] = CVariant::VariantTypeObject;

  CVariant& varScanSettings = params["scanSettings"];
  varScanSettings["parent_name"]      = scanSettings.parent_name;
  varScanSettings["parent_name_root"] = scanSettings.parent_name_root;
  varScanSettings["recurse"]          = scanSettings.recurse;
  varScanSettings["noupdate"]         = scanSettings.noupdate;
  varScanSettings["exclude"]          = scanSettings.exclude;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.SetVideoTypeForPath", params, result);
  if (!bRet) return false;

  ret = result["ret"].asBoolean();
  return true;
}

bool CProxyJSONRPC::GetVideoTypeForPath(
  const CStdString& directory,
  OUT bool& ret,
  OUT CStdString& type,
  OUT VideoScanSettings& scanSettings)
{
  CVariant params;
  params["directory"] = directory.c_str();

  CVariant result;
  bool bRet = MethodCall("VidOnMe.GetVideoTypeForPath", params, result);
  if (!bRet) return false;

  ret = result["ret"].asBoolean();
  if (!ret) return true;

  const CVariant& varScanSettings = result["scanSettings"];
  
  scanSettings.parent_name = varScanSettings["parent_name"].asBoolean();
  scanSettings.parent_name_root = varScanSettings["parent_name_root"].asBoolean();
  scanSettings.recurse = varScanSettings["recurse"].asBoolean();
  scanSettings.noupdate = varScanSettings["noupdate"].asBoolean();
  scanSettings.exclude = varScanSettings["exclude"].asBoolean();

  return true;
}

bool CProxyJSONRPC::EraseVideoTypeForPath(
  const CStdString& directory,
  OUT bool& ret)
{
  CVariant params;
  params["directory"] = directory.c_str();

  CVariant result;
  bool bRet = MethodCall("VidOnMe.EraseVideoTypeForPath", params, result);
  if (!bRet) return false;

  ret = result["ret"].asBoolean();
  return true;
}

bool CProxyJSONRPC::GetScrapers(
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
bool CProxyJSONRPC::SetVideoScraperForPath(
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

bool CProxyJSONRPC::SetMusicScraperForPath(
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

bool CProxyJSONRPC::GetVideoScraperForPath(
  const CStdString& directory, 
  OUT CStdString& content,
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

  content         = result["content"].asString();
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

bool CProxyJSONRPC::GetMusicScraperForPath(
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
bool CProxyJSONRPC::StartVideoScan(
  const CStdString &directory, 
  const bool scanAll)
{
  VDMServer server;
  if (VDMUtils::Instance().GetCurrentServer(server))
  {
    if (server.m_bLocal)
    {
      g_application.StartVideoScan(directory, scanAll);
    }
    else
    {
      CVariant params;
      params["directory"] = directory.c_str();
      params["scanAll"] = scanAll;

      CVariant result;
      bool bRet = MethodCall("VidOnMe.StartVideoScan", params, result);
      if (!bRet) return false;

      bool ret = result["ret"].asBoolean();
      assert(ret);

      return ret;
    }

    return true;
  }
  
  return false;
}

bool CProxyJSONRPC::StopVideoScan()
{
  CVariant params = CVariant::VariantTypeArray;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.StopVideoScan", params, result);
  if (!bRet) return false;

  bool ret = result["ret"].asBoolean();
  assert(ret);

  return true;
}

bool CProxyJSONRPC::IsVideoScanning(OUT bool& ret)
{
  CVariant params = CVariant::VariantTypeArray;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.IsVideoScanning", params, result);
  if (!bRet) return false;

  ret = result["ret"].asBoolean();
  return true;
}

bool CProxyJSONRPC::StartVideoCleanup()
{
  CVariant params = CVariant::VariantTypeArray;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.StartVideoCleanup", params, result);
  if (!bRet) return false;

  bool ret = result["ret"].asBoolean();
  assert(ret);

  return true;
}

bool CProxyJSONRPC::StartMusicScan(
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

bool CProxyJSONRPC::StartMusicAlbumScan(
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

bool CProxyJSONRPC::StartMusicArtistScan(
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

bool CProxyJSONRPC::StopMusicScan()
{
  CVariant params = CVariant::VariantTypeArray;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.StopMusicScan", params, result);
  if (!bRet) return false;

  bool ret = result["ret"].asBoolean();
  assert(ret);

  return true;
}

bool CProxyJSONRPC::IsMusicScanning(OUT bool& ret)
{
  CVariant params = CVariant::VariantTypeArray;

  CVariant result;
  bool bRet = MethodCall("VidOnMe.IsMusicScanning", params, result);
  if (!bRet) return false;

  ret = result["ret"].asBoolean();
  return true;
}

bool CProxyJSONRPC::IsServerDeviceAvailable(CStdString strFilePath, bool& bExist)
{
	VDMServer server;

	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( server.m_bLocal )
	{
#if defined(TARGET_ANDROID) && defined(__ANDROID_ALLWINNER__)

		bExist = false;

		if (strFilePath.GetLength() > 10 && strFilePath.Left(10).CompareNoCase("/mnt/VidTV") == 0)
		{
			CStdString strPathTmp, strTmp;
			strPathTmp = strFilePath.Left(10);
			strTmp = strFilePath.Mid(10);
			strPathTmp += strTmp.Left(strTmp.find_first_of('/'));

			CLog::Log(LOGINFO,"strPathTmp:%s",strPathTmp.c_str());
			
			XFILE::CVirtualDirectory rootDir;
			CFileItemList rootItems;
			rootDir.GetDirectory("/mnt/", rootItems, true);
			for (int i = 0; i < rootItems.Size(); i++)
			{
				CStdString strPath = rootItems[i]->GetPath();
				int nSize = strPathTmp.size();
				
				CLog::Log(LOGINFO,"strPath:%s",strPath.c_str());

				if (strPath.GetLength() >= nSize && strPath.Left(nSize).CompareNoCase(strPathTmp) == 0)
				{
					CLog::Log(LOGINFO,"true");

					bExist = true;
					break;
				}
			}
		}

#else
	
		CStdStringArray strings = URIUtils::SplitPath(strFilePath);
		if (!strings.empty())
		{
			bExist = XFILE::CDirectory::Exists(strings[0]);
		}

#endif
		
		return true;
	}
	else
	{
		bExist = true;
		return true;
	}
	
	return true;
}

bool CProxyJSONRPC::IsServerFilesExists(CStdString strFilePath, bool& bExist)
{
	VDMServer server;

	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( server.m_bLocal )
	{
    if (bExist = XFILE::CDirectory::Exists(strFilePath))
    {
      return true;
    }

		bExist = XFILE::CFile::Exists(strFilePath);

		return true;
	}
	else
	{
		CVariant	varParams;

		varParams["file"]			= strFilePath;

		CVariant varResult;
		bool bRet = MethodCall("Files.Exists", varParams, varResult);
		if (!bRet) return false;

		bExist = varResult["ret"].asBoolean();
	}

	return true;
}

bool CProxyJSONRPC::GetServerMovies(std::vector<ServerMovieInfo>& vecServerMoviesInfo)
{
  Limits limits;

  MovieSort	sort;
  sort.strSortItem = "title";

  MovieFilter	filter;

  return GetServerMovies(limits, sort, filter, vecServerMoviesInfo);
}

bool CProxyJSONRPC::GetServerMovies(const Limits& limits, const MovieSort& sort, const MovieFilter& filter, 
  std::vector<ServerMovieInfo>& vecServerMoviesInfo)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

  CVariant	varParams;

  // Properties: Movie content we need. 
  CVariant	varProperties;
	varProperties.push_back("title");
  varProperties.push_back("thumbnail");
  varProperties.push_back("country");
  varProperties.push_back("year");
  varProperties.push_back("file");
  varProperties.push_back("genre");

  // Limits: Can used to paging.
  CVariant	varLimits;
  varLimits["start"]		= limits.nStart;
  if (limits.nEnd > 0)
  {
    varLimits["end"]	= limits.nEnd;
  }

  // Sort: Can only sort by one item, can ascend or descend.
  CVariant	varSort;
  const std::string& str = sort.strSortItem;
  varSort["method"]	= str;

  if (sort.bDescending)
  {
    varSort["order"] = "descending";
  } 
  else
  {
    varSort["order"] = "ascending";
  }

  varSort["ignorearticle"]	= sort.bIgnoreArticle;

  // Filter: Can mixed filt by year/title/country/genre/... can filt by part of strings.
  CVariant		varFilterItems;
  if (filter.nYear > 0)
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "year";
    varFilterItem["operator"]	= "is";

    CStdString strYear;
    strYear.Format("%4d", filter.nYear);
    varFilterItem["value"]		= strYear;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strTitle.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "title";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strTitle;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strGenre.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "genre";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strGenre;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strActor.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "actor";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strActor;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strDirector.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "director";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strDirector;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strWriter.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "writers";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strWriter;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strStudio.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "studio";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strStudio;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strCountry.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "country";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strCountry;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strSet.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "set";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strSet;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strTag.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "tag";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strTag;

    varFilterItems.push_back(varFilterItem);
  }

  varParams["properties"]		= varProperties;

  if (limits.nEnd > 0)
  {
    varParams["limits"]				= varLimits;
  }

  if (sort.strSortItem != "")
  {
    varParams["sort"]					= varSort;
  }

  CVariant	varFileters;
  if (varFilterItems.size() > 0)
  {
    varFileters["and"]			= varFilterItems;
    varParams["filter"]			= varFileters;
  }

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.GetMovies", varParams, varResult);  
  if (!bRet) return false;

  const CVariant& varMovies = varResult["movies"];
  for (unsigned int index = 0; index < varMovies.size(); ++index)
  {
    ServerMovieInfo		stMovieInfo;

    const CVariant& varMovie	= varMovies[index];

    const CVariant& varCountries	= varMovie["country"];
    for (unsigned int Idx = 0; Idx < varCountries.size(); ++Idx)
    {
      const CVariant& varCountry	= varCountries[Idx];
      std::string			strCountry	= varCountry.asString();
      if (!strCountry.empty())
      {
        stMovieInfo.vecCountries.push_back(strCountry);
      }
    }

    const CVariant& varGenres	= varMovie["genre"];
    for (unsigned int Idx = 0; Idx < varGenres.size(); ++Idx)
    {
      const CVariant& varGenre	= varGenres[Idx];
      std::string			strGenre	= varGenre.asString();

      if (!strGenre.empty())
      {
        stMovieInfo.vecGenres.push_back(strGenre);
      }
    }

		if( server.m_bLocal )
		{
			stMovieInfo.strMovieID				= varMovie["movieid"].asString();
			stMovieInfo.strTitle					= varMovie["label"].asString();
		}
		else
		{
			stMovieInfo.strMovieID				= varMovie["idfile"].asString();
			stMovieInfo.strTitle					= varMovie["title"].asString();
			stMovieInfo.strKeyID					= varMovie["idkey"].asString();
		}

    const CVariant& varThumbnai			= varMovie["thumbnail"];
    stMovieInfo.strThumbnail				= GetThumbUrl(varThumbnai.c_str());

		stMovieInfo.strFilePath					= varMovie["file"].asString();
		stMovieInfo.strYear							= varMovie["year"].asString();

    vecServerMoviesInfo.push_back(stMovieInfo);
  }

  return true;
}

bool CProxyJSONRPC::GetServerMovieDetails(int nMovieID, ServerMovieDetail& stMovieDetails)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	CVariant	varParams;

	// Properties: Movie content we need. 
	CVariant	varProperties;

	if( server.m_bLocal )
	{
		CVariant	varProperties;
		varProperties.push_back("cast");
		varProperties.push_back("director");
		varProperties.push_back("mpaa");
		varProperties.push_back("plot");
		varProperties.push_back("resume");
		varProperties.push_back("runtime");
		varProperties.push_back("studio");
		varProperties.push_back("writer");

		varParams["movieid"] = nMovieID;
		varParams["properties"]	= varProperties;
	}
	else
	{
		varParams["idfile"]			= nMovieID;
	}

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.GetMovieDetails", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varMovie	= varResult["moviedetails"];

  stMovieDetails.vecActors.clear();
  const CVariant& varActors	= varMovie["cast"];
  for (unsigned int Idx = 0; Idx < varActors.size(); ++Idx)
  {
    Actor		stActor;
    const CVariant& varActor	= varActors[Idx];

    stActor.strName		= varActor["name"].asString();
    stActor.strRole		= varActor["role"].asString();

    stMovieDetails.vecActors.push_back(stActor);
  }

  stMovieDetails.vecWriters.clear();
  const CVariant& varWriters	= varMovie["writer"];
  for (unsigned int Idx = 0; Idx < varWriters.size(); ++Idx)
  {
    const CVariant& varWriter	= varWriters[Idx];
    stMovieDetails.vecWriters.push_back(varWriter.asString());
  }

	stMovieDetails.vecDirectors.clear();
	const CVariant& varDirectors	= varMovie["director"];
	for (unsigned int Idx = 0; Idx < varDirectors.size(); ++Idx)
	{
		const CVariant& varDirector	= varDirectors[Idx];
		stMovieDetails.vecDirectors.push_back(varDirector.asString());
	}

	if( server.m_bLocal )
	{
		stMovieDetails.vecStudios.clear();
		const CVariant& varStudios	= varMovie["studio"];
		for (unsigned int Idx = 0; Idx < varStudios.size(); ++Idx)
		{
			const CVariant& varStudio	= varStudios[Idx];
			stMovieDetails.vecStudios.push_back(varStudio.asString());
		}

		const CVariant& varResume					= varMovie["resume"];
		stMovieDetails.stResume.nPosition	= varResume["position"].asInteger();
		stMovieDetails.stResume.nTotal		= varResume["total"].asInteger();

		stMovieDetails.strMovieID				= varMovie["movieid"].asString();
		stMovieDetails.strTitle					= varMovie["label"].asString();
	}
	else
	{
		stMovieDetails.strMovieID				= varMovie["idfile"].asString();
		stMovieDetails.strTitle					= varMovie["title"].asString();
	}

	stMovieDetails.strMPAA						= varMovie["mpaa"].asString();
	stMovieDetails.strPlot						= varMovie["plot"].asString();
	stMovieDetails.strRunTime					= varMovie["runtime"].asString();

  return true;
}

bool CProxyJSONRPC::GetServerMusics(std::vector<ServerMusicInfo>& vecServerMusicsInfo)
{
  Limits limits;

  MusicSort	sort;
  sort.strSortItem = "title";

  MusicFilter	filter;

  return GetServerMusics(limits, sort, filter, vecServerMusicsInfo);
}

bool CProxyJSONRPC::GetServerMusics(const Limits& limits, const MusicSort& sort, const MusicFilter& filter, 
  std::vector<ServerMusicInfo>& vecServerMusicsInfo)
{
  CVariant	varParams;

  // Properties: Music content we need. 
  CVariant	varProperties;
  varProperties.push_back("artist");
  varProperties.push_back("duration");
  varProperties.push_back("file");
  varProperties.push_back("genre");
  varProperties.push_back("lastplayed");
  varProperties.push_back("lyrics");
  varProperties.push_back("playcount");
  varProperties.push_back("rating");
  varProperties.push_back("thumbnail");
  varProperties.push_back("title");
  varProperties.push_back("year");

  // Limits: Can used to paging.
  CVariant	varLimits;
  varLimits["start"]		= limits.nStart;
  if (limits.nEnd > 0)
  {
    varLimits["end"]	= limits.nEnd;
  }

  // Sort: Can only sort by one item, can ascend or descend.
  CVariant	varSort;
  const std::string& str = sort.strSortItem;
  varSort["method"]	= str;

  if (sort.bDescending)
  {
    varSort["order"] = "descending";
  } 
  else
  {
    varSort["order"] = "ascending";
  }

  varSort["ignorearticle"]	= sort.bIgnoreArticle;

  // Filter: Can mixed filt by year/title/country/genre/... can filt by part of strings.
  CVariant		varFilterItems;
  if (filter.nYear > 0)
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "year";
    varFilterItem["operator"]	= "is";

    CStdString strYear;
    strYear.Format("%4d", filter.nYear);
    varFilterItem["value"]		= strYear;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strTitle.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "title";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strTitle;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strGenre.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "genre";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strGenre;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strArtist.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "artist";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strArtist;

    varFilterItems.push_back(varFilterItem);
  }

  varParams["properties"]		= varProperties;

  if (limits.nEnd > 0)
  {
    varParams["limits"]				= varLimits;
  }

  if (sort.strSortItem != "title")
  {
    varParams["sort"]					= varSort;
  }

  CVariant	varFileters;
  if (varFilterItems.size() > 0)
  {
    varFileters["and"]		= varFilterItems;
    varParams["filter"]		= varFileters;
  }

  CVariant varResult;
  bool bRet = MethodCall("AudioLibrary.GetSongs", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varMusics = varResult["songs"];
  for (unsigned int index = 0; index < varMusics.size(); ++index)
  {
    ServerMusicInfo		stMusicInfo;

    const CVariant& varMusic	= varMusics[index];

    const CVariant& varArtists	= varMusic["artist"];
    for (unsigned int Idx = 0; Idx < varArtists.size(); ++Idx)
    {
      stMusicInfo.vecArtists.push_back(varArtists[Idx].asString());
    }

    const CVariant& varDuration	= varMusic["duration"];
    stMusicInfo.nDuration		= varDuration.asInteger();

    const CVariant& varFile		= varMusic["file"];
    stMusicInfo.strFilePath		= varFile.asString();

    const CVariant& varGenres	= varMusic["genre"];
    for (unsigned int Idx = 0; Idx < varGenres.size(); ++Idx)
    {
      stMusicInfo.vecGenres.push_back(varGenres[Idx].asString());
    }

    const CVariant& varLastPlayed	= varMusic["lastplayed"];
    stMusicInfo.strLastPlayed		= varLastPlayed.asString();

    const CVariant& varLyrics		= varMusic["lyrics"];
    stMusicInfo.strLyrics			= varLyrics.asString();

    const CVariant& varPlayCount	= varMusic["playcount"];
    stMusicInfo.nPlayCount			= varPlayCount.asInteger();

    const CVariant& varRating		= varMusic["rating"];
    stMusicInfo.nRating				= varRating.asInteger();

    const CVariant& varThumbnail	= varMusic["thumbnail"];
    stMusicInfo.strThumbnail		= varThumbnail.asString();

    const CVariant& varTitle		= varMusic["title"];
    stMusicInfo.strTitle			= varTitle.asString();

    const CVariant& varYear			= varMusic["year"];
    stMusicInfo.nYear				= varYear.asInteger();

    vecServerMusicsInfo.push_back(stMusicInfo);
  }

  return true;
}

bool CProxyJSONRPC::GetServerTVShow(std::vector<ServerTVShowInfo>& vecServerTVShowInfo)
{
  Limits limits;

  TVShowSort	sort;
  sort.strSortItem = "title";

  TVShowFilter	filter;

  return GetServerTVShow(limits, sort, filter, vecServerTVShowInfo);
}

bool CProxyJSONRPC::GetServerTVShow(const Limits& limits, const TVShowSort& sort, const TVShowFilter& filter, 
  std::vector<ServerTVShowInfo>& vecServerTVShowInfo)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

  CVariant	varParams;

	// Properties: Movie content we need. 
	CVariant	varProperties;

	if (server.m_bLocal)
	{
		varProperties.push_back("cast");
	}

	varProperties.push_back("genre");
	varProperties.push_back("mpaa");
	varProperties.push_back("plot");
	varProperties.push_back("studio");
	varProperties.push_back("thumbnail");
	varProperties.push_back("title");
	varProperties.push_back("year");
	varProperties.push_back("fanart");

  // Limits: Can used to paging.
  CVariant	varLimits;
  varLimits["start"]		= limits.nStart;
  if (limits.nEnd > 0)
  {
    varLimits["end"]	= limits.nEnd;
  }

  // Sort: Can only sort by one item, can ascend or descend.
  CVariant	varSort;
  const std::string& str = sort.strSortItem;
  varSort["method"]	= str;

  if (sort.bDescending)
  {
    varSort["order"] = "descending";
  } 
  else
  {
    varSort["order"] = "ascending";
  }

  varSort["ignorearticle"]	= sort.bIgnoreArticle;

  // Filter: Can mixed filt by year/title/country/genre/... can filt by part of strings.
  CVariant		varFilterItems;
  if (filter.nYear > 0)
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "year";
    varFilterItem["operator"]	= "is";

    CStdString strYear;
    strYear.Format("%4d", filter.nYear);
    varFilterItem["value"]		= strYear;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strTitle.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "title";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strTitle;

    varFilterItems.push_back(varFilterItem);
  }

	if (!filter.strGenre.empty())
	{
		CVariant	varFilterItem;
		varFilterItem["field"]		= "genre";
		varFilterItem["operator"]	= "contains";
		varFilterItem["value"]		= filter.strGenre;

    varFilterItems.push_back(varFilterItem);
  }

  if (!filter.strStudio.empty())
  {
    CVariant	varFilterItem;
    varFilterItem["field"]		= "studio";
    varFilterItem["operator"]	= "contains";
    varFilterItem["value"]		= filter.strStudio;

    varFilterItems.push_back(varFilterItem);
  }


  varParams["properties"]		= varProperties;

  if (limits.nEnd > 0)
  {
    varParams["limits"]				= varLimits;
  }

  if (sort.strSortItem != "none")
  {
    varParams["sort"]					= varSort;
  }

  CVariant	varFileters;
  if (varFilterItems.size() > 0)
  {
    varFileters["and"]		= varFilterItems;
    varParams["filter"]		= varFileters;
  }

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.GetTVShows", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varTVShows = varResult["tvshows"];
  for (unsigned int index = 0; index < varTVShows.size(); ++index)
  {
    ServerTVShowInfo		stTVShowInfo;

    const CVariant& varTVShow	= varTVShows[index];

    const CVariant& varActors	= varTVShow["cast"];
    for (unsigned int Idx = 0; Idx < varActors.size(); ++Idx)
    {
      Actor		stActor;
      const CVariant& varActor	= varActors[Idx];

      stActor.strName		= varActor["name"].asString();
      stActor.strRole		= varActor["role"].asString();

      stTVShowInfo.vecActors.push_back(stActor);
    }

    const CVariant& varStudios	= varTVShow["studio"];
    for (unsigned int Idx = 0; Idx < varStudios.size(); ++Idx)
    {
      const CVariant& varStudio	= varStudios[Idx];
      stTVShowInfo.vecStudios.push_back(varStudio.asString());
    }

    const CVariant& varGenres	= varTVShow["genre"];
    for (unsigned int Idx = 0; Idx < varGenres.size(); ++Idx)
    {
      const CVariant& varGenre	= varGenres[Idx];
      std::string			strGenre	= varGenre.asString();
      if (!strGenre.empty())
      {
        stTVShowInfo.vecGenres.push_back(strGenre);
      }
    }

    const CVariant& varThumbnai	= varTVShow["thumbnail"];
    stTVShowInfo.strThumbnail	= GetThumbUrl(varThumbnai.c_str());

		stTVShowInfo.strMPAA						= varTVShow["mpaa"].asString();
		stTVShowInfo.strPlot						= varTVShow["plot"].asString();
		stTVShowInfo.strTitle						= varTVShow["title"].asString();
		stTVShowInfo.strYear						= varTVShow["year"].asString();

		if (server.m_bLocal)
		{
			stTVShowInfo.strTVShowID				= varTVShow["tvshowid"].asString();
		} 
		else
		{
			stTVShowInfo.strTVShowID			= varTVShow["idtvshow"].asString();
		}

    vecServerTVShowInfo.push_back(stTVShowInfo);
  }

  return true;
}

bool CProxyJSONRPC::GetTVShowSeasons(int nTVShowID, std::vector<TVShowSeason>& vecTVShowSeasons)
{
	CVariant	varParams;

	// Properties: Movie content we need. 
	CVariant	varProperties;
	varProperties.push_back("season");
	varProperties.push_back("showtitle");
	varProperties.push_back("episode");
	varProperties.push_back("fanart");
	varProperties.push_back("tvshowid");
	varProperties.push_back("thumbnail");
	varProperties.push_back("watchedepisodes");

	varParams["tvshowid"]			= nTVShowID;
	varParams["properties"]		= varProperties;

	CVariant varResult;
	bool bRet = MethodCall("VideoLibrary.GetSeasons", varParams, varResult);
	if (!bRet) return false;

	const CVariant& varSeasons = varResult["seasons"];
	for (unsigned int index = 0; index < varSeasons.size(); ++index)
	{
		TVShowSeason		stTVShowSeason;

		const CVariant& varSeason	= varSeasons[index];

		const CVariant& varThumbnai	= varSeason["thumbnail"];
		stTVShowSeason.strThumbnail	= GetThumbUrl(varThumbnai.c_str());

		const CVariant& varFanart		= varSeason["fanart"];
		stTVShowSeason.strFanart		= GetThumbUrl(varFanart.c_str());

		stTVShowSeason.strShowTitle				= varSeason["showtitle"].asString();
		stTVShowSeason.strWatchedEpisodes	= varSeason["watchedepisodes"].asString();

		stTVShowSeason.strTVShowID				= varSeason["tvshowid"].asString();
		stTVShowSeason.strSeason					= varSeason["season"].asString();
		stTVShowSeason.strSeasonID				= varSeason["season"].asString();
		stTVShowSeason.strEpisode					= varSeason["episode"].asString();

		vecTVShowSeasons.push_back(stTVShowSeason);
	}

	return true;
}

bool CProxyJSONRPC::GetTVShowEpisodes(int nTVShowID, int nSeason, std::vector<TVShowEpisode>& vecTVShowEpisodes)
{
	CVariant	varParams;

	// Properties: Movie content we need. 
	CVariant	varProperties;
	varProperties.push_back("file");
	varProperties.push_back("title");
	varProperties.push_back("plot");
	varProperties.push_back("thumbnail");
	varProperties.push_back("runtime");
	varProperties.push_back("resume");
	varProperties.push_back("season");
	varProperties.push_back("episode");
	varProperties.push_back("tvshowid");
	varProperties.push_back("director");
	varProperties.push_back("writer");

	varParams["tvshowid"]			= nTVShowID;
	varParams["season"]				= nSeason;
	varParams["properties"]		= varProperties;

	CVariant varResult;
	bool bRet = MethodCall("VideoLibrary.GetEpisodes", varParams, varResult);
	if (!bRet) return false;

	const CVariant& varEpisodes = varResult["episodes"];
	for (unsigned int index = 0; index < varEpisodes.size(); ++index)
	{
		TVShowEpisode		stTVShowEpisode;

		const CVariant& varEpisode		= varEpisodes[index];
		const CVariant& varActors			= varEpisode["cast"];
		for (unsigned int Idx = 0; Idx < varActors.size(); ++Idx)
		{
			Actor		stActor;
			const CVariant& varActor	= varActors[Idx];

			stActor.strName		= varActor["name"].asString();
			stActor.strRole		= varActor["role"].asString();

			stTVShowEpisode.vecActors.push_back(stActor);
		}

		const CVariant& varWriters	= varEpisode["writer"];
		for (unsigned int Idx = 0; Idx < varWriters.size(); ++Idx)
		{
			const CVariant& varWriter	= varWriters[Idx];
			stTVShowEpisode.vecWriters.push_back(varWriter.asString());
		}

		const CVariant& varDirectors	= varEpisode["director"];
		for (unsigned int Idx = 0; Idx < varDirectors.size(); ++Idx)
		{
			const CVariant& varDirector	= varDirectors[Idx];
			stTVShowEpisode.vecDirectors.push_back(varDirector.asString());
		}

		const CVariant& varThumbnai		= varEpisode["thumbnail"];
		stTVShowEpisode.strThumbnail	= GetThumbUrl(varThumbnai.c_str());

		stTVShowEpisode.strPlot				= varEpisode["plot"].asString();
		stTVShowEpisode.strRunTime		= varEpisode["runtime"].asString();
		stTVShowEpisode.strFilePath		= varEpisode["file"].asString();
		stTVShowEpisode.strTitle			= varEpisode["title"].asString();
		stTVShowEpisode.strEpisodeID  = varEpisode["episodeid"].asString();
		stTVShowEpisode.strTVShowID		= varEpisode["tvshowid"].asString();
		stTVShowEpisode.strSeason			= varEpisode["season"].asString();
		stTVShowEpisode.strEpisode		= varEpisode["episode"].asString();

		vecTVShowEpisodes.push_back(stTVShowEpisode);
	}

	return true;
}

bool CProxyJSONRPC::GetTVShowEpisodes(int nTVShowID, std::vector<TVShowSeason>& vecTVShowSeasons)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;
	
	if (server.m_bLocal)
	{
		GetTVShowSeasons(nTVShowID, vecTVShowSeasons);

		for (int i = 0 ; i < vecTVShowSeasons.size() ; ++i)
		{
			GetTVShowEpisodes(nTVShowID, atoi(vecTVShowSeasons[i].strSeasonID.c_str()), vecTVShowSeasons[i].vecEpisodes);
		}
	}
	else
	{
		CVariant	varParams;

		varParams["idtvshow"]			= nTVShowID;

		CVariant varResult;
		bool bRet = MethodCall("VideoLibrary.GetEpisodes", varParams, varResult);
		if (!bRet) return false;

		const CVariant& varSeasons = varResult["seasons"];

		for (unsigned int i = 0 ; i < varSeasons.size() ; ++i)
		{
			TVShowSeason		stTVShowSeason;

			const CVariant& varSeason		= varSeasons[i];

			const CVariant& varEpisodes	= varSeason["episodes"];


			for (unsigned int j = 0 ; j < varEpisodes.size() ; ++j)
			{
				TVShowEpisode		stTVShowEpisode;

				const CVariant& varEpisode		= varEpisodes[j];

				CStdString strSeason					= varEpisode["season"].asString();

				stTVShowSeason.strTVShowID		= varEpisode["idtvshow"].asString();
				stTVShowSeason.strSeasonID		= strSeason;
				stTVShowSeason.strSeason		  = strSeason;

				CStdString strEpisode					= varEpisode["episode"].asString();
				if (strEpisode.Equals("-1"))
				{
					strEpisode									= varEpisode["disc"].asString();
				}

				if( server.m_bLocal )
				{
					stTVShowEpisode.strFileID		= varEpisode["tvshowid"].asString();
				}
				else
				{
					stTVShowEpisode.strFileID		= varEpisode["idfile"].asString();
				}

				stTVShowEpisode.strEpisodeID	= strEpisode;
				stTVShowEpisode.strTVShowID		= varEpisode["idtvshow"].asString();
				stTVShowEpisode.strSeasonID		= varEpisode["season"].asString();
				stTVShowEpisode.strKeyID			= varEpisode["idkey"].asString();
				stTVShowEpisode.strFilePath		= varEpisode["file"].asString();
				stTVShowEpisode.strThumbnail	= varEpisode["thumbnail"].asString();
				stTVShowEpisode.strTitle			= varEpisode["title"].asString();

				stTVShowSeason.vecEpisodes.push_back(stTVShowEpisode);
			}

			vecTVShowSeasons.push_back(stTVShowSeason);
		}
	}

	return true;
}

bool CProxyJSONRPC::GetEpisodeDetails(int nEpisodeID, EpisodeDetail& details)
{
  CVariant	varParams;

  // Properties: Movie content we need. 
  CVariant	varProperties;
  varProperties.push_back("resume");

  varParams["episodeid"]	= nEpisodeID;
  varParams["properties"]	= varProperties;

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.GetEpisodeDetails", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varEpisode	= varResult["episodedetails"];

  const CVariant& varResume	= varEpisode["resume"];
  details.resume.nPosition	= varResume["position"].asInteger();
  details.resume.nTotal		= varResume["total"].asInteger();

  return true;
}

bool CProxyJSONRPC::SetEpisodeDetails(int nEpisodeID, const CStdString& strLastPlayTime, double resumePosition)
{
  CVariant	varParams;

  varParams["episodeid"] = nEpisodeID;
  varParams["lastplayed"]	= strLastPlayTime;
  varParams["resume"]	= resumePosition;

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.SetEpisodeDetails", varParams, varResult);
  if (!bRet) return false;

  return true;
}

bool CProxyJSONRPC::GetServerPictureSources(std::vector<ServerPictureSource>& vecPicSources)
{
  Limits limits;

  return GetServerPictureSources(limits, vecPicSources);
}

bool CProxyJSONRPC::GetServerPictureSources(const Limits& limits, std::vector<ServerPictureSource>& vecPicSources)
{
  CVariant	varParams;
  varParams["media"]		= "pictures";

  CVariant	varLimits;
  if (limits.nEnd > 0)
  {
    varLimits["end"]		= limits.nEnd;
    varLimits["start"]	= limits.nStart;
    varParams["limits"]	= varLimits;
  }

  CVariant varResult;
  bool bRet = MethodCall("Files.GetSources", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varSources = varResult["sources"];
  for (unsigned int index = 0; index < varSources.size(); ++index)
  {
    ServerPictureSource		stSource;

    const CVariant& varSource	= varSources[index];

    stSource.strName				= varSource["label"].asString();
    stSource.strPath				= varSource["file"].asString();
    vecPicSources.push_back(stSource);
  }

  return true;
}

bool CProxyJSONRPC::GetServerPictures(std::string strDirectory, std::vector<ServerPictureInfo>& vecServerPictureInfo)
{
  CVariant	varParams;

  // Properties: Movie content we need. 
  CVariant	varProperties;
  varProperties.push_back("title");
  varProperties.push_back("file");
  varProperties.push_back("year");
  varProperties.push_back("lastmodified");
  varProperties.push_back("size");

  varParams["directory"]		= strDirectory;
  varParams["media"]				= "pictures";
  varParams["properties"]		= varProperties;

  CVariant varResult;
  bool bRet = MethodCall("Files.GetDirectory", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varFiles = varResult["files"];
  for (unsigned int index = 0; index < varFiles.size(); ++index)
  {
    ServerPictureInfo		stPicture;

    const CVariant& varFile	= varFiles[index];

    stPicture.strTitle				= varFile["label"].asString();
    stPicture.strFileType			= varFile["filetype"].asString();
    stPicture.strSize					= varFile["size"].asString();
    stPicture.strLastModified	= varFile["lastmodified"].asString();

    const CVariant& varPath		= varFile["file"].asString();
    if (stPicture.strFileType != "directory")
    {
      stPicture.strFilePath		= GetThumbUrl(varPath.c_str());
    }
    else
    {
      stPicture.strFilePath		= varFile["file"].asString();
    }

    vecServerPictureInfo.push_back(stPicture);
  }

  return true;
}

bool CProxyJSONRPC::GetServerVideo(std::vector<ServerVideoInfo>& vecServerVideoInfo)
{
  Limits limits;

  return GetServerVideo(limits, vecServerVideoInfo);
}

bool CProxyJSONRPC::GetServerVideo(const Limits& limits, std::vector<ServerVideoInfo>& vecServerVideoInfo)
{
  CVariant	varParams;

  CVariant	varProperties;
  varProperties.push_back("file");
  varProperties.push_back("thumbnail");
  varProperties.push_back("title");

  varParams["properties"]	= varProperties;

  CVariant	varLimits;
  if (limits.nEnd > 0)
  {
    varLimits["end"]			= limits.nEnd;
    varLimits["start"]		= limits.nStart;
    varParams["limits"]		= varLimits;
  }

  CVariant varResult;

  std::string strMethodName;

  VDMServer server;
  if (VDMUtils::Instance().GetCurrentServer(server) && server.m_bLocal)
  {
    strMethodName = "VideoLibrary.GetMusicVideos";
  }
  else
  {
    strMethodName = "VideoLibrary.GetPrivVideos";
  }

  bool bRet = MethodCall(strMethodName, varParams, varResult);
  if (!bRet) return false;

  std::string strResult = server.m_bLocal ? "musicvideos" : "privvideos";

  const CVariant& varPrivVideos = varResult[strResult];
  for (unsigned int index = 0; index < varPrivVideos.size(); ++index)
  {
    ServerVideoInfo		stVideoInfo;

    const CVariant& varVideo	= varPrivVideos[index];

		stVideoInfo.strVideoID		= varVideo["idfile"].asString();
		stVideoInfo.strKeyID			= varVideo["idkey"].asString();

    const CVariant& varThumbnai			= varVideo["thumbnail"];
    stVideoInfo.strThumbnail				= GetThumbUrl(varThumbnai.c_str());

    stVideoInfo.strFilePath					= varVideo["file"].asString();
    stVideoInfo.strTitle						= varVideo["title"].asString();

    vecServerVideoInfo.push_back(stVideoInfo);
  }

  return true;
}

bool CProxyJSONRPC::GetVideoDetails(int nVideoID, VideoDetail& details)
{
  CVariant	varParams;

  // Properties: Movie content we need. 
  CVariant	varProperties;
  varProperties.push_back("resume");

  varParams["privvideoid"]	= nVideoID;
  varParams["properties"]		= varProperties;

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.GetPrivVideoDetails", varParams, varResult);
  if (!bRet) return false;

  const CVariant& varEpisode	= varResult["privvideodetails"];

  const CVariant& varResume	= varEpisode["resume"];
  details.resume.nPosition	= varResume["position"].asInteger();
  details.resume.nTotal		= varResume["total"].asInteger();

  return true;
}

bool CProxyJSONRPC::SetVideoDetails(int nVideoID, const CStdString& strLastPlayTime, double resumePosition)
{
  CVariant	varParams;

  varParams["privvideoid"] = nVideoID;
  varParams["lastplayed"]	= strLastPlayTime;
  varParams["resume"]	= resumePosition;

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.SetPrivVideoDetails", varParams, varResult);
  if (!bRet) return false;

  return true;
}

bool CProxyJSONRPC::GetFileMetaFromPath(const CStdString& strPath, ServerMetaInfo& meta)
{
  CStdString strFullPath = strPath;
  URIUtils::RemoveSlashAtEnd(strFullPath);

  CStdString strFileName;
  CStdString strExtention;

  strFileName = URIUtils::GetFileName(strFullPath);
  if (!strFileName.IsEmpty())
  {
    strExtention = URIUtils::GetExtension(strFileName);
  }

  if (strExtention.IsEmpty())
  {
    if (!XFILE::CDirectory::Exists(strFullPath))
    {
      return false;
    }

    if (!VDMUtils::IsProbableBlurayFolder(strFullPath) && !VDMUtils::IsProbableDVDFolder(strFullPath))
    {
      return false;
    }      
  }

  if (!strExtention.IsEmpty())
  {
    if (strExtention.CompareNoCase(".iso") != 0)
    {
      if (VDMUtils::IsProbableBlurayFile(strFullPath) || VDMUtils::IsProbableDVDFile(strFullPath))
      {
        URIUtils::GetParentPath(strPath, strFullPath);
      }
      else
      {
        strExtention.erase(0, 1);//remove '.'
        meta.fileFormat = strExtention;
        return true;
      }
    }
  }

  if (CVDMPlayer::AddPlaySource(strFullPath.c_str()))
  {
    CVDMPlayer::PlaySourceMetaInfo rawMetaInfo;
    CVDMPlayer::GetPlaySourceMetaInfo(strFullPath.c_str(), rawMetaInfo);

    meta.fileFormat = rawMetaInfo.strFileFormat;
    meta.videoCodec = rawMetaInfo.strVideoCodec;
    meta.videoResolution = rawMetaInfo.strVideoResolution;
    meta.audioCodec = rawMetaInfo.strAudioCodec;
    meta.audioCodec.TrimRight(',');
    meta.audioCodec.TrimRight(';');
    meta.audioChannel = rawMetaInfo.strAudioChannel;
    meta.subtitleCodec = rawMetaInfo.strSubtitleCodec;   
  } 
  return true;
}

bool CProxyJSONRPC::GetMetaFromPath(const CStdString& strPath, ServerMetaInfo& meta)
{
  VDMServer server;

  if( !VDMUtils::Instance().GetCurrentServer(server) )
    return false;

  if( server.m_bLocal )
  {
#if 1
    CStdString strFullPath = strPath;
    URIUtils::RemoveSlashAtEnd(strFullPath);

    CStdString strFileName;
    CStdString strExtention;
    
    strFileName = URIUtils::GetFileName(strFullPath);
    if (!strFileName.IsEmpty())
    {
      strExtention = URIUtils::GetExtension(strFileName);
    }
    
    if (strExtention.IsEmpty())
    {
      if (!XFILE::CDirectory::Exists(strFullPath))
      {
        return false;
      }

      if (!VDMUtils::IsProbableBlurayFolder(strFullPath) && !VDMUtils::IsProbableDVDFolder(strFullPath))
      {
        return false;
      }      
    }

    if (!strExtention.IsEmpty())
    {
      if (strExtention.CompareNoCase(".iso") != 0)
      {
        if (VDMUtils::IsProbableBlurayFile(strFullPath) || VDMUtils::IsProbableDVDFile(strFullPath))
        {
          URIUtils::GetParentPath(strPath, strFullPath);
        }
        else
        {
          strExtention.erase(0, 1);//remove '.'
          meta.fileFormat = strExtention;
          return true;
        }
      }
    }
    
    if (CVDMPlayer::AddPlaySource(strFullPath.c_str()))
    {
      CVDMPlayer::PlaySourceMetaInfo rawMetaInfo;
      CVDMPlayer::GetPlaySourceMetaInfo(strFullPath.c_str(), rawMetaInfo);

      meta.fileFormat = rawMetaInfo.strFileFormat;
      meta.videoCodec = rawMetaInfo.strVideoCodec;
      meta.videoResolution = rawMetaInfo.strVideoResolution;
      meta.audioCodec = rawMetaInfo.strAudioCodec;
      meta.audioCodec.TrimRight(',');
      meta.audioCodec.TrimRight(';');
      meta.audioChannel = rawMetaInfo.strAudioChannel;
      meta.subtitleCodec = rawMetaInfo.strSubtitleCodec;   
    }    
#else
    CStdString ext = URIUtils::GetExtension(strPath);
    ext.ToLower();

    if (ext == ".iso" ||  ext == ".img")
    {
      if (!GetLocalMetaBluray(strPath, meta))
      {
        return GetLocalMetaDVD(strPath, meta);
      }
    }
#endif
    return true;
  }

  CVariant params;
  params["path"] = strPath.c_str();

  CVariant result;
  MethodCall("VidOnMe.GetMetaFromPath", params, result);
  const bool ret = result["ret"].asBoolean();
  if (!ret) return false;

	const CVariant& varMeta = result["meta"];
	meta.type = varMeta["type"].asString();

	if( server.m_bLocal )
	{
		const CVariant& varDetails			= varMeta["details"];
		meta.fileFormat         = varDetails["FileFormat"].asString();
		meta.videoCodec         = varDetails["VideoCodec"].asString();
		meta.videoResolution    = varDetails["VideoResolution"].asString();
		meta.audioCodec         = varDetails["AudioCodec"].asString();
		meta.audioChannel       = varDetails["AudioChannel"].asString();
		meta.subtitleCodec      = varDetails["SubtitleCodec"].asString();
	}
	else
	{
		const CVariant& varData = varMeta["data"];

		const CVariant& varDetails			= varData["details"];
		meta.fileFormat         = varDetails["FileFormat"].asString();
		meta.videoCodec         = varDetails["VideoCodec"].asString();
		meta.videoResolution    = varDetails["VideoResolution"].asString();
		meta.audioCodec         = varDetails["AudioCodec"].asString();
		meta.audioChannel       = varDetails["AudioChannel"].asString();
		meta.subtitleCodec      = varDetails["SubtitleCodec"].asString();
	}

  return true;
}
bool CProxyJSONRPC::GetFilePlayListsFromPath(const CStdString& strPath, ServerPlaylists& serverPlaylists)
{
  CStdString strFullPath = strPath;
  URIUtils::RemoveSlashAtEnd(strFullPath);

  CStdString strFileName;
  CStdString strExtention;

  strFileName = URIUtils::GetFileName(strFullPath);
  if (!strFileName.IsEmpty())
  {
    strExtention = URIUtils::GetExtension(strFileName);
  }

  if (strExtention.IsEmpty())
  {
    if (!XFILE::CDirectory::Exists(strFullPath))
    {
      return false;
    }

    if (!VDMUtils::IsProbableBlurayFolder(strFullPath) && !VDMUtils::IsProbableDVDFolder(strFullPath))
    {
      return false;
    }      
  }

  if (!strExtention.IsEmpty())
  {
    if (strExtention.CompareNoCase(".iso") != 0)
    {
      if (VDMUtils::IsProbableBlurayFile(strFullPath) || VDMUtils::IsProbableDVDFile(strFullPath))
      {
        URIUtils::GetParentPath(strPath, strFullPath);
      }
      else
      {
        return false;
      }
    }
  }

  if (CVDMPlayer::AddPlaySource(strFullPath.c_str()))
  {
    CVDMPlayer::Playlists playlists = CVDMPlayer::GetPlaySourcePlaylists(strFullPath.c_str());
    for (int i = 0; i < playlists.size(); i++)
    {
      ServerPlaylist title;

      title.strPath	= playlists[i]->strPlayPath;
      CStdString strRuntime;
      strRuntime.Format("%d", playlists[i]->nDurationMs / 1000);
      title.strRunTime = strRuntime;
      title.bIsMainTitle = playlists[i]->bMainMovie;
      title.b3D = playlists[i]->b3D;
      title.nAngles = playlists[i]->nAngles;
      serverPlaylists.push_back(title);
    }
    return true;
  }
  else
  {
    return false;
  }
}
bool CProxyJSONRPC::GetPlayListsFromPath(const CStdString& strPath, ServerPlaylists& serverPlaylists)
{
  serverPlaylists.clear();

  VDMServer server;
  if( !VDMUtils::Instance().GetCurrentServer(server) )
    return false;

  if( server.m_bLocal )
  {
#if 1
    CStdString strFullPath = strPath;
    URIUtils::RemoveSlashAtEnd(strFullPath);

    CStdString strFileName;
    CStdString strExtention;

    strFileName = URIUtils::GetFileName(strFullPath);
    if (!strFileName.IsEmpty())
    {
      strExtention = URIUtils::GetExtension(strFileName);
    }

    if (strExtention.IsEmpty())
    {
      if (!XFILE::CDirectory::Exists(strFullPath))
      {
        return false;
      }

      if (!VDMUtils::IsProbableBlurayFolder(strFullPath) && !VDMUtils::IsProbableDVDFolder(strFullPath))
      {
        return false;
      }      
    }

    if (!strExtention.IsEmpty())
    {
      if (strExtention.CompareNoCase(".iso") != 0)
      {
        if (VDMUtils::IsProbableBlurayFile(strFullPath) || VDMUtils::IsProbableDVDFile(strFullPath))
        {
          URIUtils::GetParentPath(strPath, strFullPath);
        }
        else
        {
          return false;
        }
      }
    }

    if (CVDMPlayer::AddPlaySource(strFullPath.c_str()))
    {
      CVDMPlayer::Playlists playlists = CVDMPlayer::GetPlaySourcePlaylists(strFullPath.c_str());
      for (int i = 0; i < playlists.size(); i++)
      {
        ServerPlaylist title;

        title.strPath	= playlists[i]->strPlayPath;
        CStdString strRuntime;
        strRuntime.Format("%d", playlists[i]->nDurationMs / 1000);
        title.strRunTime = strRuntime;
        title.bIsMainTitle = playlists[i]->bMainMovie;
        title.b3D = playlists[i]->b3D;
        title.nAngles = playlists[i]->nAngles;
#if 0
        //temporarily, or play playlist will crash when snapshot.
        CStdString strThumb;
        CProxyJSONRPC::GetExtractThumb(title.strPath.c_str(), strThumb);
        title.strThumbnail = strThumb;
#endif
        serverPlaylists.push_back(title);
      }

      return true;
    }
    else
    {
      return false;
    }
#else
    CStdString strBlurayDirectory;

    CFileItem item(strPath, false);

    if( item.IsBDFile() )
    {
      CStdString root = URIUtils::GetParentPath(item.GetPath());
      URIUtils::RemoveSlashAtEnd(root);
      if(URIUtils::GetFileName(root) == "BDMV")
      {
        CURL url("bluray://");
        url.SetHostName(URIUtils::GetParentPath(root));
        url.SetFileName("titles");
        strBlurayDirectory = url.Get();
      }
    }

    CStdString ext = URIUtils::GetExtension(item.GetPath());
    ext.ToLower();
    if (ext == ".iso" ||  ext == ".img")
    {
      CURL url2("udf://");
      url2.SetHostName(item.GetPath());
      url2.SetFileName("BDMV/index.bdmv");
      if (XFILE::CFile::Exists(url2.Get()))
      {
        url2.SetFileName("");

        CURL url("bluray://");
        url.SetHostName(url2.Get());
        url.SetFileName("titles");
        strBlurayDirectory = url.Get();
      }
    }

    if( !strBlurayDirectory.IsEmpty() )
    {
      CFileItemList items;

      if (!XFILE::CDirectory::GetDirectory(strBlurayDirectory, items, XFILE::CDirectory::CHints(), true))
        return false;

      //TODO, fill the playlist information 
      for (unsigned int index = 0; index < items.Size(); index++)
      {
        ServerPlaylist title;

        CFileItemPtr playitem = items[index];

        CURL fileUrl(playitem->GetPath());
        const CStdString strFileNameAndPath = URIUtils::AddFileToFolder(
          fileUrl.GetHostName(), fileUrl.GetFileName());

        title.strPath	= strFileNameAndPath;
        CStdString strRuntime;
        strRuntime.Format("%d", playitem->GetVideoInfoTag()->m_duration);
        title.strRunTime = strRuntime;
        title.bIsMainTitle = playitem->GetProperty("IsMainTitle").asBoolean();
      }

      return true;
    }
    else
    {
      return false;
    }
#endif
  }

  CVariant params = CVariant::VariantTypeObject;
  params["path"] = strPath.c_str();

	CVariant result;
	MethodCall("VidOnMe.GetMetaFromPath", params, result);
	const bool ret = result["ret"].asBoolean();
	if (!ret) return false;

	const CVariant& varMeta			= result["meta"];
	const CVariant& varData			= varMeta["data"];
  const CVariant& varPlaylist	= varData["playlist"];

  for (unsigned int index = 0; index < varPlaylist.size(); index++)
  {
    ServerPlaylist title;
    const CVariant& varPlayitem = varPlaylist[index];

    title.strPath				= varPlayitem["file"].asString();
    title.strRunTime		= varPlayitem["runtime"].asString();
    title.bIsMainTitle	= varPlayitem["IsMainTitle"].asBoolean();

    serverPlaylists.push_back(title);
  }
  return true;
}

bool CProxyJSONRPC::GetFileExtractThumb(const CStdString& strPath, CStdString& strThumb)
{
  CURL::Decode(strPath);
  CStdString strSourcePath = strPath;
  CStdString strFileName, strParentPath;
  URIUtils::Split(strPath, strParentPath, strFileName);

  strFileName.MakeLower();
  CStdString strExtension = URIUtils::GetExtension(strFileName);

  int playlist = -1;

  if(strExtension.Equals(".title") && strFileName.GetLength() == 11)
  {      
    int i = 0;
    for (; i < 5; i++)
    {
      if (strFileName.GetAt(i) < '0' || strFileName.GetAt(i) > '9')
      {
        break;
      }
    }

    if (i == 5)
    {
      sscanf(strFileName.c_str(), "%05d.title", &playlist);
    }

    strSourcePath = strParentPath;
  }
  else if( ".mpls" == strExtension  && strFileName.GetLength() == 10)
  {
    int i = 0;
    for (; i < 5; i++)
    {
      if (strFileName.GetAt(i) < '0' || strFileName.GetAt(i) > '9')
      {
        break;
      }
    }

    if (i == 5)
    {
      sscanf(strFileName.c_str(), "%05d.title", &playlist);
    }

    CStdString strPath;

    URIUtils::RemoveSlashAtEnd(strParentPath);
    if (strParentPath.Right(8).CompareNoCase("PLAYLIST") == 0)
    {
      URIUtils::GetParentPath(strParentPath, strPath);		//remove PLAYLIST
    }

    strParentPath = strPath;
    URIUtils::RemoveSlashAtEnd(strParentPath);
    if (strParentPath.Right(4).CompareNoCase("BDMV") == 0)
    {
      URIUtils::GetParentPath(strParentPath, strPath);		//remove BDMV
    }

    if (!strPath.IsEmpty())
    {
      strSourcePath = strPath;
    }
  }

  URIUtils::RemoveSlashAtEnd(strSourcePath);

  if (CVDMPlayer::AddPlaySource(strSourcePath.c_str()))
  {
    strThumb = CVDMPlayer::GetPlaySourceThumbnail(strSourcePath.c_str(), playlist);
  }
  return true;
}

bool CProxyJSONRPC::GetExtractThumb(const CStdString& strPath, CStdString& strThumb)
{
  CVariant params;
  params["path"] = strPath;

	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( server.m_bLocal )
	{
    CURL::Decode(strPath);
    CStdString strSourcePath = strPath;
    CStdString strFileName, strParentPath;
    URIUtils::Split(strPath, strParentPath, strFileName);

    strFileName.MakeLower();
    CStdString strExtension = URIUtils::GetExtension(strFileName);

    int playlist = -1;

    if(strExtension.Equals(".title") && strFileName.GetLength() == 11)
    {      
      int i = 0;
      for (; i < 5; i++)
      {
        if (strFileName.GetAt(i) < '0' || strFileName.GetAt(i) > '9')
        {
          break;
        }
      }
      
      if (i == 5)
      {
        sscanf(strFileName.c_str(), "%05d.title", &playlist);
      }

      strSourcePath = strParentPath;
    }
    else if( ".mpls" == strExtension  && strFileName.GetLength() == 10)
    {
      int i = 0;
      for (; i < 5; i++)
      {
        if (strFileName.GetAt(i) < '0' || strFileName.GetAt(i) > '9')
        {
          break;
        }
      }

      if (i == 5)
      {
        sscanf(strFileName.c_str(), "%05d.title", &playlist);
      }

      CStdString strPath;

      URIUtils::RemoveSlashAtEnd(strParentPath);
      if (strParentPath.Right(8).CompareNoCase("PLAYLIST") == 0)
      {
        URIUtils::GetParentPath(strParentPath, strPath);		//remove PLAYLIST
      }

      strParentPath = strPath;
      URIUtils::RemoveSlashAtEnd(strParentPath);
      if (strParentPath.Right(4).CompareNoCase("BDMV") == 0)
      {
        URIUtils::GetParentPath(strParentPath, strPath);		//remove BDMV
      }
       
      if (!strPath.IsEmpty())
      {
        strSourcePath = strPath;
      }
    }
    
    //zydding: 20130911, I think not need do this
    //liuhongfei:20130913, we need this to get currect thumbnail
    URIUtils::RemoveSlashAtEnd(strSourcePath);
    
		if (CVDMPlayer::AddPlaySource(strSourcePath.c_str()))
		{
      strThumb = CVDMPlayer::GetPlaySourceThumbnail(strSourcePath.c_str(), playlist);
		}
	}
	else
	{
		CVariant result;
		bool bRet = MethodCall("VidOnMe.GetExtractThumb", params, result);
		if (bRet)
		{
			bRet = result["ret"].asBoolean();
		}

		if (!bRet)
		{ 
			return false;
		}

		strThumb = GetThumbUrl(result["thumb"].asString());
	}

  return true;
}

bool CProxyJSONRPC::SetServerMovieDetails(int nMovieID, const CStdString& strLastPlayTime, double resumePosition)
{
  CVariant	varParams;

  varParams["movieid"]			= nMovieID;
  varParams["lastplayed"]		= strLastPlayTime;
  varParams["resume"]	= resumePosition;

  CVariant varResult;
  bool bRet = MethodCall("VideoLibrary.SetMovieDetails", varParams, varResult);
  if (!bRet) return false;

  return true;
}

bool CProxyJSONRPC::RemoveServerMovie(int nMovieID)
{
	CVariant	varParams;

	varParams["movieid"]			= nMovieID;

	CVariant varResult;
	bool bRet = MethodCall("VideoLibrary.RemoveMovie", varParams, varResult);
	if (!bRet) return false;

	return true;
}

bool CProxyJSONRPC::RemoveEpisode(int nEpisodeID)
{
	CVariant	varParams;

	varParams["episodeid"]			= nEpisodeID;

	CVariant varResult;
	bool bRet = MethodCall("VideoLibrary.RemoveEpisode", varParams, varResult);
	if (!bRet) return false;

	return true;
}

bool CProxyJSONRPC::RemoveServerVideo(int nVideoID)
{
	CVariant	varParams;

	varParams["privvideoid"]			= nVideoID;

	CVariant varResult;
	bool bRet = MethodCall("VideoLibrary.RemovePrivVideo", varParams, varResult);
	if (!bRet) return false;

	return true;
}

#if !defined(TARGET_ANDROID)
bool CProxyJSONRPC::CreateSessionClient(OUT SessionClientID& sessionClientId)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( !server.m_bLocal )
	{
		CVariant params = CVariant::VariantTypeArray;

		CVariant vResult;
		bool bRet = MethodCall("VidOnMe.CreateSessionClient", params, vResult);
		if (!bRet) return false;

		sessionClientId = vResult["sessionClientId"].asInteger();

		return true;
	}
	
	return false;
}

bool CProxyJSONRPC::DestroySessionClient(const SessionClientID sessionClientId)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( !server.m_bLocal )
	{
		CVariant params = CVariant::VariantTypeObject;
		params["sessionClientId"] = sessionClientId;

		CVariant vResult;
		bool bRet = MethodCall("VidOnMe.DestroySessionClient", params, vResult);
		if (!bRet) return false;
	}

	return true;
}

bool CProxyJSONRPC::SessionClientSubscribe(const SessionClientID sessionClientId, const NotifyKind kind)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( !server.m_bLocal )
	{
		CVariant params = CVariant::VariantTypeObject;
		params["sessionClientId"] = sessionClientId;
		params["kind"] = TranslateNotifyKind(kind);
		CVariant vResult;
		bool bRet = MethodCall("VidOnMe.SessionClientSubscribe", params, vResult);
		if (!bRet) return false;
	}

	return true;
}

bool CProxyJSONRPC::SessionClientUnsubscribe(const SessionClientID sessionClientId, const NotifyKind kind)
{
	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) )
		return false;

	if( !server.m_bLocal )
	{
		CVariant params = CVariant::VariantTypeObject;
		params["sessionClientId"] = sessionClientId;
		params["kind"] = TranslateNotifyKind(kind);
		CVariant vResult;
		bool bRet = MethodCall("VidOnMe.SessionClientUnsubscribe", params, vResult);
		if (!bRet) return false;
	}

	return true;
}
#endif

bool CProxyVideoLibrary::MethodCall(const std::string& method, const CVariant& params, CVariant& result)
{
  return CProxyJSONRPC::MethodCall("VideoLibrary." + method, params, result);
}

const char* CProxyVideoLibrary::TranslateType(const BaseType type)
{
  switch (type)
  {
  case Movie: return "movie";
  case TvShow: return "tvshow";
  case MusicVideo: return "musicvideo";
  }
  assert(false);
  return NULL;
}
void CProxyVideoLibrary::ToInfoList(const CVariant& varItems, const char* const idname, OUT BaseInfoList& items)
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
bool CProxyVideoLibrary::GetCountries(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetStudios(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetDirectors(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetYears(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetActors(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetTags(const BaseType type, OUT BaseInfoList& items)
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

bool CProxyVideoLibrary::GetMusicVideoAlbums(OUT BaseInfoList& items)
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


}

#endif