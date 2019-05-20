/*
mob_zbb.h - CSMoE Gameplay server : Zombie Mod Base-builder
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

#ifndef PROJECT_MOD_ZBB_H
#define PROJECT_MOD_ZBB_H

#include "mod_zb1.h"
#include <array>
#include <unordered_map>

namespace sv {

class IPlayerBuildingDelegate
{
public:
	virtual ~IPlayerBuildingDelegate() = default;
	virtual bool IsBuilding() = 0;
	virtual CBaseEntity *CurrentTarget() = 0;
	virtual bool IsGhost () = 0;
};

class IZombieModeCharacter_ZBB_ExtraDelegate
{
public:
	virtual ~IZombieModeCharacter_ZBB_ExtraDelegate() = default;
	virtual void ButtonEvent(unsigned short &bitsCurButton, int bitsOldButton) = 0;
	virtual bool IsBuilding() = 0;
	virtual void PostThink() = 0;
};

class CMod_ZombieBaseBuilder : public CMod_Zombi
{
public:
	CMod_ZombieBaseBuilder();

	BOOL IsAllowedToSpawn(CBaseEntity *pEntity) override;

	void UpdateGameMode(CBasePlayer *pPlayer) override;

	void InstallPlayerModStrategy(CBasePlayer *player) override;

	int AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet)  override;

	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;

public:
	bool CanEntityBuild(CBaseEntity *pEntity);
	// no need for weak_ptr, cause CPlayerModStrategy_ZBB handles its lifespan.
	std::array<IPlayerBuildingDelegate *, MAX_CLIENTS> m_BuildingInterfaces;
	std::unordered_map<CBaseEntity *, IPlayerBuildingDelegate *> m_BuildingEntities;
};

}

#endif //PROJECT_MOD_ZBB_H
