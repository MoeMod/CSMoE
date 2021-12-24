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

class CHudZ4Energy : public IBaseHudSub
{
public:
	CHudZ4Energy();
	int VidInit(void) override;
	int Draw(float time) override;

	void SetPercent(int iPercent) { m_iPercent = iPercent; }

protected:
	int m_iPercent;
};