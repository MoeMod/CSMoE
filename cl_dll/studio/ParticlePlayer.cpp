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

	extern engine_studio_api_t IEngineStudio;

	void CParticlePlayer::PrecacheTexture(std::string szFile)
	{
		SharedTexture Temp;

		R_InitTexture(Temp, szFile.c_str());

		m_vecCachedTexture.insert(std::pair<std::string, SharedTexture>(szFile, Temp));

	}

	void CParticlePlayer::ClearPrecache()
	{
		m_vecCachedParticleJson.clear();
	}

	void CParticlePlayer::ReadTempParticleInfo(std::string szFile)
	{
		if (m_vecCachedParticleJson.count(szFile))
		{
			cl_entity_s* viewent = gEngfuncs.GetViewModel();

			std::string vmodel = viewent->model->name;

			std::string modelname = vmodel.substr(vmodel.find_last_of('/') + 1);

			std::vector<ParticleInfo> info = m_vecCachedParticleJson[szFile];

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
					int iEmitCount = spring["EmitCount"].get<int>();
					int iStartAttachment = spring["StartAttachment"].get<int>();
					int iEndAttachment = spring["EndAttachment"].get<int>();
					int iStartBone = spring["StartBone"].get<int>();
					int iEndBone = spring["EndBone"].get<int>();
					vec3_t vecStartColor;
					vec3_t vecEndColor;
					vec3_t vecDir;
					vec3_t vecDirMin;
					vec3_t vecDirMax;
					vec3_t vecStartOffsetMin;
					vec3_t vecStartOffsetMax;
					vec3_t vecAmplitude;
					vec3_t vecFreq;
					for (int i = 0; i < 3; i++)
					{
						vecStartColor[i] = spring["StartColor"].at(i);
						vecEndColor[i] = spring["EndColor"].at(i);
						vecDir[i] = spring["Dirction"].at(i);
						vecDirMin[i] = spring["MinDir"].at(i);
						vecDirMax[i] = spring["MaxDir"].at(i);
						vecStartOffsetMin[i] = spring["StartOffsetMin"].at(i);
						vecStartOffsetMax[i] = spring["StartOffsetMax"].at(i);
						vecAmplitude[i] = spring["Amplitude"].at(i);
						vecFreq[i] = spring["Freq"].at(i);
					}
					float flStartSize = spring["StartSize"].get<float>();
					float flEndSize = spring["EndSize"].get<float>();
					float flStartAlpha = spring["StartAlpha"].get<float>();
					float flEndAlpha = spring["EndAlpha"].get<float>();
					float flLifeTime = spring["LifeTime"].get<float>();
					float flSpeed = spring["Speed"].get<float>();
					float flIntervalTime = spring["IntervalTime"].get<float>();

					vecSpring.emplace_back();
					ParticleInfo& Temp = vecSpring.back();

					Temp.iChannel = iChannel;

					if (!m_vecCachedTexture.count(szTexture))
					{
						PrecacheTexture(szTexture);
					}

					Temp.Data.pTexture = m_vecCachedTexture[szTexture];
					CalcData(Temp.Data);
					Temp.iStartBone = iStartBone;
					Temp.vecStartOffsetMin = vecStartOffsetMin;
					Temp.vecStartOffsetMax = vecStartOffsetMax;
					Temp.iEndBone = iEndBone;
					Temp.iStartAttachment = iStartAttachment;
					Temp.iEndAttachment = iEndAttachment;
					Temp.vecStartColor = vecStartColor;
					Temp.vecEndColor = vecEndColor;
					Temp.flStartSize = flStartSize;
					Temp.flEndSize = flEndSize;
					Temp.flStartAlpha = flStartAlpha;
					Temp.flEndAlpha = flEndAlpha;
					Temp.flLifeTime = flLifeTime;
					Temp.flSpeed = flSpeed;
					Temp.iEmitCount = iEmitCount;	//forever
					Temp.flIntervalTime = flIntervalTime;
					Temp.vecDir = vecDir;
					Temp.vecDirMin = vecDirMin;
					Temp.vecDirMax = vecDirMax;
					Temp.vecAmplitude = vecAmplitude;
					Temp.vecFreq = vecFreq;
					Temp.iFlag = iFlag;

					//m_vecTempParticle.insert(std::pair<std::string, std::vector<ParticleInfo>>(szFile, Temp));
					//m_vecTempParticle.emplace_back(Temp);

				}
				m_vecTempParticle.push_back(std::pair<std::string, std::vector<ParticleInfo>>(szName, vecSpring));

				m_vecCachedParticleJson.insert(std::pair<std::string, std::vector<ParticleInfo>>(szFile, vecSpring));
		
			}
		}
	}

	int CParticlePlayer::Init(void)
	{
		m_ParticleArray.clear();
		m_vecTempParticle.clear();

		PrecacheTexture("resource/weapontexture/SGFX_se_glow_05.tga");

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
					int iStartAttachment = spring["StartAttachment"].get<int>();
					int iEndAttachment = spring["EndAttachment"].get<int>();
					int iStartBone = spring["StartBone"].get<int>();
					int iEndBone = spring["EndBone"].get<int>();
					vec3_t vecStartColor;
					vec3_t vecEndColor;
					vec3_t vecDir;
					vec3_t vecDirMin;
					vec3_t vecDirMax;
					vec3_t vecStartOffsetMin;
					vec3_t vecStartOffsetMax;
					vec3_t vecAmplitude;
					vec3_t vecFreq;
					for (int i = 0; i < 3; i++)
					{
						vecStartColor[i] = spring["StartColor"].at(i);
						vecEndColor[i] = spring["EndColor"].at(i);
						vecDir[i] = spring["Dirction"].at(i);
						vecDirMin[i] = spring["MinDir"].at(i);
						vecDirMax[i] = spring["MaxDir"].at(i);
						vecStartOffsetMin[i] = spring["StartOffsetMin"].at(i);
						vecStartOffsetMax[i] = spring["StartOffsetMax"].at(i);
						vecAmplitude[i] = spring["Amplitude"].at(i);
						vecFreq[i] = spring["Freq"].at(i);
					}
					float flStartSize = spring["StartSize"].get<float>();
					float flEndSize = spring["EndSize"].get<float>();
					float flStartAlpha = spring["StartAlpha"].get<float>();
					float flEndAlpha = spring["EndAlpha"].get<float>();
					float flLifeTime = spring["LifeTime"].get<float>();
					float flSpeed = spring["Speed"].get<float>();
					float flIntervalTime = spring["IntervalTime"].get<float>();

					vecSpring.emplace_back();
					ParticleInfo& Temp = vecSpring.back();

					Temp.iChannel = iChannel;

					if (!m_vecCachedTexture.count(szTexture))
					{
						PrecacheTexture(szTexture);
					}

					Temp.Data.pTexture = m_vecCachedTexture[szTexture];
					CalcData(Temp.Data);
					Temp.iStartBone = iStartBone;
					Temp.vecStartOffsetMin = vecStartOffsetMin;
					Temp.vecStartOffsetMax = vecStartOffsetMax;
					Temp.iEndBone = iEndBone;
					Temp.iStartAttachment = iStartAttachment;
					Temp.iEndAttachment = iEndAttachment;			
					Temp.vecStartColor = vecStartColor;
					Temp.vecEndColor = vecEndColor;
					Temp.flStartSize = flStartSize;
					Temp.flEndSize = flEndSize;
					Temp.flStartAlpha = flStartAlpha;
					Temp.flEndAlpha = flEndAlpha;
					Temp.flLifeTime = flLifeTime;
					Temp.flSpeed = flSpeed;
					Temp.iEmitCount = 255;	//forever
					Temp.flIntervalTime = flIntervalTime;
					Temp.vecDir = vecDir;
					Temp.vecDirMin = vecDirMin;
					Temp.vecDirMax = vecDirMax;
					Temp.vecAmplitude = vecAmplitude;
					Temp.vecFreq = vecFreq;
					Temp.iFlag = iFlag;

				}

				m_vecStaticParticle.insert(std::pair<std::string, std::vector<ParticleInfo>>(szName, vecSpring));
			}
		}

		return 1;
	}

	void CParticlePlayer::CalcData(ParticleData& info)
	{
		int w = info.pTexture->w();
		int h = info.pTexture->h();

		info.up     = 1;//(h >> 1);
		info.left   = -1;//-(w >> 1);
		info.down   = -1;//(h >> 1) - h;
		info.right  = 1;//w + -(w >> 1);
	}

	void CParticlePlayer::DrawTGAQuad(ParticleData& particle, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color)
	{
		vec3_t	point;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gRenderAPI.GL_Upload(particle.pTexture->texnum());
		gRenderAPI.GL_Bind(0, particle.pTexture->texnum());
		gEngfuncs.pTriAPI->Color4f(color[0], color[1], color[2], color[3]);

		gEngfuncs.pTriAPI->Begin(TRI_QUADS);
		gEngfuncs.pTriAPI->TexCoord2f(0, 1);
		VectorMA(org, particle.down * scale, v_up, point);
		VectorMA(point, particle.left * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(0, 0);
		VectorMA(org, particle.up * scale, v_up, point);
		VectorMA(point, particle.left * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(1, 0);
		VectorMA(org, particle.up * scale, v_up, point);
		VectorMA(point, particle.right * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);

		gEngfuncs.pTriAPI->TexCoord2f(1, 1);
		VectorMA(org, particle.down * scale, v_up, point);
		VectorMA(point, particle.right * scale, v_right, point);
		gEngfuncs.pTriAPI->Vertex3fv(point);
	
		gEngfuncs.pTriAPI->End();
	}

	void CParticlePlayer::InitHUDData(void)
	{

	}
	int CParticlePlayer::VidInit(void)
	{

		return 1;
	}

	void CParticlePlayer::Shutdown(void)
	{
	}

	void CParticlePlayer::MSG_AddParticle(const char* szTexture, int iChannel, int iEmitCount, float flIntervalTime, float flLifeTime, float flSpeed, float flStartSize, float flEndSize, int iStartBone, int iEndBone, int iStartAttachment, int iEndAttachment, vec3_t vecStartBoneOffset, vec3_t vecDir, vec3_t vecStartColor, vec3_t vecEndColor, float flStartAlpha, float flEndAlpha, vec3_t vecAmplitude, vec3_t vecFreq)
	{
#if 0
		m_vecTempParticle.emplace_back();
		ParticleInfo& Temp = m_vecTempParticle.back();

		Temp.iChannel = iChannel;
		if (!m_vecCachedTexture.count(szTexture))
		{
			PrecacheTexture(szTexture);
		}
		Temp.Data.pTexture = m_vecCachedTexture[szTexture];
		CalcData(Temp.Data);
		Temp.iStartBone = iStartBone;
		Temp.iEndBone = iEndBone;
		Temp.vecStartBoneOffset = vecStartBoneOffset;
		Temp.iStartAttachment = iStartAttachment;
		Temp.iEndAttachment = iEndAttachment;
		Temp.vecStartColor = vecStartColor;
		Temp.vecEndColor = vecEndColor;
		Temp.flStartSize = flStartSize;
		Temp.flEndSize = flEndSize;
		Temp.flStartAlpha = flStartAlpha;
		Temp.flEndAlpha = flEndAlpha;
		Temp.flLifeTime = flLifeTime;
		Temp.flSpeed = flSpeed;
		Temp.iEmitCount = iEmitCount;
		Temp.flIntervalTime = flIntervalTime;
		Temp.vecDir = vecDir;
		Temp.vecAmplitude = vecAmplitude;
		Temp.vecFreq = vecFreq;
#endif
	}

	void CParticlePlayer::ModifyStaticParticle(std::string szModelName, const char* szTexture, int iChannel, int iEmitCount, float flIntervalTime, float flLifeTime, float flSpeed, float flStartSize, float flEndSize, int iStartBone, int iEndBone, int iStartAttachment, int iEndAttachment, vec3_t vecStartOffset, vec3_t vecDir, vec3_t vecStartColor, vec3_t vecEndColor, float flStartAlpha, float flEndAlpha, vec3_t vecAmplitude, vec3_t vecFreq)
	{
		if (m_vecStaticParticle.count(szModelName))
		{
			for (ParticleInfo& info : m_vecStaticParticle[szModelName])
			{
				if (info.iChannel == iChannel)
				{
					info.flEndAlpha = flEndAlpha;
					info.flEndSize = flEndSize;
					info.flIntervalTime = flIntervalTime;
					info.flLifeTime = flLifeTime;
					info.flSpeed = flSpeed;
					info.flStartAlpha = flStartAlpha;
					info.flStartSize = flStartSize;
					info.iEmitCount = iEmitCount;
					info.iEndAttachment = iEndAttachment;
					info.iEndBone = iEndBone;
					info.iStartAttachment = iStartAttachment;
					info.iStartBone = iStartBone;
					info.vecDir = vecDir;
					//info.vecDirMin = { 0, 0, 0 };
					//info.vecDirMax = { 0, 0, 0 };
					info.vecEndColor = vecEndColor;
					info.vecStartColor = vecStartColor;
					//info.vecStartOffsetMin = { 0, 0, 0 };
					//info.vecStartOffsetMax = { 0, 0, 0 };
					info.vecAmplitude = vecAmplitude;
					info.vecFreq = vecFreq;
					//info.iFlag = iFlag;
					if (szTexture != nullptr)
					{
						if (!m_vecCachedTexture.count(szTexture))
						{
							PrecacheTexture(szTexture);
						}
						info.Data.pTexture = m_vecCachedTexture[szTexture];
						CalcData(info.Data);
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
				int index = 0;
				for (int i = 0; i < mapParticle->second.size(); i++)
				{
					if (mapParticle->second[i].iEmitCount > 0)
					{
						if (gHUD.m_flTime > m_flNextTime[mapParticle->second[i].iChannel])
						{
							if (mapParticle->second[i].iEmitCount < 255)
								mapParticle->second[i].iEmitCount--;

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
									vecStart.x = transform[mapParticle->second[i].iStartBone][0][3] + (RandomInt(0 , 1) ? -vecStartOffset.x : vecStartOffset.x);
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
							vec3_t savedStartOffsetMin = mapParticle->second[i].vecStartOffsetMin;
							vec3_t savedStartOffsetMax = mapParticle->second[i].vecStartOffsetMax;

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
							mapParticle->second[i].vecStartOffsetMin = shootPos;
							mapParticle->second[i].vecStartOffsetMax = shootEnd;

							AddToParticleArray(mapParticle->second[i]);

							mapParticle->second[i].vecDir = savedDir;
							mapParticle->second[i].vecStartOffsetMin = savedStartOffsetMin;
							mapParticle->second[i].vecStartOffsetMax = savedStartOffsetMax;
							

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
		
		
#if 0
		if (!m_vecTempParticle.empty())
		{
			int index = 0;
			for (int i = 0; i < m_vecTempParticle.size(); i++)
			{
				if (m_vecTempParticle[i].iEmitCount > 0)
				{
					if (gHUD.m_flTime > m_flNextTime[m_vecTempParticle[i].iChannel])
					{
						if (m_vecTempParticle[i].iEmitCount < 255)
							m_vecTempParticle[i].iEmitCount--;

						vec3_t shootPos;
						vec3_t vecStart;
						vec3_t vecEnd;
						vec3_t shootDir;

						if (m_vecTempParticle[i].iStartBone != 255)
						{
							vecStart.x = transform[m_vecTempParticle[i].iStartBone][0][3] + m_vecTempParticle[i].vecStartBoneOffset.x;
							vecStart.y = transform[m_vecTempParticle[i].iStartBone][1][3] + m_vecTempParticle[i].vecStartBoneOffset.y;
							vecStart.z = transform[m_vecTempParticle[i].iStartBone][2][3] + m_vecTempParticle[i].vecStartBoneOffset.z;

							VectorTransform(vecStart, transform[m_vecTempParticle[i].iStartBone], shootPos);
						}
						else
						{
							vecStart = pattachment[m_vecTempParticle[i].iStartAttachment].org + m_vecTempParticle[i].vecStartBoneOffset;

							VectorTransform(vecStart, transform[pattachment[m_vecTempParticle[i].iStartAttachment].bone], shootPos);
						}



						m_vecTempParticle[i].vecOrigin = shootPos;

						if (m_vecTempParticle[i].iEndBone != 255)
						{
							vecEnd.x = transform[m_vecTempParticle[i].iEndBone][0][3];
							vecEnd.y = transform[m_vecTempParticle[i].iEndBone][1][3];
							vecEnd.z = transform[m_vecTempParticle[i].iEndBone][2][3];
						}
						else
						{
							vecEnd = pattachment[m_vecTempParticle[i].iEndAttachment].org;
						}

						if (m_vecTempParticle[i].vecDir.IsNaN() || m_vecTempParticle[i].vecDir.IsZero())
						{
							shootDir = (vecEnd - vecStart);
							float length = sqrt(shootDir[0] * shootDir[0] + shootDir[1] * shootDir[1] + shootDir[2] * shootDir[2]);
							shootDir.x /= length;
							shootDir.y /= length;
							shootDir.z /= length;

							if (shootDir.IsNaN())
								shootDir = { 0, 0, 0 };
							m_vecTempParticle[i].vecDir = shootDir;
						}

						AddToParticleArray(m_vecTempParticle[i]);

						m_flNextTime[m_vecTempParticle[i].iChannel] = gHUD.m_flTime + m_vecTempParticle[i].flIntervalTime;
					}			
					m_vecTempParticle[index] = m_vecTempParticle[i];
					index++;
				}
			}

			m_vecTempParticle.resize(index);

		}
#endif
		if (m_vecStaticParticle.count(modelName))
		{
			for (ParticleInfo& info : m_vecStaticParticle[modelName])
			{
				if (gHUD.m_flTime > m_flNextTime[info.iChannel] && info.iEmitCount > 0)
				{
					if (info.iEmitCount < 255)
						info.iEmitCount--;

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

					vec3_t savedDir = info.vecDir;
					vec3_t savedStartOffsetMin = info.vecStartOffsetMin;
					vec3_t savedStartOffsetMax = info.vecStartOffsetMax;

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
					info.vecStartOffsetMin = shootPos;
					info.vecStartOffsetMax = shootEnd;

					AddToParticleArray(info);

					info.vecDir = savedDir;
					info.vecStartOffsetMin = savedStartOffsetMin;
					info.vecStartOffsetMax = savedStartOffsetMax;

					m_flNextTime[info.iChannel] = gHUD.m_flTime + info.flIntervalTime;
				}
			}
		}
	}

	void CParticlePlayer::AddToParticleArray(ParticleInfo& info)
	{
		m_ParticleArray.emplace_back();
		ParticleInfo& Temp = m_ParticleArray.back();

		Temp.flStartAlpha = info.flStartAlpha;
		Temp.flEndAlpha = info.flEndAlpha;
		Temp.flIntervalTime = info.flIntervalTime;
		Temp.flLifeTime = info.flLifeTime;
		Temp.flStartSize = info.flStartSize;
		Temp.flEndSize = info.flEndSize;
		Temp.flSpeed = info.flSpeed;
		Temp.iStartBone = info.iStartBone;	//useless
		Temp.iEndBone = info.iEndBone;
		Temp.vecStartOffsetMin = info.vecStartOffsetMin;	//use for startpos
		Temp.vecStartOffsetMax = info.vecStartOffsetMax;	//use for endpos
		Temp.iStartAttachment = info.iStartAttachment;	//useless
		Temp.iEndAttachment = info.iEndAttachment;		//useless
		Temp.vecDirMin = info.vecDirMin;	//useless
		Temp.vecDirMax = info.vecDirMax;	//useless
		Temp.vecStartColor = info.vecStartColor;
		Temp.vecEndColor = info.vecEndColor;
		Temp.Data = info.Data;
		Temp.iChannel = info.iChannel;
		Temp.flStartTime = gHUD.m_flTime;	
		Temp.vecDir = info.vecDir;
		Temp.vecOrigin = info.vecOrigin;
		Temp.iEmitCount = info.iEmitCount;
		Temp.vecAmplitude = info.vecAmplitude;
		Temp.vecFreq = info.vecFreq;
		Temp.iFlag = info.iFlag;

	}


	int CParticlePlayer::Draw(float flTime, matrix3x4& matrix)
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();

		if (!viewent)0;

		int index = 0;
		for (int i = 0; i < m_ParticleArray.size(); i++)
		{
			//删除播放完的队列
			float currentTime = gHUD.m_flTime - m_ParticleArray[i].flStartTime;

			if (currentTime < m_ParticleArray[i].flLifeTime)
			{
				m_ParticleArray[index] = m_ParticleArray[i];
				index++;
			}
		}
		m_ParticleArray.resize(index);


		for (auto iter = m_ParticleArray.begin(); iter != m_ParticleArray.end(); iter++)
		{
			//转世界坐标
			if (iter->Data.pTexture)
			{
				vec3_t vecWorldOrigin;

				iter->vecOrigin[0] += iter->vecDir[0] * iter->flSpeed * gHUD.m_flTimeDelta + iter->vecAmplitude[0] * cos((gHUD.m_flTime - iter->flStartTime) * 2 * M_PI * iter->vecFreq[0]);
				iter->vecOrigin[1] += iter->vecDir[1] * iter->flSpeed * gHUD.m_flTimeDelta + iter->vecAmplitude[1] * cos((gHUD.m_flTime - iter->flStartTime) * 2 * M_PI * iter->vecFreq[1]);
				iter->vecOrigin[2] += iter->vecDir[2] * iter->flSpeed * gHUD.m_flTimeDelta + iter->vecAmplitude[2] * cos((gHUD.m_flTime - iter->flStartTime) * 2 * M_PI * iter->vecFreq[2]);

				if (iter->iFlag & PARTICLE_FLAGS_ENDCENTER)
				{
					vec3_t Dir = (iter->vecStartOffsetMax - iter->vecOrigin).Normalize();

					if (DotProduct(iter->vecDir, Dir) < 0)
					{
						iter->flLifeTime = 0;
					}

				}

				VectorTransform(iter->vecOrigin, matrix, iter->vecWorldOrigin);
			}	
		}

		float vUp[3];
		float vRight[3];
		float vNormal[3];
		float vRenderOrigin[3];
		IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);
		vec3_t vecRenderOrigin = { vRenderOrigin[0], vRenderOrigin[1], vRenderOrigin[2], 0 };
		//排序vector
		std::sort(m_ParticleArray.begin(), m_ParticleArray.end(), [vecRenderOrigin](const ParticleInfo& a, const ParticleInfo& b) -> bool {
			//auto a1 = (a.vecWorldOrigin - vecRenderOrigin).LengthSquared();
			//auto a2 = (b.vecWorldOrigin - vecRenderOrigin).LengthSquared();  
			auto a1 = (a.vecWorldOrigin[0]-vecRenderOrigin[0]) * (a.vecWorldOrigin[0]-vecRenderOrigin[0]) + 
					  (a.vecWorldOrigin[1]-vecRenderOrigin[1]) * (a.vecWorldOrigin[1]-vecRenderOrigin[1]) +
					  (a.vecWorldOrigin[2]-vecRenderOrigin[2]) * (a.vecWorldOrigin[2]-vecRenderOrigin[2]);

			auto a2 = (b.vecWorldOrigin[0]-vecRenderOrigin[0]) * (b.vecWorldOrigin[0]-vecRenderOrigin[0]) + 
					  (b.vecWorldOrigin[1]-vecRenderOrigin[1]) * (b.vecWorldOrigin[1]-vecRenderOrigin[1]) +
					  (b.vecWorldOrigin[2]-vecRenderOrigin[2]) * (b.vecWorldOrigin[2]-vecRenderOrigin[2]);

			return a1 > a2;
		});


		//播放,TGA播放必须由远到近播放
		for (auto iter = m_ParticleArray.begin(); iter != m_ParticleArray.end(); iter++)
		{
			float currentTime = gHUD.m_flTime - iter->flStartTime;

			if (iter->Data.pTexture)
			{	
				vec3_t vecRight, vecUp, vecColor;

				vecRight[0] = vRight[0];
				vecRight[1] = vRight[1];
				vecRight[2] = vRight[2];

				vecUp[0] = vUp[0];
				vecUp[1] = vUp[1];
				vecUp[2] = vUp[2];

				VectorLerp(iter->vecStartColor, currentTime / iter->flLifeTime, iter->vecEndColor, vecColor);

				float alpha = std::lerp(iter->flStartAlpha, iter->flEndAlpha, currentTime / iter->flLifeTime);
				float size = std::lerp(iter->flStartSize, iter->flEndSize, currentTime / iter->flLifeTime);

				iter->vecColor[0] = vecColor[0] / 255.0;
				iter->vecColor[1] = vecColor[1] / 255.0;
				iter->vecColor[2] = vecColor[2] / 255.0;
				iter->vecColor[3] = alpha;

				DrawTGAQuad(iter->Data, iter->vecWorldOrigin, vecRight, vecUp, size, iter->vecColor);
			}
		}


		return 1;
	}
	
}