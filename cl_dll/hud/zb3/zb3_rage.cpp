/*
zb3_rage.cpp - CSMoE Client HUD : Zombie Hero Zombie Rage System
Copyright (C) 2019 Moemod Hyakuya

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
#include "eventscripts.h"

#include "zb3.h"
#include "zb3_rage.h"

#include "gamemode/mods_const.h"
#include "gamemode/zb1/zb1_const.h"
#include "gamemode/zb3/zb3_const.h"

namespace cl {

CHudZB3Rage::CHudZB3Rage(void)
{

}

int CHudZB3Rage::VidInit(void)
{
	m_iRageBG = gHUD.GetSpriteIndex("ZB3_RageBg");
	m_iRageLevel = gHUD.GetSpriteIndex("ZB3_RageLevel");
	m_iArUpSPR = gHUD.GetSpriteIndex("zb3_arup");
	m_iHpUpSPR = gHUD.GetSpriteIndex("zb3_hpup");
	m_iFastRespawnSPR = gHUD.GetSpriteIndex("zb3_fastrespawn");

	for(int i = 0; i < 3; ++i)
		for (int j = 0; j < 8; ++j)
		{
			char szBuffer[32];
			sprintf(szBuffer, "ZB3_RageEffect%d0%d", i, j);
			m_iRageIndex[i][j] = gHUD.GetSpriteIndex(szBuffer);
		}

	m_flRageTimer = 0;
	return 1;
}

int CHudZB3Rage::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	if (!m_bEnabled && gHUD.m_iModRunning == MOD_ZBZ)
		return 0;

	int idx = IS_FIRSTPERSON_SPEC ? g_iUser2 : gEngfuncs.GetLocalPlayer()->index;
	if (!g_PlayerExtraInfo[idx].zombie)
		return 0;

	int iX = ScreenWidth / 2 - gEngfuncs.pfnSPR_Width(gHUD.GetSprite(m_iRageBG), 0) / 2;
	int iY = ScreenHeight * 0.97 - gEngfuncs.pfnSPR_Height(gHUD.GetSprite(m_iRageBG), 0) / 2;

	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iRageBG), 255, 255, 255);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iRageBG));

	// effect
	if (m_flRageTimer < time)
	{
		m_flRageTimer = time + 0.1;
		m_iRageFrame = (m_iRageFrame + 1) & 7;
	}
	assert(m_iRageFrame >= 0 && m_iRageFrame < 8);
	int iRageIndex = m_iRageIndex[m_iZombieLevel][m_iRageFrame];
	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(iRageIndex), 255, 255, 255);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY - 3, &gHUD.GetSpriteRect(iRageIndex));

	//Zombie Item
	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iArUpSPR), 255, 193, 147);
	gEngfuncs.pfnSPR_DrawAdditive(0, 4, gHUD.m_hudstyle->value == 2 ? gHUD.m_iMapHeight + 73 : ScreenHeight / 2 - 164, &gHUD.GetSpriteRect(m_iArUpSPR));

	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iHpUpSPR), 255, 193, 147);
	gEngfuncs.pfnSPR_DrawAdditive(0, 4, gHUD.m_hudstyle->value == 2 ? gHUD.m_iMapHeight + 73 + 46 : ScreenHeight / 2 - 120, &gHUD.GetSpriteRect(m_iHpUpSPR));

	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iFastRespawnSPR), 255, 193, 147);
	gEngfuncs.pfnSPR_DrawAdditive(0, 4, gHUD.m_hudstyle->value == 2 ? gHUD.m_iMapHeight + 73 + 90 : ScreenHeight / 2 + 75, &gHUD.GetSpriteRect(m_iFastRespawnSPR));


	// background bar
	iX += 76;
	iY += 50;
	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iRageLevel), 50, 50, 50);
	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iRageLevel));

	// foreground bar
	if (m_iPercent != 0)
	{
		static const wrect_t &TempRect = gHUD.GetSpriteRect(m_iRageLevel);
		wrect_t ModifyRect = TempRect;
		int iLen = ModifyRect.right - ModifyRect.left;
		iLen = iLen * (m_iPercent / 100.0f) + 1;
		ModifyRect.right = iLen + ModifyRect.left;
		gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iRageLevel), 255, 255, 255);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &ModifyRect);
	}

	return 1;
}

}