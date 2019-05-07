/*
zbb_ghost.h - CSMoE Gameplay server
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

#ifndef PROJECT_ZBB_GHOST_H
#define PROJECT_ZBB_GHOST_H

#include "player/player_mod_strategy.h"
#include "gamemode/zb1/zb1_zclass.h"
#include "gamemode/mod_zbb.h"


class CGhost_ZBB : public BasePlayerExtra, public IZombieModeCharacter, public IZombieModeCharacter_ZBB_ExtraDelegate
{
public:
	explicit CGhost_ZBB(CBasePlayer *player);

	// IZombieModeCharacter
	void Think() override;
	void PostThink() override;
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & data) override { ApplyKnockbackData(m_pPlayer, m_pPlayer->pev->origin - attacker->pev->origin, data); return true; }
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override { return 0.0f; }
	void ResetMaxSpeed() const override;
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound_Zombie() override;

	// IZombieModeCharacter_ZBB_ExtraDelegate
	void ButtonEvent(unsigned short &bitsCurButton, int bitsOldButton) override {}
	bool IsBuilding() override { return false; }

	void GiveDefaultItems();
};

extern IPlayerBuildingDelegate *g_pDelegateGhostShared;


#endif //PROJECT_ZBB_GHOST_H
