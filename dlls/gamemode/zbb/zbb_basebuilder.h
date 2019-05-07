/*
basebuilder.h - CSMoE Gameplay server
Copyright (C) 2019 Moemod Hymei

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef ZBB_BASEBUILDER_H
#define ZBB_BASEBUILDER_H
#ifdef _WIN32
#pragma once
#endif

#include "gamemode/mod_zbb.h"

class PlayerBuildingDelegate : public BasePlayerExtra, public IPlayerBuildingDelegate
{
public:
	PlayerBuildingDelegate(CBasePlayer *player, CMod_ZombieBaseBuilder *mod) : BasePlayerExtra(player), m_pModZBB(mod) {}

	bool IsBuilding() override
	{
		return m_bIsBuilding;
	}

	CBaseEntity *CurrentTarget() override
	{
		return m_pPointingTarget;
	}

	bool IsGhost() override
	{
		return false;
	}


	void Build_Start();
	void Build_Update();
	void Build_End();
	void AddPushRate(float delta);

private:

	CMod_ZombieBaseBuilder * const m_pModZBB;

	bool m_bCanBuild = true;
	bool m_bIsBuilding = false;
	float m_flBuildDistance = 0.0f;
	CBaseEntity* m_pPointingTarget = nullptr;
	Vector m_vecOffset;
};


class CHuman_ZBB : public CHuman_ZB1, public IZombieModeCharacter_ZBB_ExtraDelegate
{
public:
	CHuman_ZBB(CBasePlayer *player, CMod_ZombieBaseBuilder *mp) : CHuman_ZB1(player), m_Build(player, mp) {}
	void ButtonEvent(unsigned short &bitsCurButton, int bitsOldButton) override;

	void Think() override { m_Build.Build_Update();}
	void PostThink() override {}
	bool IsBuilding() override { return m_Build.IsBuilding(); }
public:
	PlayerBuildingDelegate m_Build;
};

#endif