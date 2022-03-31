/***+
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

#include "hud.h"

#include "cl_util.h"
#include "const.h"
#include "entity_types.h"
#include "cdll_int.h"
#include "pm_defs.h"
#include "event_api.h"
#include "triangleapi.h"

#include "parsemsg.h"
#include "fog.h"

namespace cl {

// Class declaration
CFog gFog;

int __MsgFunc_Fog(const char* pszName, int iSize, void* pBuf)
{
	return gFog.MsgFunc_Fog(pszName, iSize, pBuf);
}

/*
====================
Init

====================
*/
void CFog::Init(void)
{
	HOOK_MESSAGE(Fog);
}

/*
====================
VidInit

====================
*/
void CFog::VidInit(void)
{
	m_iFogSkyBox = 0;
	memset(&m_fogParams, 0, sizeof(fog_params_t));
	memset(&m_fogBlend1, 0, sizeof(fog_params_t));
	memset(&m_fogBlend2, 0, sizeof(fog_params_t));
}

/*
====================
SetGLFog

====================
*/
void CFog::SetGLFog(vec3_t& color)
{
	if (!m_fogParams.enddist && !m_fogParams.startdist)
	{
		gEngfuncs.pTriAPI->Fog(Vector(0, 0, 0), 0, 0, FALSE);
		return;
	}

	BlendFog();
	gEngfuncs.pTriAPI->FogParams(m_fogParams.density, m_iFogSkyBox);
	gEngfuncs.pTriAPI->Fog(color, 0, 0, TRUE);
}

/*
====================
V_CalcRefDef

====================
*/
void CFog::V_CalcRefdef(const ref_params_t* pparams)
{
	// Calculate distance to edge
	//Vector boxTotal = Vector(m_fogParams.enddist, m_fogParams.enddist, m_fogParams.enddist);
	//float edgeLength = boxTotal.Length();

	//// Set mins/maxs box
	//for (int i = 0; i < 3; i++)
	//{
	//	m_vFogBBoxMin[i] = pparams->vieworg[i] - edgeLength;
	//	m_vFogBBoxMax[i] = pparams->vieworg[i] + edgeLength;
	//}

	// Remember this
	m_clientWaterLevel = pparams->waterlevel;

	// Set the normal fog color
	SetGLFog(m_fogParams.color);
}

/*
====================
HUD_DrawNormalTriangles

====================
*/
void CFog::HUD_DrawNormalTriangles(void)
{
	gEngfuncs.pTriAPI->Fog(Vector(0, 0, 0), 0, 0, FALSE);
}

/*
====================
BlendFog

====================
*/
void CFog::BlendFog(void)
{
	if (!m_fogChangeTime && !m_fogBlendTime)
		return;

	// Clear if blend is over
	if ((m_fogChangeTime + m_fogBlendTime) < gEngfuncs.GetClientTime())
	{
		memcpy(&m_fogParams, &m_fogBlend2, sizeof(fog_params_t));
		memset(&m_fogBlend1, 0, sizeof(fog_params_t));
		memset(&m_fogBlend2, 0, sizeof(fog_params_t));

		m_fogChangeTime = 0;
		m_fogBlendTime = 0;
		if (CVAR_GET_FLOAT("developer"))
		{
			gEngfuncs.Con_Printf("FOG Blend Over --- density:%f \n", m_fogParams.density);
		}
		return;
	}

	// Perform linear blending
	float interp = (gEngfuncs.GetClientTime() - m_fogChangeTime) / m_fogBlendTime;
	float fraction = 1.0 - interp;
	VectorScale(m_fogBlend1.color, fraction, m_fogParams.color);
	VectorMA(m_fogParams.color, interp, m_fogBlend2.color, m_fogParams.color);
	m_fogParams.enddist = (m_fogBlend1.enddist * fraction) + (m_fogBlend2.enddist * interp);
	m_fogParams.startdist = (m_fogBlend1.startdist * fraction) + (m_fogBlend2.startdist * interp);
	m_fogParams.density = (m_fogBlend1.density * fraction) + (m_fogBlend2.density * interp);
}

/*
====================
CullFogBBox

====================
*/
bool CFog::CullFogBBox(const vec3_t& mins, const vec3_t& maxs)
{
	if (!m_fogParams.enddist && !m_fogParams.startdist || m_clientWaterLevel == 3)
		return false;

	if (mins[0] > m_vFogBBoxMax[0])
		return true;

	if (mins[1] > m_vFogBBoxMax[1])
		return true;

	if (mins[2] > m_vFogBBoxMax[2])
		return true;

	if (maxs[0] < m_vFogBBoxMin[0])
		return true;

	if (maxs[1] < m_vFogBBoxMin[1])
		return true;

	if (maxs[2] < m_vFogBBoxMin[2])
		return true;

	return false;
}

/*
====================
MsgFunc_Fog

====================
*/
int CFog::MsgFunc_Fog(const char* pszName, int iSize, void* pBuf)
{
	BufferReader reader(pszName, pBuf, iSize);

	vec3_t fogcolor;
	for (int i = 0; i < 3; i++)
		fogcolor[i] = (float)reader.ReadByte() / 255.0f;

	union
	{
		char b[4];
		float f;

	} density;
#ifdef XASH_BIG_ENDIAN
	for (int i = 3; i >= 0; i--)
		density.b[i] = reader.ReadByte();
#else
	for (int i = 0; i < 4; i++)
		density.b[i] = reader.ReadByte();
#endif

	if (CVAR_GET_FLOAT("developer"))
	{
		gEngfuncs.Con_Printf("FOG --- r:%d g:%d b:%d density:%f \n", fogcolor[0], fogcolor[1], fogcolor[2], density.f);
	}

	int startdist = !density.f ? 0 : 1;
	int enddist = !density.f ? 0 : min(1, 0.25f / density.f);
	float blendtime = reader.ReadByte() * 0.1f;

	// If blending, copy current fog params to the blend state if we had any active
	if (blendtime > 0 && m_fogParams.enddist > 0 && m_fogParams.startdist > 0)
	{
		memcpy(&m_fogBlend1, &m_fogParams, sizeof(fog_params_t));

		VectorCopy(fogcolor, m_fogBlend2.color);
		m_fogBlend2.startdist = startdist;
		m_fogBlend2.enddist = enddist;
		m_fogBlend2.density = density.f;

		// Set times
		m_fogChangeTime = gEngfuncs.GetClientTime();
		m_fogBlendTime = blendtime;
	}
	else
	{
		// No blending, just set
		memset(&m_fogBlend1, 0, sizeof(fog_params_t));
		memset(&m_fogBlend2, 0, sizeof(fog_params_t));

		VectorCopy(fogcolor, m_fogParams.color);
		m_fogParams.startdist = startdist;
		m_fogParams.enddist = enddist;
		m_fogParams.density = density.f;

		m_fogChangeTime = 0;
		m_fogBlendTime = 0;
	}

	return 1;
}
}