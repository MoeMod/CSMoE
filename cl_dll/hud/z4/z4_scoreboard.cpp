/*
z4_scoreboard.cpp - CSMoE Client HUD : Zombie 4 Scoreboard
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
#include "draw_util.h"

#include "z4.h"
#include "z4_scoreboard.h"

namespace cl {

CHudZ4Scoreboard::CHudZ4Scoreboard(void)
{
	BuildNumberRC(m_rcNumber, 14, 32);
	InitHUDData();
}

void CHudZ4Scoreboard::InitHUDData()
{
	m_iTeam = 1;
	m_iDamage = 0;
	m_iLastScore = 0;
}

void CHudZ4Scoreboard::BuildNumberRC(wrect_t* rgrc, int w, int h)
{
	int nw = 0;

	for (int i = 0; i < 10; i++)
	{
		rgrc[i].left = nw;
		rgrc[i].top = 0;
		rgrc[i].right = rgrc[i].left + w;
		rgrc[i].bottom = h;

		nw += w;
	}
}

int CHudZ4Scoreboard::VidInit(void)
{
	if (!m_iNumber)
		m_iNumber = R_LoadTextureShared("resource/zombi/z4number");

	if (!m_iScroreBorad)
		m_iScroreBorad = R_LoadTextureUnique("resource/zombi/z4_scoreboard");

	if (!m_iDamageBorad)
		m_iDamageBorad = R_LoadTextureUnique("resource/zombi/z4_scorebg");

	if (!m_iTeamIcon[0])
		m_iTeamIcon[0] = R_LoadTextureUnique("resource/zombi/z4_scorehuman");

	if (!m_iTeamIcon[1])
		m_iTeamIcon[1] = R_LoadTextureUnique("resource/zombi/z4_scorezombie");

	g_iSelectionHeight = gHUD.GetSpriteRect(gHUD.m_Ammo.m_HUD_selection).bottom - gHUD.GetSpriteRect(gHUD.m_Ammo.m_HUD_selection).top;
	return 1;
}

int CHudZ4Scoreboard::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	DrawScoreBoard(time);
	DrawScore(time);
	return 1;
}

void CHudZ4Scoreboard::DrawScoreBoard(float time)
{
	int iW, iH, iX, iY;
	iW = m_iScroreBorad->w();
	iH = m_iScroreBorad->h();
	iX = ScreenWidth / 2 - iW / 2;
	iY = 0;

	m_iScroreBorad->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH);

	iX = ScreenWidth / 2 - 14;
	iY = 27;
	DrawNumbers(13, iX, iY, false);

	iX = ScreenWidth / 2 - 136;
	iY = 7;
	DrawNumbers(gHUD.m_Scoreboard.m_iTeamScore_T, iX, iY, true, 0.9, 245, 245, 245);

	iX = ScreenWidth / 2 + 67;
	iY = 7;
	DrawNumbers(gHUD.m_Scoreboard.m_iTeamScore_CT, iX, iY, false, 0.9, 245, 245, 245);

	int aliveT = gHUD.m_Scoreboard.m_iTeamAlive_T;
	int aliveCT = gHUD.m_Scoreboard.m_iTeamAlive_CT;

	iX = ScreenWidth / 2 - 115;
	iY = 34;
	DrawNumbers(aliveT, iX, iY, true, 0.65, 200, 200, 200);

	iX = ScreenWidth / 2 + 63;
	iY = 34;
	DrawNumbers(aliveCT, iX, iY, false, 0.65, 200, 200, 200);
}

void CHudZ4Scoreboard::DrawScore(float time)
{
	if (g_iUser1)
		return;

	int idx = gEngfuncs.GetLocalPlayer()->index;

	if (g_PlayerExtraInfo[idx].dead == true)
		return;

	/*if (g_szCurWeapon2 && g_bAlive)
	{
		g_Font.SetColor(200, 144, 72, 255);
		g_Font.SetWidth(18);
		g_Font.DrawString(g_szCurWeapon2, ScreenWidth - g_Font.GetLen(g_szCurWeapon2) - 18, ScreenHeight - 50, 1000, 100);
	}*/
	int iOffsetY = gHUD.m_hudstyle->value == 1 ? g_iSelectionHeight * 7 : g_iSelectionHeight;
	int iW, iH, iX, iY;
	iW = m_iDamageBorad->w();
	iH = m_iDamageBorad->h();
	iX = ScreenWidth - iW - 7;
	iY = ScreenHeight - iH - 80 - iOffsetY;

	m_iDamageBorad->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH);

	int team = m_iTeam - 1;

	iW = m_iTeamIcon[team]->w();
	iH = m_iTeamIcon[team]->h();
	iX = ScreenWidth - iW - 122;
	iY = ScreenHeight - iH - 88 - iOffsetY;

	m_iTeamIcon[team]->Draw2DQuadScaled(iX, iY, iX + iW, iY + iH);

	iX = ScreenWidth - iW - 56;
	iY = ScreenHeight - iH - 80 - iOffsetY;

	int iScore = g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].frags - m_iLastScore;
	if (iScore < 0)
		iScore = 0;

	DrawNumbers(iScore, iX, iY, true, 0.95, 200, 144, 72);

	iX = ScreenWidth - iW - 44;
	iY = ScreenHeight - iH - 56 - iOffsetY;

	if (m_iDamage > 99999)
		m_iDamage = 99999;

	DrawNumbers(m_iDamage, iX, iY, true, 0.8, 200, 144, 72);
}

void CHudZ4Scoreboard::DrawNumber(int n, int x, int y, float scale, int r, int g, int b)
{
	float w = m_iNumber->w() * scale;
	float h = m_iNumber->h() * scale;
	int left = m_rcNumber[n].left * scale;
	int right = m_rcNumber[n].right * scale;
	int bottom = m_rcNumber[n].bottom * scale;
	int top = m_rcNumber[n].top * scale;
	int x2 = x + (right - left);
	int y2 = y + (bottom - top);
	m_iNumber->Draw2DQuadScaled(x, y, x2, y2, left / w, top / h, right / w, bottom / h, 225, 225, 225);
}

void CHudZ4Scoreboard::DrawNumbers(int n, int x, int y, int from_right, float scale, int r, int g, int b)
{
	int width = 14;
	width *= scale;

	int k;

	if (n >= 100000)
	{
		k = n / 100000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10000)
	{
		k = (n % 100000) / 10000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 1000)
	{
		k = (n % 10000) / 1000;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 100)
	{
		k = (n % 1000) / 100;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10)
	{
		k = (n % 100) / 10;
		DrawNumber(k, x, y, scale, r, g, b);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	k = n % 10;
	DrawNumber(k, x, y, scale, r, g, b);
}

}