/*
zb1_zclass.h - CSMoE Gameplay server : zombie class for zb1
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_ZB1_ZCLASS_H
#define PROJECT_ZB1_ZCLASS_H

#include "player/player_mod_strategy.h"

namespace sv {

class IZombieModeCharacter
{
public:
	virtual ~IZombieModeCharacter() = default;

	virtual void Think() = 0;
	virtual void ResetMaxSpeed() const = 0;
	virtual bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & kbd) = 0;
	virtual float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const = 0;
	virtual void Pain_Zombie(int m_LastHitGroup, bool HasArmour) = 0;
	virtual void DeathSound_Zombie() = 0;
	// activation on constructor in order to RAII
};

class CHuman_ZB1 : public BasePlayerExtra, virtual public IZombieModeCharacter
{
public:
	explicit CHuman_ZB1(CBasePlayer *player); // player_zombie.cpp
	void Think() override {}
	void ResetMaxSpeed() const override {}
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & kbd) override { return false;}
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override { return flDamage; }
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override {}
	void DeathSound_Zombie() override {}
};

class CZombie_ZB1 : public BasePlayerExtra, virtual public IZombieModeCharacter
{
public:
	explicit CZombie_ZB1(CBasePlayer *player, ZombieLevel lv); // player_zombie.cpp
	void Think() override {}
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & data) override { ApplyKnockbackData(m_pPlayer, m_pPlayer->pev->origin - attacker->pev->origin, data); return true; }
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override { return flDamage; }
	void ResetMaxSpeed() const override;
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound_Zombie() override;
};

}

#endif //PROJECT_ZB1_ZCLASS_H
