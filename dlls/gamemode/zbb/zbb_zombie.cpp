/*
zbb_zombie.h - CSMoE Gameplay server : Zombie Mod Base-builder
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "zbb_zombie.h"

namespace sv {

	class : public IPlayerBuildingDelegate
	{
	public:
		bool IsBuilding() override
		{
			return false;
		}

		CBaseEntity *CurrentTarget() override
		{
			return nullptr;
		}

		bool IsGhost() override
		{
			return false;
		}
	} s_DelegateZombie;
	IPlayerBuildingDelegate *g_pDelegateZombieShared = &s_DelegateZombie;

	void CZombieClass_ZBB::PostThink()
	{

	}

	CZombieClass_ZBB::CZombieClass_ZBB(CBasePlayer *player) : CZombie_ZB1(player, ZOMBIE_LEVEL_HOST)
	{
		m_pPlayer->pev->flags = 0;
	}

}
