
#if defined(__VIDONME_MEDIACENTER__)

#pragma once

#include "interfaces/json-rpc/ProxyJSONRPC.h"
#include "interfaces/json-rpc/SessionClientConnection.h"

class CVDMSessionClient
{
public:
	CVDMSessionClient(void);
	virtual ~CVDMSessionClient(void);

	bool Init();
	bool Deinit();

private:
	SessionClientID							m_nSessionID;
	std::vector<NotifyKind>			m_vecNotifyKind;
	SessionClientConnectionPtr	m_pClient;
};


#endif