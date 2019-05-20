
#ifndef MOD_DM_H
#define MOD_DM_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"

namespace sv {

class CBaseEntity; // cbase.h
class CBasePlayer; // player.h
typedef struct entvars_s entvars_t; // progdefs.h

class CMod_DeathMatch : public TBaseMod_RemoveObjects<TBaseMod_RandomSpawn<>>
{
public:
	CMod_DeathMatch();

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

public:
	void InstallPlayerModStrategy(CBasePlayer *player) override;

public:
	int CalcLeaderFrags();
	bool CheckWinLimitDM();
	void UpdateTeamScores();
};

}

#endif
