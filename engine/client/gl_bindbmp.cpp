/*
gl_studiolru.h
Copyright (C) 2023 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "common.h"
#include "client.h"
#include "gl_local.h"
#include "gl_bindbmp.h"


#include "gl_studioshader.h"
#include "../../3rdparty/json/single_include/nlohmann/json.hpp"

std::vector<BMPList> g_vecBindBMPList;

static CBindBMPManager g_BMPBindManager;

CBindBMPManager& BindBMPManager(void)
{
	return g_BMPBindManager;
}

void CBindBMPManager::ReadSprInfo(cl_entity_s* ent, studiohdr_t* header, std::string sprFile, int texIndex, int flag, float framerate, float starttime, float endtime)
{
	BMPList SprInfo;
	SprInfo.pHeader = header;
	SprInfo.iFlag = flag;
	SprInfo.pEntModel = ent;
	SprInfo.flStartTime = starttime;
	SprInfo.flEndTime = endtime;
	SprInfo.iTexIndex = texIndex;

	if (m_vecCachedSpriteJson.count(sprFile))
	{
		SprInfo.vecTexName = m_vecCachedSpriteJson[sprFile];
	}
	else
	{
		fs_offset_t length;

		nlohmann::json jsSPR;

		auto data = FS_MapFile(sprFile.c_str(), &length, true);
		if (data)
		{

			jsSPR = nlohmann::json::parse(std::string_view((const char*)data, length));
			FS_MapFree(data, length);

			std::vector<std::string> vecTexName;
			for (auto& SPR : jsSPR)
			{
				vecTexName.push_back(SPR.get<std::string>());
			}
			
			SprInfo.vecTexName = vecTexName;
		
			m_vecCachedSpriteJson.insert(std::pair<std::string, std::vector<std::string>>(sprFile, vecTexName));
		}
	}

	if (flag)
	{
		SprInfo.flFrameRate = float(SprInfo.vecTexName.size() / (endtime - starttime));
	}
	else
	{
		SprInfo.flFrameRate = framerate;
	}


	g_vecBindBMPList.push_back(SprInfo);
	
	

}

void CBindBMPManager::Vidinit(void)
{
	g_vecBindBMPList.clear();
}
