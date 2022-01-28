/*
z4_tips.cpp - CSMoE Client HUD : Zombie 4 Tips
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
#include "z4_tips.h"

const char* g_szTipsText[] = {
	// RoundStart5 0 - 2
	"感染体出现，注意四周警戒！",
	"有感染反应，危险！危险！",
	"已有人被僵尸感染，进入战备状态！",

	// RoundStart22 3 - 5
	"被精准打击技能击败的僵尸将无法复活(按数字键6)",
	"僵尸出现！受到感染就会变异为僵尸！！",
	"使用侧踢可击退僵尸！(按数字键 5)",

	// NightBegin Human 6 - 8
	"黑夜即将到来，提高警惕，僵尸即将反扑！",
	"黑夜来临！互相掩护，警戒四周！",
	"黑夜的到来，使得僵尸成就了不死之躯！",

	// NightEnd Human 9 - 11
	"弹药补给完毕！放开手脚大干一场吧！",
	"阳光使得僵尸变得脆弱，别错过这个好机会！",
	"黑夜已经过去，弹药和手雷已经补给完毕！",

	// FirstZombie 12 - 14
	"我..是..感染体..人类..攻击！！！",
	"我们..僵尸..是有特殊能力的！(双击W键)",
	"人类..敌人..僵尸..永生！",

	// NightBegin Zombie 15 - 17
	"黑夜..我们..将..变得..更..强！",
	"啊啊..黑夜中..充满了..力量！",
	"黑暗的夜晚中，僵尸的能量恢复的更快！",

	// NightEnd Zombie 18 - 20
	"啊..太阳..出现了..力量..正在..流失...",
	"不好..太阳..即将..出现..",
	"啊啊..快要..天亮了..不能放弃.."
};

CHudZ4Tips::CHudZ4Tips(void)
{
	InitHUDData();
}

void CHudZ4Tips::InitHUDData()
{
	memset(m_szText, 0, sizeof(m_szText));

	m_flDisplayEnd = 0.0f;
	m_iCount = -1;
}

int CHudZ4Tips::VidInit(void)
{
	if (!m_pBG[0])
		m_pBG[0] = R_LoadTextureUnique("resource/zombi/z4_hmmsgbg");

	if (!m_pBG[1])
		m_pBG[1] = R_LoadTextureUnique("resource/zombi/z4_zbmsgbg");

	return 1;
}

int CHudZ4Tips::Draw(float flTime)
{
	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return 0;

	int index = gEngfuncs.GetLocalPlayer()->index;
	if(g_PlayerExtraInfo[index].teamnumber != 1 && g_PlayerExtraInfo[index].teamnumber != 2)
		return 0;

	int time = flTime - gHUD.m_Timer.m_fStartTime + 1;

	int iX, iY;
	iX = ScreenWidth / 2 - m_pBG[0]->w() / 2;
	iY = ScreenHeight * 0.18;

	bool isZombie = g_PlayerExtraInfo[index].zombie;
	if (isZombie)
	{
		/*if (time > m_iNightEnd - 10 && time <= m_iNightStart + 10)
		{
			g_CHudZB4UI.m_Retina[2].SetCanDraw(true);
			g_CHudZB4UI.m_Retina[2].SetEndTime(flTime + 9999.0);
		}

		if (time >= m_iNightStart && time <= m_iNightStart + 10)
			g_CHudZB4UI.m_Retina[2].SetColor(255, 255, 255, float(m_iNightStart + 10.0 - (m_flTimeEnd - flTime + 1.0)) / 10.0 * 255);

		if (time >= m_iNightEnd - 10 && time <= m_iNightEnd)
			g_CHudZB4UI.m_Retina[2].SetColor(255, 255, 255, float((m_flTimeEnd - flTime + 1.0) - m_iNightEnd) / 10.0 * 255 + 255);*/
	}
	else
	{
		/*g_CHudZB4UI.m_Retina[2].SetCanDraw(false);*/
	}

	if (m_iCount != time)
	{
		if (time == 1)
		{
			gHUD.m_ZB4.SetStatus(0, Z4_POWERUP, Z4_ICON_FLASH);
			m_flDisplayEnd = 0.0f;
		}

		if (isZombie)
			gHUD.m_ZB4.SetStatus(0, Z4_POWERUP, Z4_ICON_HIDE);

		if (time == m_iNightStart)
		{
			if (!isZombie)
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTDOWN, Z4_ICON_FLASH);

				m_flDisplayEnd = flTime + 3.0f;
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(6, 8)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/human_message.wav");
			}
			else
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH);
				gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH);

				m_flDisplayEnd = flTime + 3.0f;
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(15, 17)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/zombie_message.wav");
			}
		}

		if (time == m_iNightEnd - 1)
		{
			if (!isZombie)
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTDOWN, Z4_ICON_FLASH2);
			}
			else
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH2);
				gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH2);
			}
		}

		if (time == m_iNightEnd - 1)
		{
			if (!isZombie)
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTDOWN, Z4_ICON_FLASH2);

				m_flDisplayEnd = flTime + 3.0f;
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(9, 11)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/human_message.wav");
			}
			else
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH2);
				gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH2);
				gHUD.m_ZB4.SetStatus(7, Z4_SPEEDDOWN, Z4_ICON_SHOW);

				m_flDisplayEnd = flTime + 3.0f;
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(18, 20)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/zombie_message.wav");
			}
		}


		if (time == m_iNightEnd - 4 && isZombie)
		{
			gHUD.m_ZB4.SetStatus(7, Z4_SPEEDDOWN, Z4_ICON_FLASH2);
		}

		// RoundStart Tips
		if (time == 5)
		{
			m_flDisplayEnd = flTime + 3.0f;
			strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(0, 2)], sizeof(m_szText));
			gEngfuncs.pfnClientCmd("spk sound/events/human_message.wav");
		}

		// FirstZombie Tips
		if (time == 21)
		{
			m_flDisplayEnd = flTime + 3.0f;
			if (!isZombie)
			{
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(3, 5)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/human_message.wav");
			}
			else
			{
				strncpy(m_szText, g_szTipsText[gEngfuncs.pfnRandomLong(12, 14)], sizeof(m_szText));
				gEngfuncs.pfnClientCmd("spk sound/events/zombie_message.wav");
			}
		}

		m_iCount = time;
	}

	if (m_flDisplayEnd < flTime)
		return 0;

	int a = 255;
	if (m_flDisplayEnd - flTime > 2.6)
		a = abs((int)((m_flDisplayEnd - flTime - 3.0f) * 255 / 0.4));

	if (a > 255) a = 255;

	if (m_flDisplayEnd - flTime < 0.5)
		a = (m_flDisplayEnd - flTime) * 255 * 2;

	if (isZombie) m_pBG[1]->Draw2DQuadScaled(iX, iY, iX + m_pBG[1]->w(), iY + m_pBG[1]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, a);
	else m_pBG[0]->Draw2DQuadScaled(iX, iY, iX + m_pBG[0]->w(), iY + m_pBG[0]->h(), 0.0f, 0.0f, 1.0f, 1.0f, 255, 255, 255, a);

	iX += 127;
	iY += 60;

	if (isZombie)
	{
		DrawUtils::DrawHudString(iX, iY, ScreenWidth, m_szText, 180, 80, 75);
	}
	else
	{
		DrawUtils::DrawHudString(iX, iY, ScreenWidth, m_szText, 146, 158, 185);
	}
	return 1;
}