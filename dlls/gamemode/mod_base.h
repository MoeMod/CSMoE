/*
mod_base.h - CSMoE Gameplay server : Base Classes and Interfaces
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

#ifndef MOD_BASE_H
#define MOD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "mods.h"
#include "ruleof350.h"

#include "monsters.h"

#include <memory>

namespace sv {

class CBaseEntity;

class CBasePlayer; // player.h

class IBaseMod : public CHalfLifeMultiplay, ruleof350::unique
{
public:
	virtual DamageTrack_e DamageTrack() { return DT_NONE; }
	virtual void InstallPlayerModStrategy(CBasePlayer *player);
	virtual float
	GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage,
	                        int bitsDamageType) { return flDamage; }
	virtual int MaxMoney() { return 16000; }
	virtual HitBoxGroup
	GetAdjustedTraceAttackHitgroup(CBaseEntity *victim, entvars_t *pevAttacker, float flDamage, const Vector &vecDir,
	                               TraceResult *ptr,
	                               int bitsDamageType) { return static_cast<HitBoxGroup>(ptr->iHitgroup); }
	virtual int
	AddToFullPack_Post(struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player,
	                   unsigned char *pSet) { return 0; }
};

template<class CBase = IBaseMod>
class TBaseMod_RemoveObjects : public CBase
{
	friend BOOL _IBaseMod_RemoveObjects_IsAllowedToSpawn_impl(IBaseMod *mod, CBaseEntity *pEntity);
	friend void _IBaseMod_RemoveObjects_CheckMapConditions_impl(IBaseMod *mod);

public: // CHalfLifeMultiplay
	BOOL IsAllowedToSpawn(CBaseEntity *pEntity) override
	{
		return _IBaseMod_RemoveObjects_IsAllowedToSpawn_impl(this, pEntity);
	}
	void CheckMapConditions() override
	{
		return _IBaseMod_RemoveObjects_CheckMapConditions_impl(this);
	}

protected:
	using Base = TBaseMod_RemoveObjects;
};

template<class CBase = IBaseMod>
class TBaseMod_RandomSpawn : public CBase
{
	friend edict_t *_IBaseMod_RandomSpawn_GetPlayerSpawnSpot_impl(IBaseMod *mod, CBasePlayer *pPlayer);

public: // CHalfLifeMultiplay
	edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer) override
	{
		return _IBaseMod_RandomSpawn_GetPlayerSpawnSpot_impl(this, pPlayer);
	}

protected:
	using Base = TBaseMod_RandomSpawn;
};

}

#endif
