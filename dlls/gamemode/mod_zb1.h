
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
	BOOL IsTeamplay(void) override { return TRUE; }
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void Think() override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	void CheckWinConditions() override;
	int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);

public:
	bool IsZBMode() override { return true; }
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
