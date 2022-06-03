#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "globals.h"

#include "mod_zbs.h"

#include "zbs/zs_subs.h"
#include "zbs/zbs_const.h"
#include "zbs/monster_entity.h"
#include "player/csdm_randomspawn.h"
#include "zbs/monster_manager.h"

#include "player/player_human_level.h"

#include <algorithm>

namespace sv {

class PlayerModStrategy_ZBS : public CPlayerModStrategy_Default
{
public:
	PlayerModStrategy_ZBS(CBasePlayer *player, CMod_ZombieScenario *mp) : CPlayerModStrategy_Default(player), m_HumanLevel(player), m_pModZBS(mp)
	{
		m_listenerAdjustDamage = mp->m_eventAdjustDamage.subscribe(
			[=](CBasePlayer *attacker, float &out) 
			{ 
				if(attacker == m_pPlayer)
					out *= m_HumanLevel.GetAttackBonus(); 
			}
		);
		m_listenerMonsterKilled = mp->m_eventMonsterKilled.subscribe(
			[=](CMonster *victim, CBaseEntity *attacker)
			{
				if (attacker == m_pPlayer)
				{
					if (victim->m_iKillBonusFrags)
						m_pPlayer->AddPoints(victim->m_iKillBonusFrags, FALSE);
					if (victim->m_iKillBonusMoney)
						m_pPlayer->AddAccount(victim->m_iKillBonusMoney);

					MESSAGE_BEGIN(MSG_ONE, gmsgZBSTip, NULL, m_pPlayer->pev);
					WRITE_BYTE(ZBS_TIP_KILL);
					MESSAGE_END();
				}
			}
		);
	}
	int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) override { return 600; }
	bool CanPlayerBuy(bool display) override
	{
		// is the player alive?
		if (m_pPlayer->pev->deadflag != DEAD_NO)
			return false;

		return true;
	}

	void ClientPutInServer() override
	{
		m_flSpawnTimeOffset = 0.0f;
		m_flSpawnTime = invalid_time_point;
	}

	void OnSpawn() override
	{
		m_pPlayer->pev->health += m_HumanLevel.GetHealthBonus();
		m_pPlayer->SpawnProtection_Start(3.0s);
	}

	bool ClientCommand(const char *pcmd) override
	{
		if (FStrEq(pcmd, "zbs_hp_up"))
		{
			m_HumanLevel.LevelUpHealth();
			return true;
		}
		else if (FStrEq(pcmd, "zbs_atk_up"))
		{
			m_HumanLevel.LevelUpAttack();
			return true;
		}
		return CPlayerModStrategy_Default::ClientCommand(pcmd);
	}

	void OnInitHUD() override
	{
		m_HumanLevel.UpdateHUD();
	}

	void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override
	{
		m_flSpawnTime = gpGlobals->time + m_flSpawnTimeOffset * 1s + 5s;
		m_flSpawnTimeOffset = min(m_flSpawnTimeOffset + 5.0f, 60.0f);
	}

	void OnThink() override
	{
		if (!m_pModZBS->FRoundStarted() || m_pModZBS->m_bRoundTerminating)
			return;

		if (m_pPlayer->pev->deadflag != DEAD_DEAD && m_pPlayer->pev->deadflag != DEAD_RESPAWNABLE)
			return;

		if (m_pPlayer->m_iTeam == TEAM_UNASSIGNED || m_pPlayer->m_iTeam == TEAM_SPECTATOR)
			return;

		if (m_flSpawnTime >= gpGlobals->time)
		{
			if (m_flSpawnTips <= gpGlobals->time)
			{
				m_flSpawnTips = gpGlobals->time + 1s;
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_ZBS_AutoRespawnWait", UTIL_dtos1((m_flSpawnTime - gpGlobals->time) / 1s));
			}
			return;
		}

		m_pPlayer->RoundRespawn();
	}

protected:
	EventListener m_listenerAdjustDamage;
	EventListener m_listenerMonsterKilled;
	PlayerExtraHumanLevel_ZBS m_HumanLevel;
	time_point_t m_flSpawnTips;
	time_point_t m_flSpawnTime;
	float m_flSpawnTimeOffset;
	CMod_ZombieScenario* const m_pModZBS;
};

class CMonsterModStrategy_ZBS : public CMonsterModStrategy_Default
{
public:
	CMonsterModStrategy_ZBS(CMonster *p, CMod_ZombieScenario * pGameRules) : CMonsterModStrategy_Default(p), mp(pGameRules)
	{

	}

	void OnKilled(entvars_t *pevKiller, int iGib) override
	{
		CMonsterModStrategy_Default::OnKilled(pevKiller, iGib);
		if (pevKiller)
		{
			CBaseEntity *pKiller = CBaseEntity::Instance(pevKiller);
			mp->m_eventMonsterKilled.dispatch(m_pMonster, pKiller);
		}
	}

protected:
	CMod_ZombieScenario * const mp;
};

void CMod_ZombieScenario::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new PlayerModStrategy_ZBS(player, this));
}

float CMod_ZombieScenario::GetAdjustedEntityDamage(CBaseEntity *victim, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = Base::GetAdjustedEntityDamage(victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);

	if(pevAttacker)
	{
		CBaseEntity *pAttackingEnt = CBaseEntity::Instance(pevAttacker);
		if (pAttackingEnt->IsPlayer())
		{
			CBasePlayer *pAttacker = static_cast<CBasePlayer *>(pAttackingEnt);
			m_eventAdjustDamage.dispatch(pAttacker, flDamage);
		}
	}
		
	return flDamage;
}

CMod_ZombieScenario::CMod_ZombieScenario()
{
	m_iRoundTimeSecs = m_iIntroRoundTime = 20s + 2s; // keep it from ReadMultiplayCvars
	
	PRECACHE_GENERIC("sound/Scenario_Ready.mp3");
	PRECACHE_MODEL("models/player/zombi_origin/zombi_origin.mdl");
	PRECACHE_MODEL("models/player/zombi_host/zombi_host.mdl");
}

void CMod_ZombieScenario::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZBS);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

void CMod_ZombieScenario::CheckMapConditions()
{
	m_vecZombieSpawns.clear();
	CBaseEntity *sp = nullptr;
	while ((sp = UTIL_FindEntityByClassname(sp, "zombiespawn")) != nullptr)
	{
		m_vecZombieSpawns.push_back(static_cast<CZombieSpawn *>(sp));
	}

	// hook from RestartRound()
	m_iRoundTimeSecs = m_iIntroRoundTime = 20s + 2s; // keep it from ReadMultiplayCvars

	return Base::CheckMapConditions();
}

void CMod_ZombieScenario::RestartRound()
{
	if (m_bCompleteReset)
	{
		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity* entity = UTIL_PlayerByIndex(iIndex);

			if (!entity)
				continue;

			CBasePlayer* player = static_cast<CBasePlayer*>(entity);
			auto strategy = dynamic_cast<PlayerModStrategy_ZBS*>(player->m_pModStrategy.get());
			if (strategy)
			{
				strategy->ClientPutInServer();
			}
		}
	}

	ClearZombieNPC();
	CHalfLifeMultiplay::RestartRound();
}

void CMod_ZombieScenario::PlayerSpawn(CBasePlayer *pPlayer)
{
	CHalfLifeMultiplay::PlayerSpawn(pPlayer);
}

void CMod_ZombieScenario::WaitingSound()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "mp3 play sound/Scenario_Ready.mp3\n");
	}
}

void CMod_ZombieScenario::Think()
{
	TeamCheck();

	if (CheckGameOver())   // someone else quit the game already
		return;
	
	if (m_fTeamCount != invalid_time_point && m_fTeamCount <= gpGlobals->time)
	{
		if (m_iNumTerroristWins)
		{
			if (!CheckCanStartVoteMaps() && !m_iVoteMapStatus)
				GoToIntermission();
			else
				RestartRound();
		}
		else
			RestartRound();
	}

#ifdef XASH_DEDICATED
	// votemap things
	VoteMapThink();
#endif

	if (IsFreezePeriod())
	{
		static int iLastCountDown = -1;
		int iCountDown = TimeRemaining() /1s;

		if (iCountDown > 0)
		{
			if (iCountDown != iLastCountDown)
			{
				iLastCountDown = iCountDown;
				if (iCountDown > 0 && iCountDown < 20)
				{
					UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_ZBS_StartCount", UTIL_dtos1(iCountDown));
				}

				if (iCountDown == 19)
				{
					WaitingSound();
				}
			}
		}
		else
		{
			RoundStart();
		}

		CheckFreezePeriodExpired();
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0s;

		if (g_psv_accelerate->value != 5.0f)
		{
			CVAR_SET_FLOAT("sv_accelerate", 5.0);
		}

		if (g_psv_friction->value != 4.0f)
		{
			CVAR_SET_FLOAT("sv_friction", 4.0);
		}

		if (g_psv_stopspeed->value != 75.0f)
		{
			CVAR_SET_FLOAT("sv_stopspeed", 75.0);
		}

		m_iMaxRounds = (int)maxrounds.value;

		if (m_iMaxRounds < 0)
		{
			m_iMaxRounds = 0;
			CVAR_SET_FLOAT("mp_maxrounds", 0);
		}

		m_iMaxRoundsWon = (int)winlimit.value;

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}
	}

	if (FRoundStarted() && !m_bRoundTerminating)
	{
		// spawn zombie
		if (gpGlobals->time > m_flNextSpawnNPC)
		{
			MakeZombieNPC();
			m_flNextSpawnNPC = gpGlobals->time + 1.0s;
		}
	}

	if (TimeRemaining() <= 0s && !m_bRoundTerminating)
		HumanWin();
}

void CMod_ZombieScenario::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	TeamCheck();

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!NumAliveCT)
	{
		ZombieWin();
	}

}

void CMod_ZombieScenario::HumanWin()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgZBSTip);
	WRITE_BYTE(ZBS_TIP_ROUNDCLEAR);
	MESSAGE_END();

	TerminateRound(5s, WINSTATUS_CTS);

	++m_iNumCTWins;
	UpdateTeamScores();
	ClearZombieNPC();

	if (m_iMaxRoundsWon && m_iNumCTWins >= m_iMaxRoundsWon)
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_CongAllRoundClear");
	}
}

void CMod_ZombieScenario::ZombieWin()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgZBSTip);
	WRITE_BYTE(ZBS_TIP_ROUNDFAIL);
	MESSAGE_END();

	TerminateRound(5s, WINSTATUS_TERRORISTS);
	++m_iNumTerroristWins;
	UpdateTeamScores();

	if (CheckCanStartVoteMaps())
	{
		SERVER_COMMAND("start_votemap\n");
	}
}

void CMod_ZombieScenario::TeamCheck()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);

		if (player->m_iTeam == TERRORIST)
		{
			player->m_iTeam = CT;
			player->pev->team = CT;
			TeamChangeUpdate(player, player->m_iTeam);
		}
	}
}

void CMod_ZombieScenario::RoundStart()
{
	m_flNextSpawnNPC = gpGlobals->time;
}

BOOL CMod_ZombieScenario::FRoundStarted()
{
	return !IsFreezePeriod();
}

CZombieSpawn *CMod_ZombieScenario::SelectZombieSpawnPoint()
{
	size_t iSize = m_vecZombieSpawns.size();
	if (!iSize)
		return nullptr;
	return m_vecZombieSpawns[RANDOM_LONG(0, iSize - 1)];
}

CBaseEntity *CMod_ZombieScenario::MakeZombieNPC()
{
#ifndef __AUTO_RESIZE__
	if (MonsterManager().EntityCount() >= MonsterManager().GetMaxNumOfEntity())
		return nullptr;
#endif
	CMonster *monster = GetClassPtr<CMonster>(nullptr);

	if (!monster)
		return nullptr;

	edict_t *pent = monster->edict();

	CZombieSpawn *sp = SelectZombieSpawnPoint();
	if (sp)
	{
		monster->pev->origin = sp->pev->origin;
		monster->pev->angles = sp->pev->angles;
	}
	else
	{
		Vector backup_v_angle = monster->pev->v_angle;
		CSDM_DoRandomSpawn(monster);
		monster->pev->v_angle = backup_v_angle;
	}

	monster->m_pMonsterStrategy.reset(new CMonsterModStrategy_ZBS(monster, this));
	monster->SetClass(MONSTER_CLASS_RANDOM);
	pent->v.spawnflags |= SF_NORESPAWN;
	DispatchSpawn(pent);

	// default settings
	monster->pev->max_health += m_iNumCTWins * 15;
	monster->pev->health = monster->pev->max_health;
	monster->pev->maxspeed += (m_iNumCTWins / 3) * 15;
	if (monster->pev->maxspeed > 250)
		monster->pev->maxspeed = 250;
	monster->m_flAttackDamage += (0.2f * m_iNumCTWins + 1);

	return monster;
}

void CMod_ZombieScenario::ClearZombieNPC()
{
	CBaseEntity *npc = nullptr;
	while ((npc = UTIL_FindEntityByClassname(npc, "monster_entity")) != nullptr)
	{
		npc->Killed(nullptr, GIB_NORMAL);
	}
}

BOOL CMod_ZombieScenario::FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
	if (!FRoundStarted())
		return FALSE;

	return CHalfLifeMultiplay::FPlayerCanTakeDamage(pPlayer, pAttacker);
}

ModConfigData_t CMod_ZombieScenario::QueryModConfig(const char *key, ModConfigData_t defaultValue, CBaseEntity *invoker) {
    if(!strcmp(key, "MaxMoney"))
        return 16000;
    return IBaseMod::QueryModConfig(key, defaultValue, invoker);
}

}