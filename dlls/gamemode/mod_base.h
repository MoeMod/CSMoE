
#ifndef MOD_BASE_H
#define MOD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "mods.h"

class CBasePlayer; // player.h

class IBaseMod_RemoveObjects : public IBaseMod
{
public:
	IBaseMod_RemoveObjects() {}

public: // CHalfLifeMultiplay
	BOOL IsAllowedToSpawn(CBaseEntity *pEntity);
	void CheckMapConditions() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;

};

#endif
