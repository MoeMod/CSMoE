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
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"
#include "eventscripts.h"

#include "zb3.h"
#include "zb3_rage.h"

#include "player/player_zombie.h"
#include "gamemode/zb3/zb3_const.h"

CHudZB3Rage::CHudZB3Rage(void)
{
	
}

int CHudZB3Rage::VidInit(void)
{
	m_iRageBG = gHUD.GetSpriteIndex("ZB3_RageBg");
	m_iRageLevel = gHUD.GetSpriteIndex("ZB3_RageLevel");

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