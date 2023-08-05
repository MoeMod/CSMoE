
#ifndef MOD_ZBS_H
#define MOD_ZBS_H
#ifdef _WIN32
#pragma once
#endif

#include "player/player_mod_strategy.h"
#include "player/player_human_level.h"
#include "mod_base.h"
#include <vector>

#include "EventDispatcher.h"

namespace sv {


enum WaveState_e
{
	WAVE_END,
	WAVE_OPEN,
	WAVE_KILL
};

enum gamemode_e
{
	MODE_DEF,
	MODE_CHASE,
};

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
	edict_t* GetPlayerSpawnSpot(CBasePlayer* pPlayer) override;

public:
	DamageTrack_e DamageTrack() override { return DT_ZBS; }
	void InstallPlayerModStrategy(CBasePlayer *player) override;
	float GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
    ModConfigData_t QueryModConfig(const char *key, ModConfigData_t defaultValue = {}, CBaseEntity *invoker = nullptr) override;

public:
	void TeamCheck();
	void WaitingSound();
	void RoundStart();
	void HumanWin();
	void ZombieWin();
	duration_t TimeRemaining();
	BOOL FRoundStarted();
	BOOL FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker) override;
	BOOL IsSpawnPointValid(CBaseEntity* pPlayer, Vector origin);

	CZombieSpawn *SelectZombieSpawnPoint();
	CZombieSpawn* SelectZombieSpawnPoint(int iID);
	CBaseEntity *MakeZombieNPC();
	void ClearZombieNPC();

	CBaseEntity *MakeZombieNPC(int spawnid, std::map<int, int> vecZombi);

public:
	std::vector<CZombieSpawn *> m_vecZombieSpawns;
	EngineClock::time_point m_flNextSpawnNPC;

	EventDispatcher<void(CBasePlayer *attacker, float &)> m_eventAdjustDamage;
	EventDispatcher<void(CMonster *victim, CBaseEntity *attacker)> m_eventMonsterKilled;

	duration_t m_flIntervalAdd;
};

class PlayerModStrategy_ZBS : public CPlayerModStrategy_Default
{
public:

	PlayerModStrategy_ZBS(CBasePlayer* player, CMod_ZombieScenario* mp);

	int ComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax) override;
	bool CanPlayerBuy(bool display) override;
	void ClientPutInServer() override;
	void OnSpawn() override;
	bool ClientCommand(const char* pcmd) override;

	void OnInitHUD() override;
	void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override;
	void OnThink() override;
	void UpdateStatusBar(CBaseEntity* pEntity, char* sbuf0, int* newSBarState) override;

	float m_flWallTakeDamage;
protected:
	EventListener m_listenerAdjustDamage;
	EventListener m_listenerMonsterKilled;
	PlayerExtraHumanLevel_ZBS m_HumanLevel;
	time_point_t m_flSpawnTips;
	time_point_t m_flSpawnTime;
	float m_flSpawnTimeOffset;

	CMod_ZombieScenario* const m_pModZBS;
};

}

#endif
