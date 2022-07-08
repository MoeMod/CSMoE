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

#pragma once

#include "hud_sub.h"

enum ZB3HumanMoraleType_e : byte;
namespace cl {
class CHudZB3Morale : public IBaseHudSub
{
public:
	CHudZB3Morale();
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData() override { m_bEnabled = false; }

	void UpdateLevel(ZB3HumanMoraleType_e type, int level);
	void SetEnabled(bool val) { m_bEnabled = val; }

protected:
	ZB3HumanMoraleType_e m_iMoraleType;
	int m_iMoraleLevel;

private:
	int m_iMoraleIconSPR;
	int m_iMoraleLevelSPR;
	int m_iMoraleEffectSPR;
	int m_iHighMoraleSPR;
	int m_iJumphigherSPR;
	int m_iAmmoupSPR;
	bool m_bEnabled;
};
}