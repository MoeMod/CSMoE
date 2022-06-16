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
#include "hud.h"
#include "cl_util.h"
#include "ammohistory.h"

#include <string.h>
#include <stdio.h>
#include "draw_util.h"
#include "calcscreen.h"
#include "com_model.h"

#include "triangleapi.h"

namespace cl {

#define BOUND_VALUE(var, min, max) if ((var) > (max)) { (var) = (max); }; if ((var) < (min)) { (var) = (min); }

struct SiFiammoItem
{
	vec3_t angle;
	vec3_t owner_org;
	vec3_t pos;
	int mdl_index;
	int value;
	char value_num[32];
};


SiFiammoItem g_clip, g_ammo;

int CHudSiFiammo:: Init( void )
{
	gHUD.AddHudElem( this );
	m_iFlags |= HUD_DRAW;

	hud_sifiammo_style = CVAR_CREATE( "hud_sifiammostyle", "0", FCVAR_ARCHIVE);
	current_style = (int)hud_sifiammo_style->value;
	return 1;
}

void CHudSiFiammo:: InitHUDData( void )
{
	g_clip.value = 0;
	g_ammo.value =  0;
}


int CHudSiFiammo:: VidInit( void )
{
	char path[128]; int i;
	switch ((int)hud_sifiammo_style->value ) {
	case 2: i = 1; break;
	case 3: i = 2; break;
	case 4: i = 3; break;
	case 5: i = 4; break;
	case 6: i = 5; break;
	case 7: i = 6; break;
	case 8: i = 7; break;
	case 9: i = 8; break;
	default: i = 1;
	}
	sprintf(path, "resource/floatingdamage/num_nomal_skin0%d.tga", i);
	R_InitTexture(m_iTex, path);
	//ConsolePrint("Numbers tga Loaded!");
	return 1;
}

void CHudSiFiammo::Shutdown(void)
{
	m_iTex = nullptr;
}

int CHudSiFiammo:: Draw( float flTime )
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_ALL)  )
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	if ((int)hud_sifiammo_style->value != current_style) {
		int new_style = (int)hud_sifiammo_style->value;
		if (new_style >= 2 && new_style <= 9) {
			current_style = new_style;
			m_iTex = nullptr;
			VidInit();
		}
	}
	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->Brightness(1.0);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);

	int r, g, b, scale;
	if ((int)hud_sifiammo_style->value == 1) {

		if (gHUD.m_Ammo.m_pWeapon == nullptr) return 1;
		if (gHUD.m_iFOV <= 40) return 1;
		int iSlot = gHUD.m_Ammo.m_pWeapon->iSlot;
		if (iSlot == 2 || iSlot == 3 || iSlot == 4) return 1;

		cl_entity_s* ent;
		ent = gEngfuncs.GetViewModel();
		vec3_t origin = ent->attachment[1];
		// hide hud if invalid attachment
		cl_entity_t* local = gEngfuncs.GetLocalPlayer();

		if (g_clip.angle != local->curstate.angles || g_clip.owner_org != local->curstate.origin || g_clip.mdl_index != ent->curstate.modelindex)
			if (g_clip.pos == ent->attachment[1]) return 1;

		g_clip.angle = local->curstate.angles;
		g_clip.owner_org = local->curstate.origin;
		g_clip.pos = ent->attachment[1];
		g_clip.mdl_index = ent->curstate.modelindex;

		float screen[2]{ -1,-1 };
		if (!CalcScreen(origin, screen))
			return 1;

		// offset, if needed
		//screen[0] += 0.03f *gHUD.m_scrinfo.iWidth;
		//BOUND_VALUE(screen[0], 0, gHUD.m_scrinfo.iWidth);
		//screen[1] += 0.06f *gHUD.m_scrinfo.iHeight;
		//BOUND_VALUE(screen[1], 0, gHUD.m_scrinfo.iHeight);

		g_clip.value = gHUD.m_Ammo.m_pWeapon->iClip;
		g_ammo.value = gWR.CountAmmo(gHUD.m_Ammo.m_pWeapon->iAmmoType);

		sprintf(g_clip.value_num, "%d", g_clip.value);
		int textlen1 = DrawUtils::HudStringLen(g_clip.value_num);
		sprintf(g_ammo.value_num, "%d", g_ammo.value);
		int textlen2 = DrawUtils::HudStringLen(g_ammo.value_num);

		//  Draw background
		int x, y, w, h;
		w = 4 * max(textlen1, textlen2) * gHUD.m_flScale;
		h = 64.0f * gHUD.m_flScale;
		x = screen[0] - w / 4;
		y = screen[1] - 3 * gHUD.m_flScale;
		FillRGBABlend(x, y, w, h, 0, 0, 0, 64);
		//set clip color
		if (g_clip.value == 0) { r = 255; g = 0; b = 0; }
		else { r = 255; g = 255; b = 255; }
		//draw clip
		if (g_clip.value >= 0)DrawUtils::DrawHudNumber(screen[0] - 0.5f * textlen1 * gHUD.m_flScale, screen[1], 1, g_clip.value, r, g, b);
		//DrawUtils::DrawHudString(screen[0] - textlen * 0.5f, screen[1], gHUD.m_scrinfo.iWidth, g_clip.value_num, r, g, b, scale);
	//set ammo color
		if (g_ammo.value == 0) { r = 255; g = 0; b = 0; }
		else { r = 255; g = 255; b = 255; }
		//draw ammo
		if (g_ammo.value >= 0)DrawUtils::DrawHudNumber(screen[0] - 0.5f * textlen2 * gHUD.m_flScale, screen[1] + 32.0f * gHUD.m_flScale, 1, g_ammo.value, r, g, b);


	}
	else if ((int)hud_sifiammo_style->value >= 2 && (int)hud_sifiammo_style->value <= 9) {

		if (gHUD.m_Ammo.m_pWeapon == nullptr) return 1;
		if (gHUD.m_iFOV <= 40) return 1;
		int iSlot = gHUD.m_Ammo.m_pWeapon->iSlot;
		if (iSlot == 2 || iSlot == 3 || iSlot == 4) return 1;

		cl_entity_s* ent;
		ent = gEngfuncs.GetViewModel();
		vec3_t origin = ent->attachment[1];
		// hide hud if invalid attachment
		cl_entity_t* local = gEngfuncs.GetLocalPlayer();

		if (g_clip.angle != local->curstate.angles || g_clip.owner_org != local->curstate.origin || g_clip.mdl_index != ent->curstate.modelindex)
			if (g_clip.pos == ent->attachment[1]) return 1;

		g_clip.angle = local->curstate.angles;
		g_clip.owner_org = local->curstate.origin;
		g_clip.pos = ent->attachment[1];
		g_clip.mdl_index = ent->curstate.modelindex;

		float screen[2]{ -1,-1 };
		if (!CalcScreen(origin, screen))
			return 1;

		// offset, if needed
		//screen[0] += 0.03f *gHUD.m_scrinfo.iWidth;
		//BOUND_VALUE(screen[0], 0, gHUD.m_scrinfo.iWidth);
		//screen[1] += 0.06f *gHUD.m_scrinfo.iHeight;
		//BOUND_VALUE(screen[1], 0, gHUD.m_scrinfo.iHeight);

		g_clip.value = gHUD.m_Ammo.m_pWeapon->iClip;
		g_ammo.value = gWR.CountAmmo(gHUD.m_Ammo.m_pWeapon->iAmmoType);

		float fScale1 = 0.8f;
		float fScale2 = 0.4f;
		int value1 = g_clip.value;
		int value2 = g_ammo.value;

		int count1 = 1;
		while (value1 >= 10) {
			value1 /= 10;
			count1++;
		}

		int count2 = 1;
		while (value2 >= 10) {
			value2 /= 10;
			count2++;
		}

		int gap1 = 0;
		gap1 = fScale1 * m_iTex->w() / 10.0f;
		int gap_height1 = 0;
		gap_height1 = fScale1 * m_iTex->h();

		int gap2 = 0;
		gap2 = fScale2 * m_iTex->w() / 10.0f;
		int gap_height2 = 0;
		gap_height2 = fScale2 * m_iTex->h();

		//draw background
		int x, y, w, h;
		w = max(count2 * gap2, count1 * gap1);
		h = 1.4f * gap_height1 + gap_height2;
		x = screen[0] - w / 2;
		y = screen[1] - gap_height1 / 2;
		FillRGBABlend(x, y, w, h, 0, 0, 0, 64);

		value1 = g_clip.value;
		value2 = g_ammo.value;
		//set clip color
		if (value1 == 0) { r = 255; g = 0; b = 0; }
		else { r = 255; g = 255; b = 255; }
		//draw clip
		if (value1 >= 0)for (int j = 1; j <= count1; j++) {
			Draw2DQuad_Custom(screen[0] + gap1 * ((count1 + 1) / 2.0f - j), screen[1], fScale1, value1 % 10, r, g, b, 255);
			value1 /= 10;
		}

		//set ammo color
		if (value2 == 0) { r = 255; g = 0; b = 0; }
		else { r = 255; g = 255; b = 255; }
		//draw ammo
		if (value2 >= 0)for (int j = 1; j <= count2; j++) {
			Draw2DQuad_Custom(screen[0] + gap2 * ((count2 + 1) / 2.0f - j), screen[1] + 1.0f * gap_height1, fScale2, value2 % 10, r, g, b, 255);
			value2 /= 10;
		}

	}
	return 1;
}

void CHudSiFiammo::Draw2DQuad_Custom(float x, float y, float scale, int num, int r, int g, int b, int alpha) //m_flScale;
{
	if (!m_iTex->valid()) return;

	gEngfuncs.pTriAPI->Color4ub(r, g, b, alpha);
	float w = m_iTex->w() / 10.0f / 2.0f, h = m_iTex->h() / 2.0f;
	w *= scale;
	h *= scale;
	m_iTex->Draw2DQuadScaled(x - w, y - h, x + w, y + h, 0.1 * num, 0, 0.1 * (num + 1), 1);
}

}
