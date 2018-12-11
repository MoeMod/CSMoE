
#ifndef MOD_BASE_H
#define MOD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "mods.h"
#include "ruleof350.h"

#include <memory>

class CBaseEntity;
class CBasePlayer; // player.h

class IBaseMod : public CHalfLifeMultiplay, ruleof350::unique
{
public:
	virtual DamageTrack_e DamageTrack() { return DT_NONE; }
	virtual void InstallPlayerModStrategy(CBasePlayer *player);
	virtual float GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return flDamage; }
	virtual int MaxMoney() { return 16000; }
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


#endif
