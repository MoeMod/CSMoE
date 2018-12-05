
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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4250) // fuck msvc diamond inhertance warning
#endif

class IBaseMod : public CHalfLifeMultiplay, ruleof350::unique
{
public:
	virtual DamageTrack_e DamageTrack() { return DT_NONE; }
	virtual void InstallPlayerModStrategy(CBasePlayer *player);
	virtual float GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) { return flDamage; }
	virtual int MaxMoney() { return 16000; }
};

class IBaseMod_RemoveObjects : virtual public IBaseMod
{
public: // CHalfLifeMultiplay
	BOOL IsAllowedToSpawn(CBaseEntity *pEntity) override;
	void CheckMapConditions() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
};

class IBaseMod_RandomSpawn : virtual public IBaseMod
{
public: // CHalfLifeMultiplay
	edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer) override;

};


#endif
