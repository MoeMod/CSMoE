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
#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include "calcscreen.h"
#include "com_model.h"


DECLARE_MESSAGE(m_HitIndicator, HitMsg )


struct HitIndicatorItem 
{
	int iVictim;
	char damage_num[32];
	int iDamage;
	vec3_t fOrigin;
	float random_offset;

	int iId;	// the index number of the associated sprite
	float flDisplayTime;
	int iHeadShot;
	float fRange;
};

#define MAX_INDICATORS	32
#define DEATHNOTICE_TOP		32

#define DAMAGENUM_DISPLAY_TIME 0.75f


HitIndicatorItem rgHitIndicatorList[MAX_INDICATORS + 1 ];

int CHudHitIndicator:: Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(HitMsg);

	hud_hitindicator_style = CVAR_CREATE( "hud_hitstyle", "2", FCVAR_ARCHIVE);
	current_style = 3;   //Ĭ����ë������
	return 1;
}

void CHudHitIndicator::Reset(void)
{
	std::fill(std::begin(m_iTex), std::end(m_iTex), nullptr);
	VidInit();
}

void CHudHitIndicator:: InitHUDData( void )
{
	memset(rgHitIndicatorList, 0, sizeof(rgHitIndicatorList) );
}


int CHudHitIndicator:: VidInit( void )
{
	char path[128]; int i;
	switch ((int)hud_hitindicator_style->value ) {
	case 2: i = 1; break;
	case 3: i = 2; break;
	case 4: i = 3; break;
	case 5: i = 4; break;
	case 6: i = 5; break;
	case 7: i = 6; break;
	case 8: i = 7; break;
	default: i = 1;
	}
	sprintf(path, "resource/floatingdamage/num_nomal_skin0%d.tga", i);
	R_InitTexture(m_iTex[0], path);
	sprintf(path, "resource/floatingdamage/num_head_skin0%d.tga", i);
	R_InitTexture(m_iTex[1], path);
	sprintf(path, "resource/floatingdamage/num_miss_skin0%d.tga", i);
	R_InitTexture(m_iTex[2], path);
	sprintf(path, "resource/floatingdamage/icon_critical_skin0%d.tga", i);
	R_InitTexture(m_iTex[3], path);
	sprintf(path, "resource/floatingdamage/icon_backatk_skin0%d.tga", i);
	R_InitTexture(m_iTex[4], path);

	return 1;
}

void CHudHitIndicator::Shutdown(void)
{
	std::fill(std::begin(m_iTex), std::end(m_iTex), nullptr);
}

int CHudHitIndicator:: Draw( float flTime )
{
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)  )
		return 1;
	HitIndicatorItem *pList = rgHitIndicatorList;

	if ((int)hud_hitindicator_style->value != current_style) {
		int new_style = (int)hud_hitindicator_style->value;
		if (new_style >= 2 && new_style <= 8) {
			current_style = new_style;
			Reset();
		}
	}
	int r, g, b, scale;
	if ( (int)hud_hitindicator_style ->value == 1) {
		for (int i = 0; i < MAX_INDICATORS; i++)
		{
			if (pList[i].iId == 0)
				break;  // we've gone through them all

			//if (g_PlayerExtraInfo[i].dead)
				//continue;

			if (pList[i].flDisplayTime < flTime)
			{ // display time has expired
				// remove the current item from the list
				memmove(&pList[i], &pList[i + 1], sizeof(HitIndicatorItem) * (MAX_INDICATORS - i));
				i--;  // continue on the next item;  stop the counter getting incremented
				continue;
			}

			pList[i].flDisplayTime = min(pList[i].flDisplayTime, flTime + DAMAGENUM_DISPLAY_TIME);

			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(pList[i].iVictim);
			vec3_t origin = ent->origin;
			if (ent->curstate.solid == SOLID_BSP && !ent->player) {
				origin = ((ent->curstate.origin + ent->curstate.maxs) + (ent->curstate.origin + ent->curstate.mins)) / 2.0f;
			}
			float fixed_offset = (DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime + flTime) / DAMAGENUM_DISPLAY_TIME;
			if (fixed_offset >= 0.5) fixed_offset = 0.5;
			fixed_offset *= pList[i].fRange / 10.0f;
			origin.z += fixed_offset;

			model_t *model = ent->model;
			if (model)
				origin.z += max(model->maxs.z, 35.0);

			float screen[2]{ -1,-1 };
			if (!CalcScreen(origin, screen))
				continue;

			int textlen = DrawUtils::HudStringLen(pList[i].damage_num);
			if (pList[i].iHeadShot == 1) {
				r = 150; g = 0; b = 0; scale = 3.0;
			}
			else {
				r = 150; g = 150; b = 150; scale = 2.0;
			}
			DrawUtils::DrawHudString(screen[0] - textlen * 0.5f, screen[1], gHUD.m_scrinfo.iWidth, pList[i].damage_num, r, g, b, scale);
		}
	}
	else if ((int)hud_hitindicator_style->value >= 2 && (int)hud_hitindicator_style->value <= 8) {

		for (int i = 0; i < MAX_INDICATORS; i++)
		{
			if (pList[i].iId == 0)
				break;  // we've gone through them all

			//if (g_PlayerExtraInfo[i].dead)
				//continue;

			if (pList[i].flDisplayTime < flTime)
			{ // display time has expired
				// remove the current item from the list
				memmove(&pList[i], &pList[i + 1], sizeof(HitIndicatorItem) * (MAX_INDICATORS - i));
				i--;  // continue on the next item;  stop the counter getting incremented
				continue;
			}

			pList[i].flDisplayTime = min(pList[i].flDisplayTime, flTime + DAMAGENUM_DISPLAY_TIME);

			float fixed_offset = 0.1f * gHUD.m_flScale*ScreenHeight * (DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime + flTime) / DAMAGENUM_DISPLAY_TIME;
			float fScale = 1.0f - 1.0f * (DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime + flTime) / DAMAGENUM_DISPLAY_TIME;
			
			int alpha = 3 * (255 - 255 * (DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime + flTime) / DAMAGENUM_DISPLAY_TIME);
			if (alpha > 255) alpha = 255;

			if (i == MAX_INDICATORS) fScale += 0.2f;
			else if (pList[i+1].iId == 0) fScale += 0.2f;    //Newest one larger

			fScale -= pList[i].fRange / 2000.0f;  //Differs by distance

			if (fScale < 0.5) fScale = 0.5f;
			if (alpha < 0) alpha = 0;

			if (pList[i].iHeadShot) fScale += 0.2f;

			float screen[2]{ -1,-1 };
			if (!CalcScreen(pList[i].fOrigin, screen))
				continue;
			screen[1] -= fixed_offset; 
			screen[0] += pList[i].random_offset;

			int iDamage = pList[i].iDamage;
			
			int count = 1;
			while (iDamage >= 10) {
				iDamage /= 10;
				count++;
			}

			int gap = 0;
			if(m_iTex[pList[i].iHeadShot]->valid()) gap = fScale * m_iTex[pList[i].iHeadShot]->w() / 10.0f;

			iDamage = pList[i].iDamage;

			char str[128];
			for (int j = 1; j <= count; j++) {
				Draw2DQuad_Custom(screen[0] + gap * ((count+1) / 2.0f - j), screen[1], fScale, iDamage % 10, pList[i].iHeadShot, alpha);
				iDamage /= 10;
			}
		}
	}

	return 1;
}

// This message handler may be better off elsewhere
int CHudHitIndicator::MsgFunc_HitMsg( const char *pszName, int iSize, void *pbuf )
{
	m_iFlags |= HUD_DRAW;

	int i;
	for (i = 0; i < MAX_INDICATORS; i++)
	{
		if (rgHitIndicatorList[i].iId == 0)
			break;
	}
	if (i == MAX_INDICATORS)
	{ // move the rest of the list forward to make room for this item
		memmove(rgHitIndicatorList, rgHitIndicatorList + 1, sizeof(HitIndicatorItem) * MAX_INDICATORS);
		i = MAX_INDICATORS - 1;
	}

	BufferReader reader(pszName, pbuf, iSize);

	rgHitIndicatorList[i].iDamage = reader.ReadLong();

	int iVictim = reader.ReadShort();
	int iHead = reader.ReadByte();

	rgHitIndicatorList[i].iId = 1;
	sprintf(rgHitIndicatorList[i].damage_num, "%d", rgHitIndicatorList[i].iDamage);
	rgHitIndicatorList[i].iVictim = iVictim;  
	if(iHead == 1) rgHitIndicatorList[i].iHeadShot = 1;
	else rgHitIndicatorList[i].iHeadShot = 0;
	rgHitIndicatorList[i].flDisplayTime = gHUD.m_flTime + DAMAGENUM_DISPLAY_TIME;
	rgHitIndicatorList[i].random_offset = gHUD.m_flScale * 0.1f * (rand() % (ScreenWidth / 2) - (ScreenWidth / 4));

	cl_entity_t *ent = gEngfuncs.GetEntityByIndex(iVictim);

	vec3_t center = ent->origin;
	if (ent->curstate.solid == SOLID_BSP && !ent->player) {
		center = ((ent->curstate.origin + ent->curstate.maxs) + (ent->curstate.origin + ent->curstate.mins)) / 2.0f;
	}

	rgHitIndicatorList[i].fRange = (gEngfuncs.GetLocalPlayer()->origin - center).Length();
	rgHitIndicatorList[i].fOrigin = center;

	model_t *model = ent->model;
	if (model)
		rgHitIndicatorList[i].fOrigin.z += max(model->maxs.z, 35.0);

	return 1;
}


void CHudHitIndicator::Draw2DQuad_Custom(float x, float y, float scale, int num, int iTexID, int alpha) //m_flScale;
{
	if (!m_iTex[iTexID]->valid()) return;
	x *= gHUD.m_flScale;    //Xash3D feature
	y *= gHUD.m_flScale;

	float w = m_iTex[iTexID]->w() / 10.0f / 2.0f, h = m_iTex[iTexID]->h() / 2.0f;
	w *= scale;
	h *= scale;
	w *= gHUD.m_flScale;     //Xash3D feature
	h *= gHUD.m_flScale;

	m_iTex[iTexID]->Draw2DQuad(x - w, y - h, x + w, y + h, 0.1*num , 0, 0.1*(num + 1), 1, 255, 255, 255, alpha);
}