/*
z4_status.cpp - CSMoE Client HUD : Zombie 4 Status
Copyright (C) 2021 Moemod Hyakuya

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
#include "z4_status.h"

namespace cl {

#define X_Start 10

const char* g_szStatusIcon[Z4_MAX_STATUS] = {
	"z4_powerup",
	"z4_criticalshot",
	"z4_midnightup",
	"z4_midnightdown",
	"z4_speedup",
	"z4_speeddown",
	"z4_cannotcontrol",
	"z4_defenseup",
	"z4_defensedown",
	"z4_hpup",
	"z4_hpdown"
};

CHudZ4Status::CHudZ4Status(void)
{
	InitHUDData();
}

void CHudZ4Status::InitHUDData()
{
	memset(m_iId, 0, sizeof(m_iId));
	memset(m_iStatus, 0, sizeof(m_iStatus));
	memset(m_iAlpha, 0, sizeof(m_iAlpha));
	memset(m_flFlashTime, 0, sizeof(m_flFlashTime));
	memset(m_iFlash, 0, sizeof(m_iFlash));
}

void CHudZ4Status::AddIcon(int slot, Z4Status id, Z4StatusIconDraw status)
{
	if (m_iFlash[slot] > 0 && m_iStatus[slot] == Z4_ICON_FLASH)
		return;

	m_iId[slot] = id;
	m_iStatus[slot] = status;
	m_iAlpha[slot] = 0;
	m_flFlashTime[slot] = gHUD.m_flTime;
	m_iFlash[slot] = 0;

	if (m_iStatus[slot] == Z4_ICON_FLASH)
	{
		m_iFlash[slot] = 2;
		m_iAlpha[slot] = 0;
	}

	if (m_iStatus[slot] == Z4_ICON_FLASH2)
	{
		m_iFlash[slot] = 2;
		m_iAlpha[slot] = 255;
	}
}

int CHudZ4Status::VidInit(void)
{
	char szPath[256];
	for (size_t i = 0; i < Z4_MAX_STATUS; i++)
	{
		if (!m_pIcon[i])
		{
			snprintf(szPath, sizeof(szPath), "resource/zombi/%s", g_szStatusIcon[i]);
			m_pIcon[i] = R_LoadTextureUnique(szPath);
		}
	}

	return 1;
}

int CHudZ4Status::Draw(float time)
{
	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	int iX = X_Start, iY = ScreenHeight * 0.54f;
	int iIcons = 0;

	for (int i = 0; i < Z4_MAX_STATUS; i++)
	{
		if (m_iStatus[i] == Z4_ICON_FLASH)
		{
			float flFlashTime = time - m_flFlashTime[i];

			if (2.5 * flFlashTime >= M_PI)
			{
				m_flFlashTime[i] = time;
				m_iFlash[i] --;
			}

			m_iAlpha[i] = 255.0f * sin(2.5 * flFlashTime);

			if (m_iAlpha[i] > 255)
				m_iAlpha[i] = 255;

			if (m_iAlpha[i] < 0)
				m_iAlpha[i] = 0;

			if (m_iFlash[i] <= 1 && m_iAlpha[i] >= 250)
			{
				m_iAlpha[i] = 255;
				m_iStatus[i] = Z4_ICON_SHOW;
			}
		}

		if (m_iStatus[i] == Z4_ICON_FLASH2)
		{
			float flFlashTime = time - m_flFlashTime[i];

			if (5.5 * flFlashTime >= M_PI)
			{
				m_flFlashTime[i] = time;
				m_iFlash[i] --;
			}

			m_iAlpha[i] = 255.0f * abs((int)(sin(5.5 * flFlashTime + 0.5 * M_PI)));

			if (m_iAlpha[i] < 50)	m_iAlpha[i] = 0;

			m_iAlpha[i] *= 1.25;
			if (m_iAlpha[i] > 255) m_iAlpha[i] = 255;

			if (m_iFlash[i] <= 1 && m_iAlpha[i] <= 10)
			{
				m_iAlpha[i] = 255;
				m_iStatus[i] = Z4_ICON_HIDE;
			}
		}

		if (m_iStatus[i] != Z4_ICON_HIDE)
		{
			if (m_iStatus[i] == Z4_ICON_SHOW)
				m_iAlpha[i] = 255;

			m_pIcon[m_iId[i]]->Draw2DQuadScaled(iX, iY, iX + m_pIcon[m_iId[i]]->w(), iY + m_pIcon[m_iId[i]]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, m_iAlpha[i]);

			iIcons++;
			iX += m_pIcon[m_iId[i]]->w() + 5;
		}

		if ((i + 1) % 4 == 0)
		{
			iX = X_Start;
			iY += m_pIcon[m_iId[i]]->h() + 5;
		}
	}

	if (m_iStatus[0] != Z4_ICON_HIDE)
	{
		char szPower[8];
		snprintf(szPower, sizeof(szPower), "%d%%", m_iPower);
		/*g_Font.SetWidth(14);
		g_Font.SetColor(125, 160, 200, m_iAlpha[0]);
		g_Font.DrawString(ppower, X_Start + m_rcIcon[0].right - g_Font.GetLen(ppower) - 3, g_sScreenInfo.iHeight * 0.54 + m_rcIcon[0].bottom - 6, 100, 12);*/
		//DrawUtils::DrawHudString(X_Start + m_pIcon[0]->w() - DrawUtils::HudStringLen(power), ScreenHeight * 0.54f + m_pIcon[0]->h() - 20, 100, power, 125, 160, 200);
		DrawUtils::DrawHudStringReverse(X_Start + m_pIcon[0]->w() - 10, ScreenHeight * 0.54f + m_pIcon[0]->h() - 20, 100, szPower, 125, 160, 200);
	}

	return 1;
}

}