
#ifndef MOD_BASE_H
#define MOD_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "bte_mod.h"

class CBasePlayer; // player.h

class IBaseMod_RemoveObjects : public IBaseMod
{
public:
	IBaseMod_RemoveObjects() {}

public: // CHalfLifeMultiplay
	void CheckMapConditions() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;

};

#endif
