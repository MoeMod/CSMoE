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
	void InitHUDData(void) override;

public:
	void BuildNumberRC(wrect_t* rgrc, int w, int h);
	void SetPower(int power, float flashtime);
	void DrawNumber(int n, int x, int y, float size);
	void DrawNumbers(int n, int x, int y, int from_right, float size);
	void SetZbSkill(int text, int usage)
	{
		m_iText = text;
		m_iSkillUsage = usage;
	}

private:
	UniqueTexture m_iNumber;
	wrect_t m_rcNumber[10];

	UniqueTexture m_iPowerBackground;
	UniqueTexture m_iPowerLine;
	UniqueTexture m_iPowerLine2;
	UniqueTexture m_iPowerLeft2;
	SharedTexture m_iPowerPoint;
	UniqueTexture m_iPowerRightClick[2];
	UniqueTexture m_iKey[2];

	int m_iPower[2];
	int m_iPower2[2];
	int m_iPowerServer;

	int m_iUseTimesOld;
	int m_iSkillUsage;
	int m_iText;

	float m_flPowerStartRefreshTime[2];
	float m_flPowerEndRefreshTime[2];

	float m_flFlashTime;
	int m_iAlpha;

	char m_szDrawText[64];
	float m_flTextFlashStart;
	int m_iTextFlash;
};