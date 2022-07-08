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

namespace cl {

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
	m_iCount = -1;
}

int CHudZ4Tips::VidInit(void)
{
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
		}

		if (isZombie)
			gHUD.m_ZB4.SetStatus(0, Z4_POWERUP, Z4_ICON_HIDE);

		if (time == m_iNightStart)
		{
			if (!isZombie)
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTDOWN, Z4_ICON_FLASH);

                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(6, 8)], 3, 0);
			}
			else
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH);
				gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH);

                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(15, 17)], 3, 1);
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

                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(9, 11)], 3, 0);
			}
			else
			{
				gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH2);
				gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH2);
				gHUD.m_ZB4.SetStatus(7, Z4_SPEEDDOWN, Z4_ICON_SHOW);

                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(18, 20)], 3, 1);
			}
		}


		if (time == m_iNightEnd - 4 && isZombie)
		{
			gHUD.m_ZB4.SetStatus(7, Z4_SPEEDDOWN, Z4_ICON_FLASH2);
		}

		// RoundStart Tips
		if (time == 5)
		{
            gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(0, 2)], 3, 0);
		}

		// FirstZombie Tips
		if (time == 21)
		{
			if (!isZombie)
			{
                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(3, 5)], 3, 0);
			}
			else
			{
                gHUD.m_CenterTips.Show(g_szTipsText[gEngfuncs.pfnRandomLong(12, 14)], 3, 0);
			}
		}

		m_iCount = time;
	}
	return 1;
}

}