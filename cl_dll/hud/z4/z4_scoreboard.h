/*
zb3_scoreboard.h - CSMoE Client HUD : Zombie 4 Scoreboard
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

namespace cl {

class CHudZ4Scoreboard : public IBaseHudSub
{
public:
	CHudZ4Scoreboard();
	int VidInit(void) override;
	int Draw(float time) override;
	void InitHUDData(void) override;

public:
	void BuildNumberRC(wrect_t* rgrc, int w, int h);
	void DrawScoreBoard(float time);
	void DrawScore(float time);
	void DrawNumber(int n, int x, int y, float scale, int r, int g, int b);
	void DrawNumbers(int n, int x, int y, int from_right, float scale = 1.0f, int r = 255, int g = 255, int b = 255);

	void SetTeam(int team) { m_iTeam = team; }
	void SetDamage(int damage) { m_iDamage = damage; }
	void SetLastScore() { m_iLastScore = g_PlayerExtraInfo[gEngfuncs.GetLocalPlayer()->index].frags; }

private:
	SharedTexture m_iNumber;
	wrect_t m_rcNumber[10];

	UniqueTexture m_iScroreBorad;
	UniqueTexture m_iDamageBorad;

	UniqueTexture m_iTeamIcon[2];

	int m_iTeam;
	int m_iDamage;
	int m_iLastScore;
	int g_iSelectionHeight;
};

}