
#ifndef MOD_ZBS_H
#define MOD_ZBS_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"
#include <vector>

#include "EventDispatcher.h"

class CZombieSpawn;
class CMonster;

class CMod_ZombieScenario : public TBaseMod_RemoveObjects<>
{
public:
	CMod_ZombieScenario();

public: // CHalfLifeMultiplay
	BOOL IsTeamplay(void) override { return TRUE; }
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void Think() override;
	void CheckWinConditions() override;
	void CheckMapConditions() override;

public:
	DamageTrack_e DamageTrack() override { return DT_ZBS; }
	void InstallPlayerModStrategy(CBasePlayer *player) override;
	float GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	int MaxMoney() override { return 32000; }

public:
	void TeamCheck();
	void WaitingSound();
	void RoundStart();
	void HumanWin();
	void ZombieWin();
	BOOL FRoundStarted();

	CZombieSpawn *SelectZombieSpawnPoint();
	CBaseEntity *MakeZombieNPC();
	void ClearZombieNPC();

public:
	std::vector<CZombieSpawn *> m_vecZombieSpawns;
	float m_flNextSpawnNPC;

	EventDispatcher<void(CBasePlayer *attacker, float &)> m_eventAdjustDamage;
	EventDispatcher<void(CMonster *victim, CBaseEntity *attacker)> m_eventMonsterKilled;
};

#endif
