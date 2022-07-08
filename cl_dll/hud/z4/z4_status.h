/*
z4_status.h - CSMoE Client HUD : Zombie 4 Status
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
#include "gamemode/z4/z4_const.h"

namespace cl {

class CHudZ4Status : public IBaseHudSub
{
public:
	CHudZ4Status();
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData(void) override;

public:
	void SetPower(int value) { m_iPower = value; }
	void AddIcon(int slot, Z4Status id, Z4StatusIconDraw status);
	void HideAll()
	{
		for (size_t i = 0; i < Z4_MAX_STATUS; i++)
		{
			m_iStatus[i] = Z4_ICON_HIDE;
		}
	}

private:
	UniqueTexture m_pIcon[Z4_MAX_STATUS];

	Z4StatusIconDraw m_iStatus[Z4_MAX_STATUS];
	Z4Status m_iId[Z4_MAX_STATUS];
	int m_iAlpha[Z4_MAX_STATUS];
	float m_flFlashTime[Z4_MAX_STATUS];
	int m_iFlash[Z4_MAX_STATUS];

	int m_iPower;
	//int m_iTeam;
};

}