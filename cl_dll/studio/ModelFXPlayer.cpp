/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// new alarm
//
#include <assert.h>
#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "dlight.h"
#include "triangleapi.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>

#include "studio_util.h"
#include "r_studioint.h"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include <client/gl_export.h>
#include "bonequatlru.h"

#include "event_api.h"
#include "pm_defs.h"
#include "pm_math.h"
#include "pm_shared.h"

#include "com_weapons.h"
#include "ModelFXPlayer.h"

namespace cl {

	extern engine_studio_api_t IEngineStudio;

	void CModelFXPlayer::ReadTempModelFXInfo(std::string szFile)
	{
		if (m_vecCachedModelFXJson.count(szFile))
		{
			cl_entity_s* viewent = gEngfuncs.GetViewModel();

			std::string vmodel = viewent->model->name;

			std::string modelname = vmodel.substr(vmodel.find_last_of('/') + 1);

			std::vector<ModelFXInfo> info = m_vecCachedModelFXJson[szFile];

			for (auto iter = info.begin(); iter != info.end(); iter++)
			{
				iter->flStartTime = gHUD.m_flTime + iter->flStartTime;
				iter->flEndTime = gHUD.m_flTime + iter->flEndTime;
			}

			m_vecTempModelFX.push_back(std::pair<std::string, std::vector<ModelFXInfo>>(modelname, info));
		}
		else
		{
			fs_offset_t length;

			nlohmann::json jsModelFX;

			auto data = gFileSystemAPI.FS_MapFile(szFile.c_str(), &length, true);
			if (data)
			{
				jsModelFX = nlohmann::json::parse(std::string_view((const char*)data, length));
				gFileSystemAPI.FS_MapFree(data, length);

				std::string szName = jsModelFX["Name"].get<std::string>();
				std::vector<ModelFXInfo> vecModelFX;
				for (auto& ModelFX : jsModelFX["ModelFX"])
				{
					
					std::string szBoneName = ModelFX["AttachBone"].get<std::string>();
					std::string szModelName = ModelFX["Model"].get<std::string>();

					float flStartTime = ModelFX["StartTime"].get<float>();
					float flEndTime = ModelFX["EndTime"].get<float>();
					float flFrameRate = ModelFX["FrameRate"].get<float>();
					int rendermode = ModelFX["RenderMode"].get<int>();
					int renderfx = ModelFX["RenderFX"].get<int>();
					int sequence = ModelFX["Sequence"].get<int>();
					int iFlag = ModelFX["Flag"].get<int>();
					int iMergeType = ModelFX["MergeType"].get<int>();

					std::vector<Color> vecColorKeyFrame;
					for (const auto& row : ModelFX["ColorKeyFrame"]) {
						Color rowData;
						rowData.rate = row[0];
						rowData.r = row[1];
						rowData.g = row[2];
						rowData.b = row[3];
						rowData.a = row[4];
						vecColorKeyFrame.push_back(rowData);
					}

					std::vector<SpriteSkin> vecSpriteSkin;
					for (const auto& row : ModelFX["SpriteSkin"])
					{
						SpriteSkin rowData;
						rowData.iTex = row["TexIndex"].get<int>();
						rowData.szSprName = row["TexName"].get<std::string>();
						vecSpriteSkin.push_back(rowData);
					}

					vecModelFX.emplace_back();
					ModelFXInfo& Temp = vecModelFX.back();
					
					Temp.szBoneName = szBoneName;
					Temp.szFXModelName = szModelName;
					
					s_pFXModel[szModelName] = IEngineStudio.Mod_ForName(szModelName.c_str(), FALSE);
					Temp.flStartTime = flStartTime;
					Temp.flEndTime = flEndTime;
					Temp.rendermode = rendermode;
					Temp.renderfx = renderfx;
					Temp.iSequence = sequence;
					Temp.flFrameRate = flFrameRate;
					Temp.iFlag = iFlag;
					Temp.vecColorKeyFrame = vecColorKeyFrame;
					Temp.iMergeType = iMergeType;
					Temp.vecSpriteSkin = vecSpriteSkin;
				}

				m_vecCachedModelFXJson.insert(std::pair<std::string, std::vector<ModelFXInfo>>(szFile, vecModelFX));

				for (auto iter = vecModelFX.begin(); iter != vecModelFX.end(); iter++)
				{
					iter->flStartTime = gHUD.m_flTime + iter->flStartTime;
					iter->flEndTime = gHUD.m_flTime + iter->flEndTime;
				}

				m_vecTempModelFX.push_back(std::pair<std::string, std::vector<ModelFXInfo>>(szName, vecModelFX));
			}
		}
	}



	void CModelFXPlayer::AddToModelFXArray(std::string szModelName, ModelFXInfo& info)
	{
		m_vecModelFXArray[szModelName].push_back(info);
	}

	CModelFXPlayer::Color* CModelFXPlayer::GetCurrentColor(std::vector<Color> colorArray, float rate)
	{
		static Color result;

		int iNumFrame = colorArray.size();

		rate = std::clamp(rate, colorArray[0].rate, colorArray[iNumFrame - 1].rate);

		for (int i = 0; i < iNumFrame; i++)
		{
			Color& FrameData = colorArray[i];

			if (rate == FrameData.rate)
			{
				result = FrameData;

				if (i == iNumFrame - 1)
				{
					//Final
				}

				return &result;
			}

			if (i + 1 < iNumFrame)
			{
				Color& NextFramedata = colorArray[i + 1];

				if (rate > FrameData.rate && rate < NextFramedata.rate)
				{
					float percen = (rate - FrameData.rate) / (NextFramedata.rate - FrameData.rate);

					result.r = FrameData.r + (NextFramedata.r - FrameData.r) * percen;
					result.g = FrameData.g + (NextFramedata.g - FrameData.g) * percen;
					result.b = FrameData.b + (NextFramedata.b - FrameData.b) * percen;
					result.a = FrameData.a + (NextFramedata.a - FrameData.a) * percen;
					
					return &result;
				}
			}
		}

		return NULL;
	}

	void CModelFXPlayer::UpdateModelInfo(std::string szModelName, float flTime)
	{
		for (auto mapModelFX = m_vecTempModelFX.begin(); mapModelFX != m_vecTempModelFX.end();)
		{
			if (mapModelFX->first != szModelName)
			{
				mapModelFX = m_vecTempModelFX.erase(mapModelFX);
			}
			else
			{
				if (mapModelFX->second.empty())
				{
					mapModelFX = m_vecTempModelFX.erase(mapModelFX);
					continue;
				}

				int index = 0;
				for (int i = 0; i < mapModelFX->second.size(); i++)
				{
					if (mapModelFX->second[i].flStartTime < gHUD.m_flTime)
					{
						AddToModelFXArray(szModelName, mapModelFX->second[i]);	
					}
					else
					{
						mapModelFX->second[index] = mapModelFX->second[i];
						index++;
					}
				}

				mapModelFX->second.resize(index);
				mapModelFX++;
			}
		}


		for (auto mapModelFX = m_vecModelFXArray.begin(); mapModelFX != m_vecModelFXArray.end();)
		{
			if (mapModelFX->first != szModelName)
			{
				mapModelFX = m_vecModelFXArray.erase(mapModelFX);
			}
			else
			{
				int index = 0;
				for (int i = 0; i < mapModelFX->second.size(); i++)
				{
					if (mapModelFX->second[i].flEndTime >= gHUD.m_flTime)
					{
						float life = mapModelFX->second[i].flEndTime - mapModelFX->second[i].flStartTime;

						float currentTime = gHUD.m_flTime - mapModelFX->second[i].flStartTime;

						float rate = currentTime / life;

						Color* pColor = GetCurrentColor(mapModelFX->second[i].vecColorKeyFrame, rate);
					
						mapModelFX->second[i].r = int(pColor->r * 255);
						mapModelFX->second[i].g = int(pColor->g * 255);
						mapModelFX->second[i].b = int(pColor->b * 255);
						mapModelFX->second[i].renderamt = int((1 - pColor->a) * 255);

						mapModelFX->second[index] = mapModelFX->second[i];
						index++;
					}
				}

				mapModelFX->second.resize(index);
				mapModelFX++;
			}
		}
	}
}