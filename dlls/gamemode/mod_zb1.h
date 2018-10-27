
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
	BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason) override;
	void ClientDisconnected(edict_t *pClient) override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	void CheckWinConditions() override;
	int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled);
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;

public: // IBaseMod
	DamageTrack_e DamageTrack() override { return DT_ZB; }
	void InstallPlayerModStrategy(CBasePlayer *player) override;

protected:
	virtual int ZombieOriginNum();
	virtual void PickZombieOrigin();
	virtual void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker);
	virtual void RoundEndScore(int iWinStatus);
	virtual void MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel);

protected:
	void TeamCheck();
	void InfectionSound();

	void HumanWin();
	void ZombieWin();

	BOOL FInfectionStarted();
};

class CPlayerModStrategy_ZB1 : public CPlayerModStrategy_Default
{
public:
	CPlayerModStrategy_ZB1(CBasePlayer *player) : CPlayerModStrategy_Default(player) {}
	void CheckBuyZone() override { m_pPlayer->m_signals.Signal(SIGNAL_BUY); };
	bool CanPlayerBuy(bool display) override;
	int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) override;
};

#endif
