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
#include "ParticlePlayer.h"

namespace cl {

static constexpr float ParticleUp = 1.0;
static constexpr float ParticleDown = -1.0;
static constexpr float ParticleLeft = 1.0;
static constexpr float ParticleRight = -1.0;

	extern engine_studio_api_t IEngineStudio;

	void CParticlePlayer::SinCos(float rad, float* sine, float* cosine)
	{
		*sine = sinf(rad);
		*cosine = cosf(rad);
	}

	void CParticlePlayer::PrecacheTexture(std::string szFile)
	{
		if (!m_vecCachedTexture.count(szFile))
		{
			SharedTexture Temp;

			R_InitTexture(Temp, szFile.c_str());

			m_vecCachedTexture.insert(std::pair<std::string, SharedTexture>(szFile, Temp));
		}
	}

	void CParticlePlayer::ClearPrecache()
	{
		m_vecCachedParticleJson.clear();
	}

	CParticlePlayer::Scale* CParticlePlayer::GetCurrentScale(std::vector<Scale> scaleArray, float rate)
	{
		static Scale result;

		int iNumFrame = scaleArray.size();

		rate = std::clamp(rate, scaleArray[0].rate, scaleArray[iNumFrame - 1].rate);

		for (int i = 0; i < iNumFrame; i++)
		{
			Scale& FrameData = scaleArray[i];

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
				Scale& NextFramedata = scaleArray[i + 1];

				if (rate > FrameData.rate && rate < NextFramedata.rate)
				{
					float percen = (rate - FrameData.rate) / (NextFramedata.rate - FrameData.rate);

					result.scale = FrameData.scale + (NextFramedata.scale - FrameData.scale) * percen;

					return &result;
				}
			}
		}

		return NULL;
	}

	CParticlePlayer::Color* CParticlePlayer::GetCurrentColor(std::vector<Color> colorArray, float rate)
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

	void CParticlePlayer::ReadSprInfo(ParticleInfo& info, std::string sprFile)
	{
		if (m_vecCachedSpriteJson.count(sprFile))
		{
			
			info.vecSpriteTexName = m_vecCachedSpriteJson[sprFile];
		}
		else
		{
			fs_offset_t length;

			nlohmann::json jsSPR;

			auto data = gFileSystemAPI.FS_MapFile(sprFile.c_str(), &length, true);
			if (data)
			{

				jsSPR = nlohmann::json::parse(std::string_view((const char*)data, length));
				gFileSystemAPI.FS_MapFree(data, length);

				std::vector<std::string> vecTexName;
				for (auto& SPR : jsSPR)
				{
					vecTexName.push_back(SPR.get<std::string>());
					PrecacheTexture(SPR.get<std::string>());
				}


				info.vecSpriteTexName = vecTexName;

				m_vecCachedSpriteJson.insert(std::pair<std::string, std::vector<std::string>>(sprFile, vecTexName));
			}
		}
	}

	void CParticlePlayer::ReadTempParticleInfo(std::string szFile)
	{
		if (m_vecCachedParticleJson.count(szFile))
		{
			cl_entity_s* viewent = gEngfuncs.GetViewModel();

			std::string vmodel = viewent->model->name;

			std::string modelname = vmodel.substr(vmodel.find_last_of('/') + 1);

			std::vector<ParticleInfo> info = m_vecCachedParticleJson[szFile];

			for (auto iter = info.begin(); iter != info.end(); iter++)
			{
				iter->flStartTime = gHUD.m_flTime + iter->flStartTime;
				iter->flEndTime = gHUD.m_flTime + iter->flEndTime;
			}

			m_vecTempParticle.push_back(std::pair<std::string, std::vector<ParticleInfo>>(modelname, info));


			//for (auto& info : m_vecCachedParticleJson[szFile])
			//{
			//	m_vecTempParticle.emplace_back(info);
			//}
		}
		else
		{
			fs_offset_t length;

			nlohmann::json particleEmiiter;

			auto data = gFileSystemAPI.FS_MapFile(szFile.c_str(), &length, true);
			if (data)
			{
				particleEmiiter = nlohmann::json::parse(std::string_view((const char*)data, length));
				gFileSystemAPI.FS_MapFree(data, length);

				std::string szName = particleEmiiter["Name"].get<std::string>();
				std::vector<ParticleInfo> vecSpring;
				for (auto& spring : particleEmiiter["Spring"])
				{
					int iChannel = spring["Channel"].get<int>();
					int iFlag = spring["Flag"].get<int>();
					std::string szTexture = spring["TextureFile"].get<std::string>();
					std::string szSprite = spring["Sprite"].get<std::string>();
					int iEmitCount = spring["EmitCount"].get<int>();
					int iStartAttachment = spring["StartAttachment"].get<int>();
					int iEndAttachment = spring["EndAttachment"].get<int>();
					int iStartBone = spring["StartBone"].get<int>();
					int iEndBone = spring["EndBone"].get<int>();
					int iEmitPerCount = spring["ParticlesPerEmission"].get<int>();
					
					vec3_t vecDir;
					vec3_t vecDirMin;
					vec3_t vecDirMax;
					vec3_t vecStartOffsetMin;
					vec3_t vecStartOffsetMax;
					vec3_t vecAmplitude;
					vec3_t vecFreq;
					std::vector<Color> vecColorKeyFrame;
					std::vector<Scale> vecScaleKeyFrame;

					for (const auto& row : spring["ColorKeyFrame"]) {
						Color rowData;
						rowData.rate = row[0];
						rowData.r = row[1];
						rowData.g = row[2];
						rowData.b = row[3];
						rowData.a = row[4];
						vecColorKeyFrame.push_back(rowData);
					}

					for (const auto& row : spring["ScaleKeyFrame"]) {
						Scale rowData;
						rowData.rate = row[0];
						rowData.scale = row[1];
						vecScaleKeyFrame.push_back(rowData);
					}

					for (int i = 0; i < 3; i++)
					{
						vecDir[i] = spring["Dirction"].at(i);
						vecDirMin[i] = spring["MinDir"].at(i);
						vecDirMax[i] = spring["MaxDir"].at(i);
						vecStartOffsetMin[i] = spring["StartOffsetMin"].at(i);
						vecStartOffsetMax[i] = spring["StartOffsetMax"].at(i);
						vecAmplitude[i] = spring["Amplitude"].at(i);
						vecFreq[i] = spring["Freq"].at(i);
					}
					float flSpeed = spring["Speed"].get<float>();
					float flIntervalTime = spring["IntervalTime"].get<float>();
					float flRotateSpeed = spring["RotateSpeed"].get<float>();
					float flStartTime = spring["StartTime"].get<float>();
					float flEndTime = spring["EndTime"].get<float>();
					float flFrameRate = spring["FrameRate"].get<float>();

					vecSpring.emplace_back();
					ParticleInfo& Temp = vecSpring.back();

					Temp.iChannel = iChannel;

					if (szSprite == "" || szSprite == "null")
					{
						PrecacheTexture(szTexture);
					
						Temp.pTexture = m_vecCachedTexture[szTexture];			
					}
					else
					{
						ReadSprInfo(Temp, szSprite);
					}


					Temp.iStartBone = iStartBone;
					Temp.vecStartOffsetMin = vecStartOffsetMin;
					Temp.vecStartOffsetMax = vecStartOffsetMax;
					Temp.iEndBone = iEndBone;
					Temp.iStartAttachment = iStartAttachment;
					Temp.iEndAttachment = iEndAttachment;
					Temp.flSpeed = flSpeed;
					Temp.iEmitCount = iEmitCount;	//forever
					Temp.flIntervalTime = flIntervalTime;
					Temp.vecDir = vecDir;
					Temp.vecDirMin = vecDirMin;
					Temp.vecDirMax = vecDirMax;
					Temp.vecAmplitude = vecAmplitude;
					Temp.vecFreq = vecFreq;
					Temp.flRotateSpeed = flRotateSpeed;
					Temp.flStartTime = flStartTime;
					Temp.flEndTime = flEndTime;
					Temp.iFlag = iFlag;
					Temp.iEmitPerCount = iEmitPerCount;
					Temp.vecColorKeyFrame = vecColorKeyFrame;
					Temp.vecScaleKeyFrame = vecScaleKeyFrame;
					Temp.flFrameRate = flFrameRate;

					//m_vecTempParticle.insert(std::pair<std::string, std::vector<ParticleInfo>>(szFile, Temp));
					//m_vecTempParticle.emplace_back(Temp);

				}
				
				m_vecCachedParticleJson.insert(std::pair<std::string, std::vector<ParticleInfo>>(szFile, vecSpring));

				for (auto iter = vecSpring.begin(); iter != vecSpring.end(); iter++)
				{
					iter->flStartTime = gHUD.m_flTime + iter->flStartTime;
					iter->flEndTime = gHUD.m_flTime + iter->flEndTime;
				}
		
				m_vecTempParticle.push_back(std::pair<std::string, std::vector<ParticleInfo>>(szName, vecSpring));

			}
		}
	}

	int CParticlePlayer::Init(void)
	{
		m_ParticleArray.clear();
		m_vecTempParticle.clear();

		fs_offset_t length;

		nlohmann::json g_VVIPEyeWeapons;

		auto data = gFileSystemAPI.FS_MapFile("VVIPEyeWeapons.json", &length, true);
		if (data)
		{
			g_VVIPEyeWeapons = nlohmann::json::parse(std::string_view((const char*)data, length));
			gFileSystemAPI.FS_MapFree(data, length);

			for (auto& info : g_VVIPEyeWeapons)
			{
				std::string szName = info["Name"].get<std::string>();
				std::vector<ParticleInfo> vecSpring;
				for (auto& spring : info["Spring"])
				{
					int iChannel = spring["Channel"].get<int>();
					int iFlag = spring["Flag"].get<int>();
					std::string szTexture = spring["TextureFile"].get<std::string>();
					std::string szSprite = spring["Sprite"].get<std::string>();
					int iStartAttachment = spring["StartAttachment"].get<int>();
					int iEndAttachment = spring["EndAttachment"].get<int>();
					int iStartBone = spring["StartBone"].get<int>();
					int iEndBone = spring["EndBone"].get<int>();
					int iEmitPerCount = spring["ParticlesPerEmission"].get<int>();

					vec3_t vecDir;
					vec3_t vecDirMin;
					vec3_t vecDirMax;
					vec3_t vecStartOffsetMin;
					vec3_t vecStartOffsetMax;
					vec3_t vecAmplitude;
					vec3_t vecFreq;
					std::vector<Color> vecColorKeyFrame;
					std::vector<Scale> vecScaleKeyFrame;

					for (const auto& row : spring["ColorKeyFrame"]) {
						Color rowData;
						rowData.rate = row[0];
						rowData.r = row[1];
						rowData.g = row[2];
						rowData.b = row[3];
						rowData.a = row[4];
						vecColorKeyFrame.push_back(rowData);
					}

					for (const auto& row : spring["ScaleKeyFrame"]) {
						Scale rowData;
						rowData.rate = row[0];
						rowData.scale = row[1];			
						vecScaleKeyFrame.push_back(rowData);
					}

					for (int i = 0; i < 3; i++)
					{
						vecDir[i] = spring["Dirction"].at(i);
						vecDirMin[i] = spring["MinDir"].at(i);
						vecDirMax[i] = spring["MaxDir"].at(i);
						vecStartOffsetMin[i] = spring["StartOffsetMin"].at(i);
						vecStartOffsetMax[i] = spring["StartOffsetMax"].at(i);
						vecAmplitude[i] = spring["Amplitude"].at(i);
						vecFreq[i] = spring["Freq"].at(i);
					}
					float flSpeed = spring["Speed"].get<float>();
					float flIntervalTime = spring["IntervalTime"].get<float>();
					float flStartTime = spring["StartTime"].get<float>();
					float flEndTime = spring["EndTime"].get<float>();
					float flRotateSpeed = spring["RotateSpeed"].get<float>();
					float flFrameRate = spring["FrameRate"].get<float>();

					vecSpring.emplace_back();
					ParticleInfo& Temp = vecSpring.back();

					Temp.iChannel = iChannel;

					if (szSprite == "" || szSprite == "null")
					{
						PrecacheTexture(szTexture);
					
						Temp.pTexture = m_vecCachedTexture[szTexture];
					}
					else
					{
						ReadSprInfo(Temp, szSprite);
					}

					Temp.iStartBone = iStartBone;
					Temp.vecStartOffsetMin = vecStartOffsetMin;
					Temp.vecStartOffsetMax = vecStartOffsetMax;
					Temp.iEndBone = iEndBone;
					Temp.iStartAttachment = iStartAttachment;
					Temp.iEndAttachment = iEndAttachment;			
					Temp.flSpeed = flSpeed;
					Temp.iEmitCount = 255;	//forever
					Temp.flIntervalTime = flIntervalTime;
					Temp.vecDir = vecDir;
					Temp.vecDirMin = vecDirMin;
					Temp.vecDirMax = vecDirMax;
					Temp.vecAmplitude = vecAmplitude;
					Temp.vecFreq = vecFreq;
					Temp.flRotateSpeed = flRotateSpeed;
					Temp.flStartTime = flStartTime;
					Temp.flEndTime = flEndTime;
					Temp.iFlag = iFlag;
					Temp.iEmitPerCount = iEmitPerCount;
					Temp.vecColorKeyFrame = vecColorKeyFrame;
					Temp.vecScaleKeyFrame = vecScaleKeyFrame;
					Temp.flFrameRate = flFrameRate;
				

				}

				m_vecStaticParticle.insert(std::pair<std::string, std::vector<ParticleInfo>>(szName, vecSpring));
			}
		}

		return 1;
	}

	void CParticlePlayer::DrawTGAQuad(int iTexnum, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color)
	{
		vec3_t	point;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gRenderAPI.GL_Upload(iTexnum);
		gRenderAPI.GL_Bind(0, iTexnum);
		gEngfuncs.pTriAPI->Color4f(color[0], color[1], color[2], color[3]);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		VectorMA(org, ParticleDown * scale, v_up, point);
		VectorMA(point, ParticleLeft * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		VectorMA(org, ParticleUp * scale, v_up, point);
		VectorMA(point, ParticleLeft * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		VectorMA(org, ParticleUp * scale, v_up, point);
		VectorMA(point, ParticleRight * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		VectorMA(org, ParticleDown * scale, v_up, point);
		VectorMA(point, ParticleRight * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);
	
		gEngfuncs.pTriAPI->End();
	}

	void CParticlePlayer::ResetData(void)
	{
		m_ParticleArray.clear();
		m_vecTempParticle.clear();

		for (int i = 0; i < MAX_PARTICLE_CHANNEL; i++)
		{
			m_flNextTime[i] = 0;
		}
		
	}
	int CParticlePlayer::VidInit(void)
	{

		return 1;
	}

	void CParticlePlayer::Shutdown(void)
	{
	}

	void CParticlePlayer::ModifyStaticParticle(std::string szModelName, const char* szTexture, int iChannel, int iEmitCount, float flIntervalTime, float flLifeTime, float flSpeed, float flStartSize, float flEndSize, int iStartBone, int iEndBone, int iStartAttachment, int iEndAttachment, vec3_t vecStartOffset, vec3_t vecDir, vec3_t vecStartColor, vec3_t vecEndColor, float flStartAlpha, float flEndAlpha, vec3_t vecAmplitude, vec3_t vecFreq)
	{
		if (m_vecStaticParticle.count(szModelName))
		{
			for (ParticleInfo& info : m_vecStaticParticle[szModelName])
			{
				if (info.iChannel == iChannel)
				{
					info.flIntervalTime = flIntervalTime;
					info.flSpeed = flSpeed;
					info.iEmitCount = iEmitCount;
					info.iEndAttachment = iEndAttachment;
					info.iEndBone = iEndBone;
					info.iStartAttachment = iStartAttachment;
					info.iStartBone = iStartBone;
					info.vecDir = vecDir;
					info.vecAmplitude = vecAmplitude;
					info.vecFreq = vecFreq;
					if (szTexture != nullptr)
					{			
						PrecacheTexture(szTexture);

						info.pTexture = m_vecCachedTexture[szTexture];
					}
						
				}	
			}
		}
	}

	void CParticlePlayer::AddParticleInfo(mstudioattachment_t* pattachment, matrix3x4* transform, char modelName[64])
	{
		for (auto mapParticle = m_vecTempParticle.begin(); mapParticle != m_vecTempParticle.end();)
		{
			if (mapParticle->first != modelName)
			{
				mapParticle = m_vecTempParticle.erase(mapParticle);
			}				
			else
			{
				if (mapParticle->second.empty())
				{
					mapParticle = m_vecTempParticle.erase(mapParticle);
					continue;
				}

				int index = 0;
				for (int i = 0; i < mapParticle->second.size(); i++)
				{
					if (mapParticle->second[i].iEmitCount > 0)
					{
						if (mapParticle->second[i].flStartTime < gHUD.m_flTime && gHUD.m_flTime > m_flNextTime[mapParticle->second[i].iChannel])
						{
							if (mapParticle->second[i].iEmitCount < 255)
								mapParticle->second[i].iEmitCount--;

							for (int j = 0; j < mapParticle->second[i].iEmitPerCount; j++)
							{
								vec3_t shootPos;
								vec3_t vecStart;
								vec3_t vecStartOffset;
								vec3_t vecEnd;
								vec3_t shootDir;
								vec3_t shootEnd;	//world

								vecStartOffset.x = RandomFloat(mapParticle->second[i].vecStartOffsetMin.x, mapParticle->second[i].vecStartOffsetMax.x);
								vecStartOffset.y = RandomFloat(mapParticle->second[i].vecStartOffsetMin.y, mapParticle->second[i].vecStartOffsetMax.y);
								vecStartOffset.z = RandomFloat(mapParticle->second[i].vecStartOffsetMin.z, mapParticle->second[i].vecStartOffsetMax.z);

								if (mapParticle->second[i].iStartBone != 255)
								{
									vecStart.x = transform[mapParticle->second[i].iStartBone][0][3] + vecStartOffset.x;
									vecStart.y = transform[mapParticle->second[i].iStartBone][1][3] + vecStartOffset.y;
									vecStart.z = transform[mapParticle->second[i].iStartBone][2][3] + vecStartOffset.z;

									if (mapParticle->second[i].iFlag & PARTICLE_FLAGS_ENDCENTER)
									{
										vecStart.x = transform[mapParticle->second[i].iStartBone][0][3] + (RandomInt(0, 1) ? -vecStartOffset.x : vecStartOffset.x);
										vecStart.y = transform[mapParticle->second[i].iStartBone][1][3] + (RandomInt(0, 1) ? -vecStartOffset.y : vecStartOffset.y);
										vecStart.z = transform[mapParticle->second[i].iStartBone][2][3] + (RandomInt(0, 1) ? -vecStartOffset.z : vecStartOffset.z);
									}

									VectorTransform(vecStart, transform[mapParticle->second[i].iStartBone], shootPos);
								}
								else
								{
									vecStart = pattachment[mapParticle->second[i].iStartAttachment].org + vecStartOffset;

									if (mapParticle->second[i].iFlag & PARTICLE_FLAGS_ENDCENTER)
									{
										vecStart.x = pattachment[mapParticle->second[i].iStartAttachment].org.x + (RandomInt(0, 1) ? -vecStartOffset.x : vecStartOffset.x);
										vecStart.y = pattachment[mapParticle->second[i].iStartAttachment].org.y + (RandomInt(0, 1) ? -vecStartOffset.y : vecStartOffset.y);
										vecStart.z = pattachment[mapParticle->second[i].iStartAttachment].org.z + (RandomInt(0, 1) ? -vecStartOffset.z : vecStartOffset.z);
									}

									VectorTransform(vecStart, transform[pattachment[mapParticle->second[i].iStartAttachment].bone], shootPos);
								}



								mapParticle->second[i].vecOrigin = shootPos;

								if (mapParticle->second[i].iEndBone != 255)
								{
									vecEnd.x = transform[mapParticle->second[i].iEndBone][0][3];
									vecEnd.y = transform[mapParticle->second[i].iEndBone][1][3];
									vecEnd.z = transform[mapParticle->second[i].iEndBone][2][3];

									VectorTransform(vecEnd, transform[mapParticle->second[i].iEndBone], shootEnd);
								}
								else
								{
									vecEnd = pattachment[mapParticle->second[i].iEndAttachment].org;

									VectorTransform(vecEnd, transform[pattachment[mapParticle->second[i].iEndAttachment].bone], shootEnd);
								}

								vec3_t savedDir = mapParticle->second[i].vecDir;
			
								if (mapParticle->second[i].vecDir.IsNaN() || mapParticle->second[i].vecDir.IsZero())
								{
									shootDir = (vecEnd - vecStart);
									float length = sqrt(shootDir[0] * shootDir[0] + shootDir[1] * shootDir[1] + shootDir[2] * shootDir[2]);
									shootDir.x /= length;
									shootDir.y /= length;
									shootDir.z /= length;

									if (shootDir.IsNaN())
										shootDir = { 0, 0, 0 };
									mapParticle->second[i].vecDir = shootDir;
								}

								vec3_t vecDirOffset;

								vecDirOffset.x = RandomFloat(mapParticle->second[i].vecDirMin.x, mapParticle->second[i].vecDirMax.x);
								vecDirOffset.y = RandomFloat(mapParticle->second[i].vecDirMin.y, mapParticle->second[i].vecDirMax.y);
								vecDirOffset.z = RandomFloat(mapParticle->second[i].vecDirMin.z, mapParticle->second[i].vecDirMax.z);

								mapParticle->second[i].vecDir = (mapParticle->second[i].vecDir + vecDirOffset).Normalize();
							
								if (!mapParticle->second[i].vecSpriteTexName.empty())
								{
									mapParticle->second[i].pTexture = m_vecCachedTexture[mapParticle->second[i].vecSpriteTexName[0]];
								}

								AddToParticleArray(modelName, mapParticle->second[i]);

								mapParticle->second[i].vecDir = savedDir;
							}

							m_flNextTime[mapParticle->second[i].iChannel] = gHUD.m_flTime + mapParticle->second[i].flIntervalTime;
						}
						
						mapParticle->second[index] = mapParticle->second[i];
						index++;
						
						
					}
				}
				mapParticle->second.resize(index);
				mapParticle++;
			}
		}
		
		if (m_vecStaticParticle.count(modelName))
		{
			for (ParticleInfo& info : m_vecStaticParticle[modelName])
			{
				if (gHUD.m_flTime > m_flNextTime[info.iChannel] && info.iEmitCount > 0)
				{
					if (info.iEmitCount < 255)
						info.iEmitCount--;

					for (int i = 0; i < info.iEmitPerCount; i++)
					{
						vec3_t shootPos;
						vec3_t vecStart;
						vec3_t vecStartOffset;
						vec3_t vecEnd;
						vec3_t shootDir;
						vec3_t shootEnd;	//world

						vecStartOffset.x = RandomFloat(info.vecStartOffsetMin.x, info.vecStartOffsetMax.x);
						vecStartOffset.y = RandomFloat(info.vecStartOffsetMin.y, info.vecStartOffsetMax.y);
						vecStartOffset.z = RandomFloat(info.vecStartOffsetMin.z, info.vecStartOffsetMax.z);

						if (info.iStartBone != 255)
						{
							vecStart.x = transform[info.iStartBone][0][3] + vecStartOffset.x;
							vecStart.y = transform[info.iStartBone][1][3] + vecStartOffset.y;
							vecStart.z = transform[info.iStartBone][2][3] + vecStartOffset.z;

							VectorTransform(vecStart, transform[info.iStartBone], shootPos);
						}
						else
						{
							vecStart = pattachment[info.iStartAttachment].org + vecStartOffset;

							VectorTransform(vecStart, transform[pattachment[info.iStartAttachment].bone], shootPos);
						}

						info.vecOrigin = shootPos;

						if (info.iEndBone != 255)
						{
							vecEnd.x = transform[info.iEndBone][0][3];
							vecEnd.y = transform[info.iEndBone][1][3];
							vecEnd.z = transform[info.iEndBone][2][3];

							VectorTransform(vecEnd, transform[info.iEndBone], shootEnd);
						}
						else
						{
							vecEnd = pattachment[info.iEndAttachment].org;

							VectorTransform(vecEnd, transform[pattachment[info.iEndAttachment].bone], shootEnd);
						}

						float life = info.flEndTime - info.flStartTime;
						float savedStartTime = info.flStartTime;
						float savedEndTime = info.flEndTime;
						vec3_t savedDir = info.vecDir;
					
						if (info.vecDir.IsZero())
						{
							shootDir = (vecEnd - vecStart);
							float length = sqrt(shootDir[0] * shootDir[0] + shootDir[1] * shootDir[1] + shootDir[2] * shootDir[2]);
							shootDir.x /= length;
							shootDir.y /= length;
							shootDir.z /= length;
							if (shootDir.IsNaN())
								shootDir = { 0, 0, 0 };

							info.vecDir = shootDir;
						}

						vec3_t vecDirOffset;

						vecDirOffset.x = RandomFloat(info.vecDirMin.x, info.vecDirMax.x);
						vecDirOffset.y = RandomFloat(info.vecDirMin.y, info.vecDirMax.y);
						vecDirOffset.z = RandomFloat(info.vecDirMin.z, info.vecDirMax.z);

						info.vecDir = (info.vecDir + vecDirOffset).Normalize();
						info.flStartTime = gHUD.m_flTime;
						info.flEndTime = gHUD.m_flTime + life;
						if (!info.vecSpriteTexName.empty())
						{
							info.pTexture = m_vecCachedTexture[info.vecSpriteTexName[0]];
						}

						AddToParticleArray(modelName,info);

						info.vecDir = savedDir;
						info.flStartTime = savedStartTime;
						info.flEndTime = savedEndTime;
					}

					m_flNextTime[info.iChannel] = gHUD.m_flTime + info.flIntervalTime;
				}
			}
		}
	}

	void CParticlePlayer::AddToParticleArray(std::string modelname, ParticleInfo& info)
	{
		m_ParticleArray.push_back(std::pair<std::string, ParticleInfo>(modelname, info));
	}


	int CParticlePlayer::Draw(float flTime, matrix3x4& matrix, mstudioattachment_t* pattachment, matrix3x4* transform, char modelName[64])
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();

		if (!viewent)
			return 0;

		for (auto Particle = m_ParticleArray.begin(); Particle != m_ParticleArray.end();)
		{
			if (Particle->first != modelName)
			{
				Particle = m_ParticleArray.erase(Particle);
				continue;
			}
			else
			{
				if (gHUD.m_flTime > Particle->second.flEndTime)
				{
					Particle = m_ParticleArray.erase(Particle);
					continue;
				}
			}

			Particle++;
		}

		for (auto iter = m_ParticleArray.begin(); iter != m_ParticleArray.end(); iter++)
		{
			//转世界坐标
			if (iter->second.pTexture)
			{
				vec3_t vecWorldOrigin;

				iter->second.vecOrigin[0] += iter->second.vecDir[0] * iter->second.flSpeed * gHUD.m_flTimeDelta + iter->second.vecAmplitude[0] * cos((gHUD.m_flTime - iter->second.flStartTime) * 2 * M_PI * iter->second.vecFreq[0]);
				iter->second.vecOrigin[1] += iter->second.vecDir[1] * iter->second.flSpeed * gHUD.m_flTimeDelta + iter->second.vecAmplitude[1] * cos((gHUD.m_flTime - iter->second.flStartTime) * 2 * M_PI * iter->second.vecFreq[1]);
				iter->second.vecOrigin[2] += iter->second.vecDir[2] * iter->second.flSpeed * gHUD.m_flTimeDelta + iter->second.vecAmplitude[2] * cos((gHUD.m_flTime - iter->second.flStartTime) * 2 * M_PI * iter->second.vecFreq[2]);

				if (iter->second.iFlag & PARTICLE_FLAGS_ENDCENTER)
				{
					vec3_t Dir = (iter->second.vecStartOffsetMax - iter->second.vecOrigin).Normalize();

					if (DotProduct(iter->second.vecDir, Dir) < 0)
					{
						iter->second.flEndTime = 0;
					}

				}

				if (iter->second.iFlag & PARTICLE_FLAGS_ATTACHED)
				{
					vec3_t shootPos;
					vec3_t vecStart;
					vec3_t vecStartOffset;

					vecStartOffset.x = RandomFloat(iter->second.vecStartOffsetMin.x, iter->second.vecStartOffsetMax.x);
					vecStartOffset.y = RandomFloat(iter->second.vecStartOffsetMin.y, iter->second.vecStartOffsetMax.y);
					vecStartOffset.z = RandomFloat(iter->second.vecStartOffsetMin.z, iter->second.vecStartOffsetMax.z);

					if (iter->second.iStartBone != 255)
					{
						vecStart.x = transform[iter->second.iStartBone][0][3] + vecStartOffset.x;
						vecStart.y = transform[iter->second.iStartBone][1][3] + vecStartOffset.y;
						vecStart.z = transform[iter->second.iStartBone][2][3] + vecStartOffset.z;

						VectorTransform(vecStart, transform[iter->second.iStartBone], shootPos);
					}
					else
					{
						vecStart = pattachment[iter->second.iStartAttachment].org + vecStartOffset;

						VectorTransform(vecStart, transform[pattachment[iter->second.iStartAttachment].bone], shootPos);
					}

					iter->second.vecOrigin = shootPos;
				}
				
				VectorTransform(iter->second.vecOrigin, matrix, iter->second.vecWorldOrigin);
				
			}	
		}

		float vUp[3];
		float vRight[3];
		float vNormal[3];
		float vRenderOrigin[3];
		IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);
		vec3_t vecRenderOrigin = { vRenderOrigin[0], vRenderOrigin[1], vRenderOrigin[2], 0 };
		//排序vector
		std::sort(m_ParticleArray.begin(), m_ParticleArray.end(), [vecRenderOrigin](const std::pair<std::string, ParticleInfo> &a, const std::pair<std::string, ParticleInfo> &b) -> bool {
			//auto a1 = (a.vecWorldOrigin - vecRenderOrigin).LengthSquared();
			//auto a2 = (b.vecWorldOrigin - vecRenderOrigin).LengthSquared();  
			auto a1 = (a.second.vecWorldOrigin[0]-vecRenderOrigin[0]) * (a.second.vecWorldOrigin[0]-vecRenderOrigin[0]) +
					  (a.second.vecWorldOrigin[1]-vecRenderOrigin[1]) * (a.second.vecWorldOrigin[1]-vecRenderOrigin[1]) +
					  (a.second.vecWorldOrigin[2]-vecRenderOrigin[2]) * (a.second.vecWorldOrigin[2]-vecRenderOrigin[2]);

			auto a2 = (b.second.vecWorldOrigin[0]-vecRenderOrigin[0]) * (b.second.vecWorldOrigin[0]-vecRenderOrigin[0]) +
					  (b.second.vecWorldOrigin[1]-vecRenderOrigin[1]) * (b.second.vecWorldOrigin[1]-vecRenderOrigin[1]) +
					  (b.second.vecWorldOrigin[2]-vecRenderOrigin[2]) * (b.second.vecWorldOrigin[2]-vecRenderOrigin[2]);

			return a1 > a2;
		});


		//播放,TGA播放必须由远到近播放
		for (auto iter = m_ParticleArray.begin(); iter != m_ParticleArray.end(); iter++)
		{
			float currentTime = gHUD.m_flTime - iter->second.flStartTime;

			if (iter->second.pTexture)
			{	
				vec3_t vecRight, vecUp;
				vec4_t vecColor;
				float sr, cr;

				vecRight[0] = vRight[0];
				vecRight[1] = vRight[1];
				vecRight[2] = vRight[2];

				vecUp[0] = vUp[0];
				vecUp[1] = vUp[1];
				vecUp[2] = vUp[2];

				iter->second.flAngle += iter->second.flRotateSpeed;

				if (iter->second.flAngle > 360)
					iter->second.flAngle = 0;

				float flAngle = iter->second.flAngle * (M_PI2 / 360.0f);
				SinCos(flAngle, &sr, &cr);

				vecRight = vecRight * cr + vecUp * sr;
				vecUp = vecRight * -sr + vecUp * cr;

				float life = iter->second.flEndTime - iter->second.flStartTime;

				float rate = currentTime / life;

				Color* pColor = GetCurrentColor(iter->second.vecColorKeyFrame, rate);
				Scale* pScale = GetCurrentScale(iter->second.vecScaleKeyFrame, rate);

				vecColor[0] = pColor->r;
				vecColor[1] = pColor->g;
				vecColor[2] = pColor->b;
				vecColor[3] = 1.0 - pColor->a;

				if (!iter->second.vecSpriteTexName.empty())
				{		
					int frame = 0;
					int maxframe = iter->second.vecSpriteTexName.size();
					frame = int(iter->second.flFrameRate * currentTime);

					if (frame >= maxframe)
					{
						if (iter->second.iFlag & PARTICLE_FLAGS_SPR_LOOP)
						{
							frame = int(frame) % maxframe;						
						}
						else
						{
							frame = maxframe - 1;
						}

					}

					iter->second.pTexture = m_vecCachedTexture[iter->second.vecSpriteTexName[frame]];
				}

				DrawTGAQuad(iter->second.pTexture->texnum(), iter->second.vecWorldOrigin, vecRight, vecUp, pScale->scale, vecColor);
		
				
			}
		}


		return 1;
	}
	
}