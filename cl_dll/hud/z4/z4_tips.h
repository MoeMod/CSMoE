/*
z4_tips.h - CSMoE Client HUD : Zombie 4 Tips
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

#pragma once

#include "hud_sub.h"

namespace cl {

class CHudZ4Tips : public IBaseHudSub
{
public:
	CHudZ4Tips();
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData(void) override;

public:
	void SetNightTime(int start, int end)
	{
		m_iNightStart = start;
		m_iNightEnd = end;
	}

	bool IsNightTime()
	{
		int time = gHUD.m_flTime - gHUD.m_Timer.m_fStartTime + 1;
		return (time >= m_iNightStart && time <= m_iNightEnd);
	}

	float GetNightTime()
	{
		return m_iNightEnd - m_iNightStart;
	}

	/*void ZombieIconsCheck(float flTime)
	{
		int time = m_flTimeEnd - flTime + 1;

		if (time > m_iNightEnd && time <= m_iNightStart)
		{
			gHUD.m_ZB4.SetStatus(5, Z4_MIDNIGHTUP, Z4_ICON_FLASH);
			gHUD.m_ZB4.SetStatus(4, Z4_HPUP, Z4_ICON_FLASH);
		}
	}*/

private:
	int m_iCount;

	int m_iNightStart;
	int m_iNightEnd;
};

}