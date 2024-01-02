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


	int CParticlePlayer::Init(void)
	{
		m_TempParticleArray.clear();

		R_InitTexture(test.Data.pTexture, "resource/weapontexture/SGFX_se_glow_05.tga");
		CalcData(test.Data);

		//test.flIntervalTime = 0.01;
		test.flLifeTime = 0.6;
		test.flSize = 1.9;
		test.flSpeed = 70.0;
		test.flAlpha = 200;
		//test.iStartAttachment = 1;
		//test.iEndAttachment = 2;
		//test.iStartColor = { 0, 135, 255 };
		//test.iEndColor = { 50, 0, 200 };
		m_flNextTime = 0;
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
		gEngfuncs.pTriAPI->Color4f(0.0, 0.1, 0.8, 1);

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

	void CParticlePlayer::AddAnimInfo(const vec3_t& shootPos, const vec3_t& shootDir)
	{
		if (gHUD.m_flTime > m_flNextTime)
		{
			// for (int i = 0 ; i < 20; i++)
			// {
			// 	test.vecAttachment[i] = attachment[i];
			// }
			test.vecOrigin = shootPos;
			test.vecDir = shootDir;
			
			AddToTempParticleArray(test);
			m_flNextTime = gHUD.m_flTime + 0.01;
		}
			
	}

	void CParticlePlayer::AddToTempParticleArray(ParticleInfo& info)
	{
		m_TempParticleArray.emplace_back();
		ParticleInfo& Temp = m_TempParticleArray.back();

		// for (int i = 0; i < 20; i++)
		// {
		// 	Temp.vecAttachment[i] = info.vecAttachment[i];
		// }
		
		//Temp.flAlpha = info.flAlpha;
		//Temp.flIntervalTime = info.flIntervalTime;
		Temp.flLifeTime = info.flLifeTime;
		Temp.flSize = info.flSize;
		Temp.flSpeed = info.flSpeed;
		//Temp.iStartAttachment = info.iStartAttachment;
		//Temp.iEndAttachment = info.iEndAttachment;
		//Temp.iStartColor = info.iStartColor;
		//Temp.iEndColor = info.iEndColor;
		Temp.Data = info.Data;
		//Temp.iId = m_TempParticleArray.size() - 1;
		Temp.flStartTime = gHUD.m_flTime;
		Temp.bActive = true;

		//Temp.vecStartAttachment = Temp.vecAttachment[info.iStartAttachment];
		//Temp.vecEndAttachment = Temp.vecAttachment[info.iEndAttachment];
		Temp.vecDir = info.vecDir; //(Temp.vecEndAttachment - Temp.vecStartAttachment).Normalize();
		Temp.vecOrigin = info.vecOrigin;

	}

	bool CParticlePlayer::compare(ParticleInfo& Particle1, ParticleInfo& Particle2)
	{
		float vUp[3];
		float vRight[3];
		float vNormal[3];
		float vRenderOrigin[3];

		IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);

		vec3_t vecRenderOrigin;

		vecRenderOrigin[0] = vRenderOrigin[0];
		vecRenderOrigin[1] = vRenderOrigin[1];
		vecRenderOrigin[2] = vRenderOrigin[2];


		return (Particle1.vecWorldOrigin - vecRenderOrigin).Length() < (Particle2.vecWorldOrigin - vecRenderOrigin).Length();
	}
	
	int CParticlePlayer::Draw(float flTime, matrix3x4& matrix)
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();

		if (!viewent)
			return 0;

		int index = 0;
		for (int i = 0; i < m_TempParticleArray.size(); i++)
		{
			//删除播放完的队列
			float currentTime = gHUD.m_flTime - m_TempParticleArray[i].flStartTime;

			if (currentTime < m_TempParticleArray[i].flLifeTime)
			{
				m_TempParticleArray[index] = m_TempParticleArray[i];
				index++;
			}
			/*else
			{
				m_TempParticleArray[i].bActive = false;
			}	*/
		}
		m_TempParticleArray.resize(index);


		for (auto iter = m_TempParticleArray.begin(); iter != m_TempParticleArray.end(); iter++)
		{
			//转世界坐标
			float currentTime = gHUD.m_flTime - iter->flStartTime;

			if (iter->Data.pTexture)
			{
				//float vUp[3];
				//float vRight[3];
				//float vNormal[3];
				//float vRenderOrigin[3];

				//IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);
				//vec3_t vecOrigin, vecRight, vecUp, vecColor, vecWorldOrigin;

				//StartAttachment = iter->vecStartAttachment + iter->vecDir * iter->flSpeed * currentTime;

				//VectorLerp(iter->vecStartAttachment, (iter->flSpeed * currentTime) / iter->flLifeTime, iter->vecEndAttachment, vecOrigin);

				//VectorLerp(iter->iStartColor, currentTime / iter->flLifeTime, iter->iEndColor, vecColor);

				iter->vecOrigin += iter->vecDir * iter->flSpeed * gHUD.m_flTimeDelta;
				//iter->vecColor[0] = vecColor[0];
				//iter->vecColor[1] = vecColor[1];
				//iter->vecColor[2] = vecColor[2];
				//iter->vecColor[3] = iter->flAlpha;
				VectorTransform(iter->vecOrigin, matrix, iter->vecWorldOrigin);
			}

			
		}


		// std::sort(m_TempParticleArray.begin(), m_TempParticleArray.end(), &compare);
		float vRenderOrigin[3];
		IEngineStudio.GetViewInfo(vRenderOrigin, NULL, NULL, NULL);
		vec3_t vecRenderOrigin = { vRenderOrigin[0],vRenderOrigin[1],vRenderOrigin[2], 0 };
		//排序vector
		std::sort(m_TempParticleArray.begin(), m_TempParticleArray.end(), [vecRenderOrigin](const ParticleInfo& a, const ParticleInfo& b) -> bool {
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
		for (auto iter = m_TempParticleArray.begin(); iter != m_TempParticleArray.end(); iter++)
		{
			if (iter->Data.pTexture)
			{
				float vUp[3];
				float vRight[3];
				float vNormal[3];
				float vRenderOrigin[3];

				IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);
				vec3_t vecOrigin, vecRight, vecUp, vecColor;

				vecRight[0] = vRight[0];
				vecRight[1] = vRight[1];
				vecRight[2] = vRight[2];

				vecUp[0] = vUp[0];
				vecUp[1] = vUp[1];
				vecUp[2] = vUp[2];

				DrawTGAQuad(iter->Data, iter->vecWorldOrigin, vecRight, vecUp, iter->flSize, iter->vecColor);
			}
		}


		return 1;
	}
	
}