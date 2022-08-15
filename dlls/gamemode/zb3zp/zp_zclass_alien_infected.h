/*
zb2_zclass_nohead.h - CSMoE Gameplay server : Zombie Mod 2
Copyright (C) 2019 Moemod Yanase

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#ifndef PROJECT_ZP_ZCLASS_ALIEN_INFECTED_H
#define PROJECT_ZP_ZCLASS_ALIEN_INFECTED_H
#pragma once
#include "gamemode/zb2/zb2_zclass.h"

namespace sv {

class CZombieClass_Alien_Infected : public CBaseZombieClass_ZB2
{
public:
	CZombieClass_Alien_Infected(CBasePlayer *player, ZombieLevel lv);
	void InitHUD() const override;
	void ResetMaxSpeed() const override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override;
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & kbd) override;
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound_Zombie() override;
	void OnWeaponDeploy(CBasePlayerItem *item) override;
	void Evolution(ZombieLevel iEvolutionLevel) override;
};

}

#endif //PROJECT_ZB2_ZCLASS_TANK_H
