
#ifndef MOD_ZBS_H
#define MOD_ZBS_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

class CMod_ZombieScenario : public IBaseMod_RemoveObjects
{
public:
	CMod_ZombieScenario() {}

public: // CHalfLifeMultiplay
	BOOL IsTeamplay(void) override { return TRUE; }
	void UpdateGameMode(CBasePlayer *pPlayer) override;

public:
	bool IsZBMode() override { return false; }
	bool CanPlayerBuy(CBasePlayer *player, bool display) override;

};

#endif
