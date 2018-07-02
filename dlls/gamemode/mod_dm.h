
#ifndef MOD_DM_H
#define MOD_DM_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

class CBaseEntity; // cbase.h
class CBasePlayer; // player.h
typedef struct entvars_s entvars_t; // progdefs.h

class CMod_DeathMatch : public IBaseMod_RemoveObjects, public IBaseMod_RandomSpawn
{
public:
	CMod_DeathMatch();

	using IBaseMod_RemoveObjects::IsAllowedToSpawn;

public: // CHalfLifeMultiplay
	BOOL IsTeamplay(void) override { return FALSE; }
	void Think(void) override;
	int PlayerRelationship(CBasePlayer *pPlayer, CBaseEntity *pTarget) override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	BOOL FPlayerCanRespawn(CBasePlayer *pPlayer) override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void CheckWinConditions() override {}
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	bool CanPlayerBuy(CBasePlayer *player, bool display) override { return true; }
public:
	bool IsZBMode() override { return false; }

public:
	int CalcLeaderFrags();
	bool CheckWinLimitDM();
	void UpdateTeamScores();
};

#endif
