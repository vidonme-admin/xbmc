/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "TextureDatabaseEx.h"
#include "utils/log.h"
#include "XBDateTime.h"
#include "dbwrappers/dataset.h"
#include "URL.h"


CTextureDatabaseEx::CTextureDatabaseEx()
{
	m_bQuery = false;
}

CTextureDatabaseEx::~CTextureDatabaseEx()
{
}

bool CTextureDatabaseEx::QueryAllTexture()
{
	int iReturn = 0;
	try
	{
		if (NULL == m_pDB.get()) return false;
		if (NULL == m_pDS.get()) return false;

		CStdString sql = PrepareSQL("SELECT id,url,cachedurl,imagehash,lasthashcheck,width, height,lastusetime FROM texture JOIN sizes ON (texture.id=sizes.idtexture AND sizes.size=1) order by lastusetime Asc");
		m_pDS->query(sql.c_str());

		m_vecTextures.clear();
		m_bQuery = false;
		while (!m_pDS->eof())
		{
			structTextureDetails TextureDetails;
			TextureDetails.nTextureID		= m_pDS->fv(0).get_asInt();
			TextureDetails.strUrl			= m_pDS->fv(1).get_asString();
			TextureDetails.strCachedUrl		= m_pDS->fv(2).get_asString();
			TextureDetails.strImageHash		= m_pDS->fv(3).get_asString();
			TextureDetails.strLasthashcheck	= m_pDS->fv(4).get_asString();
			TextureDetails.nWidth			= m_pDS->fv(5).get_asInt();
			TextureDetails.nHeight			= m_pDS->fv(6).get_asInt();
			TextureDetails.strLastUseTime	= m_pDS->fv(7).get_asString();

			m_vecTextures.push_back(TextureDetails);
			iReturn++;
			m_pDS->next();
		}
		m_pDS->close();
	}
	catch (...)
	{
		CLog::Log(LOGERROR, "%s, failed ", __FUNCTION__);
	}
	if(iReturn)
		m_bQuery = true;
	return true;
}

bool CTextureDatabaseEx::GetAllTextAt(uint32_t nIndex,structTextureDetails &details)
{
	if(!m_bQuery)return false;
	if(nIndex>nIndex+1 > m_vecTextures.size()) return false;
	details = m_vecTextures[nIndex];
	return true;
}
