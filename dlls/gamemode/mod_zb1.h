
#ifndef MOD_ZB1_H
#define MOD_ZB1_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

#include <vector>
#include <utility>

class CMod_Zombi : public IBaseMod_RemoveObjects, public IBaseMod_RandomSpawn
{
public:
	CMod_Zombi();

public: // CHalfLifeMultiplay
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void Think() override;

public:
	bool IsZBMode() override { return true; }
	bool CanPlayerBuy(CBasePlayer *player, bool display) override;

};

#endif
