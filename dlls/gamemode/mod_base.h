
#ifndef MOD_BASE_H
#define MOD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "mods.h"

class CBasePlayer; // player.h

#pragma warning(push)
#pragma warning(disable:4250) // fuck diamond inhertance warning

class IBaseMod : public CHalfLifeMultiplay
{
public:
	virtual DamageTrack_e DamageTrack() { return DT_NONE; }
	virtual bool FIgnoreBuyZone(CBasePlayer *player) { return false; }
	virtual bool CanPlayerBuy(CBasePlayer *player, bool display) = 0;
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
