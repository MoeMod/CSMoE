/*
z4_energy.cpp - CSMoE Client HUD : Zombie 4 Energy System
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
#include "z4_energy.h"

namespace cl {

constexpr int MAX_ENERGY = 120;

CHudZ4Energy::CHudZ4Energy(void)
{
	BuildNumberRC(m_rcNumber, 14, 32);
	InitHUDData();
}

void CHudZ4Energy::InitHUDData()
{
	m_iSkillUsage = 5;
	m_iZClass = Z4_CLASS_NORMAL;
	m_iPower[0] = 0;
	m_iPower[1] = 0;
	m_iPower2[0] = 0;
	m_iPower2[1] = 0;
	m_iPowerServer = 0;
	m_iUseTimesOld = -1;

	m_flPowerStartRefreshTime[0] = 0.0f;
	m_flPowerEndRefreshTime[0] = 0.0f;
	m_flPowerStartRefreshTime[1] = 0.0f;
	m_flPowerEndRefreshTime[1] = 0.0f;

	m_flFlashTime = 0.0f;

	memset(m_szDrawText, 0, sizeof(m_szDrawText));
}

void CHudZ4Energy::BuildNumberRC(wrect_t* rgrc, int w, int h)
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

int CHudZ4Energy::VidInit(void)
{
	if (!m_iNumber)
		m_iNumber = R_LoadTextureUnique("resource/zombi/z4number");

	if (!m_iPowerBackground)
		m_iPowerBackground = R_LoadTextureUnique("resource/zombi/adrenalinebg");
	if (!m_iPowerLine)
		m_iPowerLine = R_LoadTextureUnique("resource/zombi/adrenalinnegauge");
	if (!m_iPowerLine2)
		m_iPowerLine2 = R_LoadTextureUnique("resource/zombi/adrenalinnegauge2");
	if (!m_iPowerLeft2)
		m_iPowerLeft2 = R_LoadTextureUnique("resource/zombi/adrenalinneleft");
	if (!m_iPowerPoint)
		m_iPowerPoint = R_LoadTextureShared("resource/zombi/adrenalinnepoint");
	if (!m_iKey[0])
		m_iKey[0] = R_LoadTextureUnique("resource/zombi/z4_6key");
	if (!m_iKey[1])
		m_iKey[1] = R_LoadTextureUnique("resource/zombi/z4_rightclick");

	return 1;
}

void CHudZ4Energy::SetPower(int power, float flashtime)
{
	if (power < m_iPowerServer)
	{
		if (m_iPowerServer < m_iPower[0] && m_flPowerEndRefreshTime[0] > gHUD.m_flTime)
		{
			m_iPower2[0] = power;
			m_flPowerEndRefreshTime[0] = gHUD.m_flTime + flashtime;
		}
		else
		{
			m_iPower[0] = m_iPowerServer;
			m_iPower2[0] = power;
			m_flPowerEndRefreshTime[0] = gHUD.m_flTime + flashtime;
			m_flPowerStartRefreshTime[0] = gHUD.m_flTime;
			m_iAlpha = 0;
		}
	}

	if (power > m_iPowerServer)
	{
		if (m_iPowerServer > m_iPower[1] && m_flPowerEndRefreshTime[1] > gHUD.m_flTime)
		{
			m_iPower2[1] = power;
			m_flPowerEndRefreshTime[1] = gHUD.m_flTime + flashtime;
		}
		else
		{
			m_iPower[1] = m_iPowerServer;
			m_iPower2[1] = power;
			m_flPowerEndRefreshTime[1] = gHUD.m_flTime + flashtime;
			m_flPowerStartRefreshTime[1] = gHUD.m_flTime;
		}
	}

	m_iPowerServer = power;
}

int CHudZ4Energy::Draw(float time)
{
	if (!gHUD.m_pCvarDraw->value)
		return 0;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	if (g_iUser1)
		return 0;

	if (!(gHUD.m_iWeaponBits & (1 << (WEAPON_SUIT))))
		return 0;

	int idx = gEngfuncs.GetLocalPlayer()->index;
	int iX, iY;
	iX = ScreenWidth / 2 - m_iPowerBackground->w() / 2;
	iY = ScreenHeight - m_iPowerBackground->h() - 60;

	m_iPowerBackground->Draw2DQuadScaled(iX, iY, iX + m_iPowerBackground->w(), iY + m_iPowerBackground->h());

	float flFlashTime = time - m_flFlashTime;

	if (2 * flFlashTime >= M_PI)
		m_flFlashTime = time;

	m_iAlpha = 255.0f * sin(2 * flFlashTime);

	if (m_iAlpha > 255)
		m_iAlpha = 255;

	if (m_iAlpha < 0)
		m_iAlpha = 0;

	m_iPowerLeft2->Draw2DQuadScaled(iX, iY, iX + m_iPowerLeft2->w(), iY + m_iPowerLeft2->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, m_iAlpha);
	for (int i = 0; i <= 3; i++)
	{
		m_iPowerPoint->Draw2DQuadScaled(iX + 141 + i * 42, iY + 44, iX + 141 + i * 42 + m_iPowerPoint->w(), iY + 44 + m_iPowerPoint->h());
	}

	iX += 108;
	iY += 46;

	int iPower = m_iPowerServer;
	int iProgress = 0;

	if (m_iPowerServer < m_iPower[0] && m_flPowerEndRefreshTime[0] > time)
	{
		iPower = m_iPower2[0] + (m_iPower[0] - m_iPower2[0]) * (m_flPowerEndRefreshTime[0] - time) / (m_flPowerEndRefreshTime[0] - m_flPowerStartRefreshTime[0]);

		if (iPower < 0)
			iPower = 0;

		float flPowerLine2Width = m_iPowerLine2->w();
		int left = 0;
		if (m_iPowerServer > m_iPower[1] && m_flPowerEndRefreshTime[1] > time)
			left = m_iPower[1] / (float)MAX_ENERGY * flPowerLine2Width;
		else
			left = m_iPowerServer / (float)MAX_ENERGY * flPowerLine2Width;

		iProgress = iPower / (float)MAX_ENERGY * flPowerLine2Width;

		if (iProgress >= m_iPower[1] / (float)MAX_ENERGY * flPowerLine2Width)
		{
			m_iPowerLine2->Draw2DQuadScaled(iX, iY, iX + iProgress, iY + m_iPowerLine2->h(), 0.0f, 0.0f, iProgress / flPowerLine2Width, 1.0f);
		}
	}

	float flPowerLineWidth = m_iPowerLine->w();
	float flPowerLineHeight = m_iPowerLine->h();
	if (m_iPowerServer > m_iPower[1] && m_flPowerEndRefreshTime[1] > time)
	{
		iPower = m_iPower2[1] - (m_iPower2[1] - m_iPower[1]) * (m_flPowerEndRefreshTime[1] - time) / (m_flPowerEndRefreshTime[1] - m_flPowerStartRefreshTime[1]);

		if (iPower > MAX_ENERGY)
			iPower = MAX_ENERGY;

		iProgress = iPower / (float)MAX_ENERGY * flPowerLineWidth;
		m_iPowerLine->Draw2DQuadScaled(iX, iY, iX + iProgress, iY + flPowerLineHeight, 0.0f, 0.0f, iProgress / flPowerLineWidth, 1.0f, 225, 225, 225);
	}
	else
	{
		m_iPower[1] = m_iPowerServer;

		iProgress = m_iPowerServer / (float)MAX_ENERGY * flPowerLineWidth;
		m_iPowerLine->Draw2DQuadScaled(iX, iY, iX + iProgress, iY + flPowerLineHeight, 0.0f, 0.0f, iProgress / flPowerLineWidth, 1.0f, 225, 225, 225);
	}

	iProgress = m_iPowerServer / (float)MAX_ENERGY * flPowerLineWidth;
	m_iPowerLine->Draw2DQuadScaled(iX, iY, iX + iProgress, iY + flPowerLineHeight, 0.0f, 0.0f, iProgress / flPowerLineWidth, 1.0f, 225, 225, 225, 200);

	iX = ScreenWidth / 2 - m_iPowerBackground->w() / 2;
	iY = ScreenHeight - m_iPowerBackground->h() - 60;

	iX += 52;
	iY += 43;

	if (iPower < 10)
		iX += 6;

	if (iPower == MAX_ENERGY)
		iX -= 10;

	DrawNumbers(iPower, iX, iY, false, 1.1);

	iX = ScreenWidth / 2 - m_iPowerBackground->w() / 2 + 95;
	iY = ScreenHeight - m_iPowerBackground->h() - 60 - 15;

	if (!g_PlayerExtraInfo[idx].zombie && m_iPowerServer >= 30)
	{
		if (!m_iTextFlash)
		{
			m_iTextFlash = 1;
			m_flTextFlashStart = time;
		}

		float flFlashTime = time - m_flTextFlashStart;

		int iAlpha = abs((int)((sin(flFlashTime * M_PI / 1.5)) * 255 * 0.9 + 25));
		iAlpha = iAlpha > 255 ? 255 : iAlpha;
		iAlpha = iAlpha < 0 ? 0 : iAlpha;

		if (flFlashTime >= 3.75)
			iAlpha = 255;

		m_iKey[0]->Draw2DQuadScaled(iX, iY, iX + m_iKey[0]->w(), iY + m_iKey[0]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, iAlpha);
		iX += m_iKey[0]->w() + 5;

		DrawUtils::DrawHudString(iX, iY + 15, ScreenWidth, "可使用[精准打击]", 251, 201, 96);
	}
	else if (g_PlayerExtraInfo[idx].zombie && m_iPowerServer >= m_iSkillUsage)
	{
		m_iTextFlash = 0;
		m_flTextFlashStart = 0.0f;

		if (m_iUseTimesOld != int(m_iPowerServer / m_iSkillUsage))
		{
			m_iUseTimesOld = m_iPowerServer / m_iSkillUsage;

			/*switch (m_iText)
			{
			case 0: sprintf(szBuf[1], "残暴镰魔"); break;
			case 1: sprintf(szBuf[1], "暗夜魅影"); break;
			case 2: sprintf(szBuf[1], "重锤血手"); break;
			case 3: sprintf(szBuf[1], "幽暗领主"); break;
			case 4: sprintf(szBuf[1], "厄运之轮"); break;
			}*/

			const char *szSkillBuf[] = {
					"可使用[攀爬]",
					"可使用[飞跃]",
					"可使用[撼地]",
					"可使用[制空]",
					"可使用[飞跃]"
			};
			if(m_iZClass >= 0 && m_iZClass < std::extent_v<decltype(szSkillBuf)>)
				snprintf(m_szDrawText, 64, "%s(%d次)", szSkillBuf[m_iZClass], m_iUseTimesOld);
			else
				m_szDrawText[0] = '\0';
		}

		m_iKey[1]->Draw2DQuadScaled(iX, iY, iX + m_iKey[1]->w(), iY + m_iKey[1]->h());
		iX += m_iKey[1]->w() + 5;
		DrawUtils::DrawHudString(iX, iY + 15, ScreenWidth, m_szDrawText, 251, 201, 96);
	}

	return 1;
}

void CHudZ4Energy::DrawNumber(int n, int x, int y, float size)
{
	float w = m_iNumber->w();
	float h = m_iNumber->h();

	int x2 = x + (m_rcNumber[n].right - m_rcNumber[n].left);
	int y2 = y + (m_rcNumber[n].bottom - m_rcNumber[n].top);

	m_iNumber->Draw2DQuadScaled(x, y, x2, y2, m_rcNumber[n].left / w, m_rcNumber[n].top / h, m_rcNumber[n].right / w, m_rcNumber[n].bottom / h, 225, 225, 225);
}

void CHudZ4Energy::DrawNumbers(int n, int x, int y, int from_right, float size)
{
	int width = 14;
	width *= size;

	int k;

	if (n >= 100000)
	{
		k = n / 100000;
		DrawNumber(k, x, y, size);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10000)
	{
		k = (n % 100000) / 10000;
		DrawNumber(k, x, y, size);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 1000)
	{
		k = (n % 10000) / 1000;
		DrawNumber(k, x, y, size);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 100)
	{
		k = (n % 1000) / 100;
		DrawNumber(k, x, y, size);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	if (n >= 10)
	{
		k = (n % 100) / 10;
		DrawNumber(k, x, y, size);
		if (!from_right)
			x += width;
	}
	if (from_right)
		x += width;

	k = n % 10;
	DrawNumber(k, x, y, size);
}

}