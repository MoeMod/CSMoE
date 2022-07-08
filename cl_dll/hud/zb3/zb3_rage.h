/*
zb3_rage.h - CSMoE Client HUD : Zombie Hero Zombie Rage System
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

#pragma once

#include "hud_sub.h"

enum ZombieLevel : int;

namespace cl {

class CHudZB3Rage : public IBaseHudSub
{
public:
	CHudZB3Rage();
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData() override { m_bEnabled = false; }

	void SetPercent(int iPercent) { m_iPercent = iPercent; }
	void SetZombieLevel(ZombieLevel zl) { m_iZombieLevel = zl; }
	void SetEnabled(bool val) { m_bEnabled = val; }

protected:
	ZombieLevel m_iZombieLevel;
	int m_iPercent;
	int m_iRageFrame;
	float m_flRageTimer;

private:
	int m_iRageBG;
	int m_iRageLevel;

	int m_iRageIndex[3][8];

	int m_iArUpSPR;
	int m_iHpUpSPR;
	int m_iFastRespawnSPR;
	bool m_bEnabled;
};

}