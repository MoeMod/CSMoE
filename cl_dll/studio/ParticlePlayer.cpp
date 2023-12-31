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
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include "ParticlePlayer.h"
#include "mathlib.h"
#include "r_studioint.h"

namespace cl {

	extern engine_studio_api_t IEngineStudio;


	int CParticlePlayer::Init(void)
	{
		m_TempParticleArray.clear();

		R_InitTexture(test.Data.pTexture, "resource/weapontexture/SGFX_se_glow_05.tga");
		CalcData(test.Data);

		test.flIntervalTime = 0.01;
		test.flLifeTime = 0.8;
		test.flSize = 0.2;
		test.flSpeed = 0.7;
		test.flAlpha = 200;
		test.iStartAttachment = 1;
		test.iEndAttachment = 2;
		test.iStartColor = { 0, 135, 255 };
		test.iEndColor = { 50, 0, 200 };
		m_flNextTime = 0;
		return 1;
	}

	void CParticlePlayer::CalcData(ParticleData& info)
	{
		int w = info.pTexture->w();
		int h = info.pTexture->h();

		info.up = (h >> 1);
		info.left = -(w >> 1);
		info.down = (h >> 1) - h;
		info.right = w + -(w >> 1);
	}

	void CParticlePlayer::DrawTGAQuad(ParticleData& particle, vec3_t org, vec3_t v_right, vec3_t v_up, float scale, vec4_t color)
	{
		vec3_t	point;

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
		gRenderAPI.GL_Upload(particle.pTexture->texnum());
		gRenderAPI.GL_Bind(0, particle.pTexture->texnum());
		gEngfuncs.pTriAPI->Color4ub(color[0], color[1], color[2], color[3]);

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

	void CParticlePlayer::AddAnimInfo()
	{
		if (gHUD.m_flTime > m_flNextTime)
		{
			AddToTempParticleArray(test);
			m_flNextTime = gHUD.m_flTime + test.flIntervalTime;
		}
			
	}

	void CParticlePlayer::AddToTempParticleArray(ParticleInfo& info)
	{
		m_TempParticleArray.emplace_back();
		ParticleInfo& Temp = m_TempParticleArray.back();

		Temp.flAlpha = info.flAlpha;
		Temp.flIntervalTime = info.flIntervalTime;
		Temp.flLifeTime = info.flLifeTime;
		Temp.flSize = info.flSize;
		Temp.flSpeed = info.flSpeed;
		Temp.iStartAttachment = info.iStartAttachment;
		Temp.iEndAttachment = info.iEndAttachment;
		Temp.iStartColor = info.iStartColor;
		Temp.iEndColor = info.iEndColor;
		Temp.Data = info.Data;
		Temp.iId = m_TempParticleArray.size() - 1;
		Temp.flStartTime = gHUD.m_flTime;
		Temp.bActive = true;


		cl_entity_t* viewent = gEngfuncs.GetViewModel();

		if (viewent)
		{
			Temp.vecDir = (viewent->attachment[info.iEndAttachment] - viewent->attachment[info.iStartAttachment]).Normalize();
			Temp.vecStartAttachment = viewent->attachment[info.iStartAttachment];
			Temp.vecEndAttachment = viewent->attachment[info.iEndAttachment];
			
		}
		else
		{
			 Temp.vecDir = Temp.vecEndAttachment = Temp.vecStartAttachment = { 0, 0, 0 };
		}
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


		return (Particle1.vecOrigin - vecRenderOrigin).Length() > (Particle2.vecOrigin - vecRenderOrigin).Length();
	}


	void CParticlePlayer::UpdateTempAnimation(void)
	{
		cl_entity_t* viewent = gEngfuncs.GetViewModel();

		if (!viewent)
			return;


		for (auto iter = m_TempParticleArray.begin(); iter != m_TempParticleArray.end();/**/)
		{

			float currentTime = gHUD.m_flTime - iter->flStartTime;

			if (currentTime > iter->flLifeTime)
			{
				iter->bActive = false;
			}

			if (!iter->bActive)
			{
				iter = m_TempParticleArray.erase(iter);
				continue;
			}

			if (iter->Data.pTexture)
			{
				float vUp[3];
				float vRight[3];
				float vNormal[3];
				float vRenderOrigin[3];

				IEngineStudio.GetViewInfo(vRenderOrigin, vUp, vRight, vNormal);
				vec3_t vecOrigin, vecRight, vecUp, vecColor;
				vec3_t StartAttachment = viewent->attachment[iter->iStartAttachment];
				vec3_t EndAttachment = viewent->attachment[iter->iEndAttachment];

				if (iter->vecDir.IsZero())
				{
					VectorLerp(StartAttachment, currentTime / iter->flLifeTime, EndAttachment, vecOrigin);
				}
				else
				{
					//StartAttachment = iter->vecStartAttachment + iter->vecDir * iter->flSpeed * currentTime;

					VectorLerp(iter->vecStartAttachment, (iter->flSpeed * currentTime) / iter->flLifeTime, iter->vecEndAttachment, vecOrigin);
				}

				VectorLerp(iter->iStartColor, currentTime / iter->flLifeTime, iter->iEndColor, vecColor);

				iter->vecOrigin = vecOrigin;
				iter->vecColor[0] = vecColor[0];
				iter->vecColor[1] = vecColor[1];
				iter->vecColor[2] = vecColor[2];
				iter->vecColor[3] = iter->flAlpha;

			}

			++iter;
		}
			

		std::sort(m_TempParticleArray.begin(), m_TempParticleArray.end(), &compare);

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

				DrawTGAQuad(iter->Data, iter->vecOrigin, vecRight, vecUp, iter->flSize, iter->vecColor);
			}
		}
	

	}

	
	int CParticlePlayer::Draw(float flTime)
	{
		UpdateTempAnimation();
		return 1;
	}
	
}