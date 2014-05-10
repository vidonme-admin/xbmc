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

#pragma once

#include "TextureDatabase.h"

struct structTextureDetails
{
	int nTextureID;
	int nWidth;
	int nHeight;
	CStdString strLastUseTime;
	CStdString strLasthashcheck;
	CStdString strUrl;
	CStdString strCachedUrl;
	CStdString strImageHash; 
};

class CTextureDatabaseEx : public CTextureDatabase
{
public:
	CTextureDatabaseEx();
	virtual ~CTextureDatabaseEx();
public:
	bool QueryAllTexture();
	uint32_t GetAllTextureSize(){return m_vecTextures.size();};
	//nIndex: 0/1/2/....
	bool GetAllTextAt(uint32_t nIndex,structTextureDetails &details);
private:
	bool m_bQuery;
	std::vector<structTextureDetails> m_vecTextures;
};
