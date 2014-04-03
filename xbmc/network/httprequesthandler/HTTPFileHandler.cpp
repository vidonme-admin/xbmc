/*
 *      Copyright (C) 2012-2013 Team XBMC
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

#include "HTTPFileHandler.h"
#include "network/WebServer.h"
#include "URL.h"
#include "filesystem/ImageFile.h"

using namespace std;

bool CHTTPFileHandler::CheckHTTPRequest(const HTTPRequest &request)
{
  return (request.url.find("/file/") == 0);
}

int CHTTPFileHandler::HandleHTTPRequest(const HTTPRequest &request)
{
	if (request.url.size() > 6)
	{
		m_path = request.url.substr(6);
		CURL::Decode(m_path);

		if (XFILE::CFile::Exists(m_path) )
    {
      m_responseCode = MHD_HTTP_OK;
      m_responseType = HTTPFileDownload;
    }
    else
    {
      m_responseCode = MHD_HTTP_NOT_FOUND;
      m_responseType = HTTPError;
    }
  }
  else
  {
    m_responseCode = MHD_HTTP_BAD_REQUEST;
    m_responseType = HTTPError;
  }

  return MHD_YES;
}
