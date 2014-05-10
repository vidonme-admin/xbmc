
#if defined(__VIDONME_MEDIACENTER__)

#include "utils/log.h"
#include "VDMUtils.h"
#include "VDMSessionClient.h"

using namespace VidOnMe;


CVDMSessionClient::CVDMSessionClient(void) 
{
	m_nSessionID			= 0;

	m_vecNotifyKind.push_back(nkTest1);
	m_vecNotifyKind.push_back(nkTest2);
	m_vecNotifyKind.push_back(nkUpdateVideoLibrary);
	m_vecNotifyKind.push_back(nkScanDirectory);
	m_vecNotifyKind.push_back(nkAnalysisFile);
	m_vecNotifyKind.push_back(nkScraperFile);
}

CVDMSessionClient::~CVDMSessionClient(void)
{
	Deinit();
}

bool CVDMSessionClient::Deinit()
{
	m_pClient.reset();

	if (m_nSessionID > 0)
	{
		JSONRPC::CProxyJSONRPC::DestroySessionClient(m_nSessionID);
	}

	m_nSessionID = 0;

	return true;
}

bool CVDMSessionClient::Init()
{
	Deinit();

	VDMServer server;
	if( !VDMUtils::Instance().GetCurrentServer(server) || server.m_bLocal)
		return false;


	if (!JSONRPC::CProxyJSONRPC::CreateSessionClient(m_nSessionID))
		return false;

	for (int i = 0 ; i < m_vecNotifyKind.size() ; ++i)
	{
		if (!JSONRPC::CProxyJSONRPC::SessionClientSubscribe(m_nSessionID, m_vecNotifyKind[i]))
		{
			JSONRPC::CProxyJSONRPC::DestroySessionClient(m_nSessionID);
			m_nSessionID = 0;

			return false;
		}
	}

	m_pClient.reset(new CSessionClientConnection());
	if (!m_pClient->ContentTCP(server.m_strIP, server.m_nEventPort))
		return false;

	return m_pClient->SessionClientConnect(m_nSessionID);
}


#endif