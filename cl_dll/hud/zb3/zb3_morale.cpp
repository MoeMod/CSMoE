/*
zb3_morale.cpp - CSMoE Client HUD : Zombie Hero Human Morale System
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

#include "zb3.h"
#include "zb3_morale.h"

#include "gamemode/zb3/zb3_const.h"

CHudZB3Morale::CHudZB3Morale(void)
{
	
}

int CHudZB3Morale::VidInit(void)
{
	m_iMoraleIconSPR = gHUD.GetSpriteIndex("ZB3_MoraleIcon");
	m_iMoraleLevelSPR = gHUD.GetSpriteIndex("ZB3_MoraleLevel");
	m_iMoraleEffectSPR = gHUD.GetSpriteIndex("ZB3_MoraleEffect");
	return 1;
}

int CHudZB3Morale::Draw(float time)
{
	int iX = ScreenWidth / 2 - 130;
	int iY = ScreenHeight - 100;
	
	int iMorale = m_iMoraleLevel;
	int iMaxMorale = 10;
	if (m_iMoraleType == ZB3_MORALE_STRENGTHEN)
	{
		iMorale += 3;
		iMaxMorale += 3;
	}
	
	int r = 0, g = 0, b = 0, a = 0;

	if (iMorale < 4)
		std::tie(r, g, b, a) = std::make_tuple( 0, 177, 0, 255 );
	else if (iMorale < 6)
		std::tie(r, g, b, a) = std::make_tuple(137, 191, 20, 255);
	else if (iMorale < 8)
		std::tie(r, g, b, a) = std::make_tuple(250, 229, 0, 255);
	else if (iMorale < 10)
		std::tie(r, g, b, a) = std::make_tuple(243, 127, 1, 255);
	else if (iMorale = 10)
		std::tie(r, g, b, a) = std::make_tuple(255, 3, 0, 255);
	else if (iMorale > 10)
		std::tie(r, g, b, a) = std::make_tuple(127, 40, 208, 255);

	gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iMoraleIconSPR), r, g, b);

	gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &gHUD.GetSpriteRect(m_iMoraleIconSPR));
	if (iMorale >= 13)
	{
		gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iMoraleEffectSPR), 255, 255, 255);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX - 17, iY - 16, &gHUD.GetSpriteRect(m_iMoraleEffectSPR));
	}

	iX += 53;
	iY += 13;
	static const wrect_t &TempRect = gHUD.GetSpriteRect(m_iMoraleLevelSPR);
	wrect_t ModifyRect = TempRect;

	int iLen = TempRect.right - ModifyRect.left;
	ModifyRect.right = iLen + ModifyRect.left;
	// background
	if(iMorale < iMaxMorale)
	{
		float flMorale = iMaxMorale / 13.0f;
		iLen = TempRect.right - ModifyRect.left;
		iLen = iLen * flMorale;
		ModifyRect.right = iLen + ModifyRect.left;
		gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iMoraleLevelSPR), 50, 50, 50);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &ModifyRect); 
	}

	// foreground
	if (iMorale != 0)
	{
		float flMorale = iMorale / 13.0f;
		iLen = TempRect.right - ModifyRect.left;
		iLen = iLen * flMorale;
		ModifyRect.right = iLen + ModifyRect.left;
		gEngfuncs.pfnSPR_Set(gHUD.GetSprite(m_iMoraleLevelSPR), 255, 255, 255);
		gEngfuncs.pfnSPR_DrawAdditive(0, iX, iY, &ModifyRect);
	}

	// text
	char szBuffer[64];
	sprintf(szBuffer, "ATT : %d %%", 100 + iMorale * 10);
	DrawUtils::DrawHudString(ScreenWidth / 2 - 77, ScreenHeight - 113, ScreenWidth, szBuffer, r, g, b);

	return 1;
}

void CHudZB3Morale::UpdateLevel(ZB3HumanMoraleType_e type, int level)
{
	if (level > m_iMoraleLevel)
	{
		// TODO : Level Up Tip
	}

	std::tie(m_iMoraleType, m_iMoraleLevel) = std::make_pair(type, level);
}
