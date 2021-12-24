/*
zb3_rage.cpp - CSMoE Client HUD : Zombie Hero Zombie Rage System
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
#include "followicon.h"
#include "cl_util.h"
#include "draw_util.h"
#include "triangleapi.h"
#include "eventscripts.h"

#include "z4.h"
#include "z4_energy.h"

#include "gamemode/zb1/zb1_const.h"
#include "gamemode/zb3/zb3_const.h"

CHudZ4Energy::CHudZ4Energy(void)
{
	
}

int CHudZ4Energy::VidInit(void)
{
	return 1;
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

	int idx = IS_FIRSTPERSON_SPEC ? g_iUser2 : gEngfuncs.GetLocalPlayer()->index;
	if (!g_PlayerExtraInfo[idx].zombie)
		return 0;

	return 1;
}