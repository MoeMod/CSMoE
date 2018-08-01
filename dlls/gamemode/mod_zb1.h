
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
	void CheckMapConditions() override;
	BOOL IsTeamplay(void) override { return TRUE; }
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void Think() override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	void CheckWinConditions() override;
	int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;

public: // IBaseMod
	bool IsZBMode() override { return true; }
	bool FIgnoreBuyZone(CBasePlayer *player) override { return true; }
	bool CanPlayerBuy(CBasePlayer *player, bool display) override;

protected:
	virtual int ZombieOriginNum();
	virtual void MakeZombieOrigin();
	virtual void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker);
	virtual void RoundEndScore(int iWinStatus);

protected:
	void TeamCheck();
	void InfectionSound();
};

#endif
