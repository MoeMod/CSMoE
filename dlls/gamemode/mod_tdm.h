
#ifndef MOD_TDM_H
#define MOD_TDM_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

class CBaseEntity; // cbase.h
class CBasePlayer; // player.h
typedef struct entvars_s entvars_t; // progdefs.h

class CMod_TeamDeathMatch : public IBaseMod_RemoveObjects
{
public:
	CMod_TeamDeathMatch();

public: // CHalfLifeMultiplay
	BOOL IsTeamplay(void) override { return TRUE; }
	void Think(void) override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	BOOL FPlayerCanRespawn(CBasePlayer *pPlayer) override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void CheckWinConditions() override {}
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	bool CanPlayerBuy(CBasePlayer *player, bool display) override { return true; }
};

#endif
