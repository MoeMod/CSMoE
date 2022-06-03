/*
zbz_skill.cpp - CSMoE Client HUD : elements for Zombie Skills
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "hud.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"

#include "zbz.h"
#include "zbz_ghosthunter.h"

#include <set>

namespace cl {

CHudZBZ_GhostHunter::CHudZBZ_GhostHunter(void)
{
	
}

int CHudZBZ_GhostHunter::VidInit(void)
{
	m_flTime = 0.0;

	if (!m_iGhostHunterAppear)
		m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/ghosthunter_appear.tga");

	return 1;
}

void CHudZBZ_GhostHunter::Reset(void)
{
}

int CHudZBZ_GhostHunter::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	if (m_flTime < time)
		return 0;

	switch (m_iType)
	{
	case CLASS_GHOSTHUNTER:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/ghosthunter_appear.tga"); break;
	case CLASS_MASTERHUNTER:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/masterhunter_appear.tga"); break;
	case CLASS_TIMEHUNTER:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/timehunter_appear.tga"); break;
	case CLASS_ASCETICHERO:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/ascetic_appear.tga"); break;
	case CLASS_BACKGROUND:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/ghosthunterselect.tga"); break;
	case CLASS_HEALHUNTER:m_iGhostHunterAppear = R_LoadTextureUnique("resource/zombi/healhunter_appear.tga"); break;
	default:
		break;
	}

	int iX, iY, iH, iW;

	if (m_iGhostHunterAppear) 
	{
		iW = m_iGhostHunterAppear->w();
		iH = m_iGhostHunterAppear->h();
		iX = ScreenWidth / 2;
		iY = ScreenHeight / 2;

		iX = iX - iW / 2;
		iY = iY - iH / 2;

		m_iGhostHunterAppear->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH);
	}

	return 1;
}

void CHudZBZ_GhostHunter::SetImageDrawTime(float time)
{
	m_flTime = gHUD.m_flTime + time;
}

void CHudZBZ_GhostHunter::SetGhostHunterClass(int type)
{
	m_iType = type;
}

}
