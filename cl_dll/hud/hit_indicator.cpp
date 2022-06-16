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
#include "triangleapi.h"

namespace cl {

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
	byte byFlag;
	float fRange;
};

#define MAX_INDICATORS	32
#define DEATHNOTICE_TOP		32

#define DAMAGENUM_DISPLAY_TIME 0.75f

#define HIT_FLAGS_HEAD (1<<0)
#define HIT_FLAGS_CRITICAL (1<<1)
#define HIT_FLAGS_BACKATK (1<<2)
#define HIT_FLAGS_BURN (1<<3)

HitIndicatorItem pList[MAX_INDICATORS + 1 ];

int CHudHitIndicator:: Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_DRAW;
	HOOK_MESSAGE(HitMsg);

	hud_hitindicator_style = CVAR_CREATE( "hud_hitstyle", "2", FCVAR_ARCHIVE);
	current_style = 3;
	return 1;
}

void CHudHitIndicator:: InitHUDData( void )
{
	memset(pList, 0, sizeof(pList) );
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
	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL))
		return 1;


	if ((int)hud_hitindicator_style->value != current_style) {
		int new_style = (int)hud_hitindicator_style->value;
		if (new_style >= 2 && new_style <= 8) {
			current_style = new_style;
			std::fill(std::begin(m_iTex), std::end(m_iTex), nullptr);
			VidInit();
		}
	}
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Brightness(1.0);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);

	if ((int)hud_hitindicator_style->value == 1) {
		for (int i = 0; i < MAX_INDICATORS; i++)
		{
			if (pList[i].iId == 0)
				break;  // we've gone through them all

			if (pList[i].flDisplayTime < flTime)
			{ // display time has expired
				// remove the current item from the list
				memmove(&pList[i], &pList[i + 1], sizeof(HitIndicatorItem) * (MAX_INDICATORS - i));
				i--;  // continue on the next item;  stop the counter getting incremented
				continue;
			}

			float fProgress;
			float fScale;
			float fOffset;
			byte byFlag = pList[i].byFlag;

			int r, g, b;

			fProgress = (flTime + DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime) / DAMAGENUM_DISPLAY_TIME;

			cl_entity_t* ent = gEngfuncs.GetEntityByIndex(pList[i].iVictim);
			vec3_t origin = ent->origin;
			fOffset = fProgress;
			if (fOffset >= 1.0f) fOffset = 1.0f;
			fOffset *= pList[i].fRange / 10.0f;
			origin.z += fOffset;

			model_t* model = ent->model;
			if (model)
				origin.z += max(model->maxs.z, 35.0);

			float screen[2]{ -1,-1 };
			if (!CalcScreen(origin, screen))
				continue;

			int textlen = DrawUtils::HudStringLen(pList[i].damage_num);
			if (byFlag & HIT_FLAGS_HEAD) {
				r = 150; g = 0; b = 0; fScale = 3.0f;
			}
			else {
				r = 150; g = 150; b = 150; fScale = 2.0f; //scale not working
			}
			DrawUtils::DrawHudString(screen[0] - textlen * 0.5f, screen[1], ScreenWidth, pList[i].damage_num, r, g, b, fScale);
		}
	}
	else if ((int)hud_hitindicator_style->value >= 2 && (int)hud_hitindicator_style->value <= 8) {

		for (int i = 0; i < MAX_INDICATORS; i++)
		{
			if (pList[i].iId == 0)
				break;  // we've gone through them all

			if (pList[i].flDisplayTime < flTime)
			{ // display time has expired
				// remove the current item from the list
				memmove(&pList[i], &pList[i + 1], sizeof(HitIndicatorItem) * (MAX_INDICATORS - i));
				i--;  // continue on the next item;  stop the counter getting incremented
				continue;
			}

			BOOL bIsNewest = false;
			float fProgress;
			float fScale;
			float fOffset;
			float iAlpha;
			byte byFlag = pList[i].byFlag;

			if (i == MAX_INDICATORS || pList[i + 1].iId == 0) bIsNewest = true; //Newest one larger

			fProgress = (flTime + DAMAGENUM_DISPLAY_TIME - pList[i].flDisplayTime) / DAMAGENUM_DISPLAY_TIME;
			if (!bIsNewest) fProgress += 0.2f;
			if (fProgress < 0.0f || fProgress >= 1.0f) continue;

			fOffset = 0.1f * gHUD.m_flScale * ScreenHeight * fProgress;
			fScale = 1.25f - fProgress;

			iAlpha = 3 * (255 - 255 * fProgress);
			if (iAlpha > 255) iAlpha = 255;

			fScale -= pList[i].fRange / 2000.0f;  //Differs by distance

			if (fScale < 0.5f || !bIsNewest) fScale = 0.5f;
			if (iAlpha < 0) iAlpha = 0;

			if (byFlag & HIT_FLAGS_HEAD) fScale += 0.25f;    // 1==head 2==critical 3==backattack 4==burn

			float screen[2]{ -1,-1 };
			if (!CalcScreen(pList[i].fOrigin, screen))
				continue;
			screen[1] -= fOffset;
			screen[0] += pList[i].random_offset;

			int iDamage = pList[i].iDamage;

			int count = 1;
			while (iDamage >= 10) {
				iDamage /= 10;
				count++;
			}

			int gap = 0;
			int isheadshot;
			if (byFlag & HIT_FLAGS_HEAD) isheadshot = 1;
			else isheadshot = 0;
			if (m_iTex[isheadshot]->valid()) gap = fScale * m_iTex[isheadshot]->w() / 10.0f;

			iDamage = pList[i].iDamage;

			char str[128];
			for (int j = 1; j <= count; j++) {
				Draw2DQuad_Custom(screen[0] + gap * ((count + 1) / 2.0f - j), screen[1], fScale, iDamage % 10, isheadshot, iAlpha);
				iDamage /= 10;
			}

			float x, y;
			if (byFlag & HIT_FLAGS_CRITICAL) {   //Critical

				x = screen[0] - (gap * (count / 2.0f));
				y = screen[1] - fScale * m_iTex[0]->h() / 2.0f;
				Draw2DQuad_Custom(x, y, fScale, 0, 3, iAlpha);
			}

			if (byFlag & HIT_FLAGS_BACKATK) {   //BackAttack
				x = screen[0] + fScale * m_iTex[3]->w() / 2.0f;
				y = screen[1] - fScale * (m_iTex[4]->h() / 2.0f + m_iTex[0]->h() / 2.0f);
				Draw2DQuad_Custom(x, y, fScale, 0, 4, iAlpha);
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
		if (pList[i].iId == 0)
			break;
	}
	if (i == MAX_INDICATORS)
	{ // move the rest of the list forward to make room for this item
		memmove(pList, pList + 1, sizeof(HitIndicatorItem) * MAX_INDICATORS);
		i = MAX_INDICATORS - 1;
	}

	BufferReader reader(pszName, pbuf, iSize);

	pList[i].iDamage = reader.ReadLong();

	int iVictim = reader.ReadShort();
	byte byFlag = reader.ReadByte();

	pList[i].iId = 1;
	sprintf(pList[i].damage_num, "%d", pList[i].iDamage);
	pList[i].iVictim = iVictim;
	pList[i].byFlag = byFlag;
	pList[i].flDisplayTime = gHUD.m_flTime + DAMAGENUM_DISPLAY_TIME;
	pList[i].random_offset = gHUD.m_flScale * 0.1f * (rand() % (ScreenWidth / 2) - (ScreenWidth / 4));

	cl_entity_t* ent = gEngfuncs.GetEntityByIndex(iVictim);
	pList[i].fRange = (gEngfuncs.GetLocalPlayer()->origin - ent->origin).Length();


	//if (!CheckForPlayer(ent))
		//continue;

	pList[i].fOrigin = ent->origin;

	model_t* model = ent->model;
	if (model)
		pList[i].fOrigin.z += max(model->maxs.z, 35.0);

	return 1;
}


void CHudHitIndicator::Draw2DQuad_Custom(float x, float y, float scale, int num, int iTexID, int alpha) //m_flScale;
{
	if (!m_iTex[iTexID]->valid()) return;
	x *= gHUD.m_flScale;    //Xash3D feature
	y *= gHUD.m_flScale;

	float w, h;

	if (iTexID < 2) { w = m_iTex[iTexID]->w() / 10.0f / 2.0f; h = m_iTex[iTexID]->h() / 2.0f; }
	else { w = m_iTex[iTexID]->w() / 2.0f; h = m_iTex[iTexID]->h() / 2.0f; }
	w *= scale;
	h *= scale;
	w *= gHUD.m_flScale;     //Xash3D feature
	h *= gHUD.m_flScale;

	float a1, a2, a3, a4;
	if (iTexID < 2) { a1 = 0.1 * num; a2 = 0; a3 = 0.1 * (num + 1); a4 = 1; }
	else { a1 = 0; a2 = 0; a3 = 1; a4 = 1; }

	m_iTex[iTexID]->Draw2DQuad(x - w, y - h, x + w, y + h, a1 , 0, a3, 1, 255, 255, 255, alpha);
}
}