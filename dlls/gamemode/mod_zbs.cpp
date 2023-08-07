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
#include "zbs/zbs_map_manager.h"


#include <algorithm>

namespace sv {


PlayerModStrategy_ZBS::PlayerModStrategy_ZBS(CBasePlayer *player, CMod_ZombieScenario *mp) : CPlayerModStrategy_Default(player), m_HumanLevel(player), m_pModZBS(mp)
{
	m_flWallTakeDamage = 0.0f;
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

int PlayerModStrategy_ZBS::ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) 
{ 
	return 600;
}

bool PlayerModStrategy_ZBS::CanPlayerBuy(bool display)
{
	// is the player alive?
	if (m_pPlayer->pev->deadflag != DEAD_NO)
		return false;

	return true;
}

void PlayerModStrategy_ZBS::ClientPutInServer()
{
	m_flSpawnTimeOffset = 0.0f;
	m_flSpawnTime = invalid_time_point;
}

void PlayerModStrategy_ZBS::OnSpawn()
{
	m_pPlayer->pev->health += m_HumanLevel.GetHealthBonus();
	m_pPlayer->SpawnProtection_Start(3.0s);
}

bool PlayerModStrategy_ZBS::ClientCommand(const char *pcmd)
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
	else if (FStrEq(pcmd, "humanwin"))
	{
		m_pModZBS->HumanWin();
	}
	return CPlayerModStrategy_Default::ClientCommand(pcmd);
}

void PlayerModStrategy_ZBS::OnInitHUD()
{
	m_HumanLevel.UpdateHUD();
}

void PlayerModStrategy_ZBS::OnKilled(entvars_t* pKiller, entvars_t* pInflictor)
{
	m_flSpawnTime = gpGlobals->time + m_flSpawnTimeOffset * 1s + 5s;
	m_flSpawnTimeOffset = min(m_flSpawnTimeOffset + 5.0f, 60.0f);
}

void PlayerModStrategy_ZBS::OnThink()
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

void PlayerModStrategy_ZBS::UpdateStatusBar(CBaseEntity* pEntity, char* sbuf0, int* newSBarState)
{
	if (!FClassnameIs(pEntity->pev, "zbsbreak"))
		return;

	if (playerid.value != PLAYERID_OFF || m_pPlayer->IsObserver())
		strcpy(sbuf0, "1 %c1 %i3");
	else
		strcpy(sbuf0, " ");

	newSBarState[SBAR_ID_TARGETTYPE] = SBAR_TARGETTYPE_WALL;
	newSBarState[SBAR_ID_TARGETHEALTH] = int(pEntity->pev->health);

	m_pPlayer->m_flStatusBarDisappearDelay = gpGlobals->time + 2s;
}


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


extern std::vector<SpawnData> g_ZBSSpawnData;
extern std::vector<WaveData> g_ZBSWaveData;
extern std::vector<ZombiTypeData> g_ZBSZombiTypeData;
extern ZBSData g_ZBSData;
extern std::vector<ZBSRoundData> g_ZBSRoundData;
extern std::vector<TriggerData> g_ZBSTriggerData;

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
	m_flIntervalAdd = 0s;
	PRECACHE_GENERIC("sound/Scenario_Ready.mp3");
	PRECACHE_GENERIC("sound/Scenario_Normal.mp3");
	PRECACHE_GENERIC("sound/Scenario_Rush.mp3");
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
	m_iLimitTeams = 0;
	CVAR_SET_FLOAT("mp_autoteambalance", 0);

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

					if (!g_ZBSTriggerData.empty())
					{
						for (auto waveData = g_ZBSWaveData.begin(); waveData != g_ZBSWaveData.end(); waveData++)
						{
							(*waveData).wavestate = WAVE_END;
							(*waveData).m_flNextWave = invalid_time_point;
							(*waveData).m_flEndWave = invalid_time_point;
						}
					}
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

		if (g_ZBSData.maxround)
		{
			m_iMaxRoundsWon = g_ZBSData.maxround;
		}
		else
		{
			m_iMaxRoundsWon = (int)winlimit.value;
		}
		

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}
	}

	if (FRoundStarted() && !m_bRoundTerminating)
	{
		// spawn zombie
		if (!g_ZBSRoundData.empty() && !g_ZBSWaveData.empty())
		{	
			for (auto iter = g_ZBSWaveData.begin(); iter != g_ZBSWaveData.end(); iter++)
			{	
				if (iter->m_flNextWave != invalid_time_point && iter->m_flNextWave < gpGlobals->time)
				{
					(*iter).m_flNextWave = gpGlobals->time + iter->intervalTime + m_flIntervalAdd;
					//for jun00 boss only spawn once

					if (iter->intervalTime == 0s)
					{
						(*iter).m_flNextWave = gpGlobals->time + iter->endTime;
					}
					
					if (iter->spawnPoint == 9999)
					{
						//todo hud msg
						
						UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_ZS_Warning_MSG_%s", iter->eventmessage.c_str());
					}
					else if((*iter).wavestate == WAVE_OPEN)
					{
						MakeZombieNPC(iter->waveID, iter->spawnPoint, iter->vecZombi);
					}

					if (iter->startTime >= iter->endTime)
					{
						//once msg or spawn boss
						(*iter).m_flNextWave = invalid_time_point;
						(*iter).m_flEndWave = invalid_time_point;
						(*iter).wavestate = WAVE_END;
					}
				}

				if (iter->startTime < iter->endTime && iter->m_flEndWave != invalid_time_point && iter->m_flEndWave < gpGlobals->time)
				{
					(*iter).m_flEndWave = invalid_time_point;
					(*iter).wavestate = WAVE_END;
				}
			}

		}
		else
		{
			if (gpGlobals->time > m_flNextSpawnNPC)
			{
				MakeZombieNPC();
				m_flNextSpawnNPC = gpGlobals->time + 2.0s;
			}
		}
	}

	if (g_ZBSData.gametype != MODE_CHASE)
	{
		if (TimeRemaining() <= 0s && !m_bRoundTerminating)
			HumanWin();
	}
	
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
	UTIL_BroadcastStuffText("mp3 stop\n");

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
	UTIL_BroadcastStuffText("mp3 stop\n");

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

duration_t CMod_ZombieScenario::TimeRemaining() 
{ 

	if (g_ZBSData.gametype == MODE_CHASE && !IsFreezePeriod())
	{
		return  duration_t(gpGlobals->time - m_fRoundCount);
	}
	else
		return duration_t(m_iRoundTimeSecs) - (gpGlobals->time - m_fRoundCount);
	
}

void CMod_ZombieScenario::RoundStart()
{
	if (g_ZBSData.gametype == MODE_DEF && g_ZBSData.roundTime)
	{
		m_iRoundTime = g_ZBSData.roundTime * 1s;
	}
	else if (g_ZBSData.gametype == MODE_CHASE)
	{

	}

	m_flNextSpawnNPC = gpGlobals->time;


	if (!g_ZBSWaveData.empty() && !g_ZBSRoundData.empty())
	{
		for (auto roundData = g_ZBSRoundData.begin(); roundData != g_ZBSRoundData.end(); roundData++)
		{
			if ((m_iNumCTWins + 1) == roundData->round)
			{
				if (roundData->BGM == 6)
					UTIL_BroadcastStuffText("mp3 loop sound/Scenario_Rush.mp3\n");
				else if (roundData->BGM == 5)
					UTIL_BroadcastStuffText("mp3 loop sound/Scenario_Normal.mp3\n");
				else
					UTIL_BroadcastStuffText("mp3 loop sound/Scenario_Normal.mp3\n");
				
				m_flIntervalAdd = roundData->IntervalAdd;

				if (g_ZBSTriggerData.empty())
				{
					for (auto waveData = g_ZBSWaveData.begin(); waveData != g_ZBSWaveData.end(); waveData++)
					{
						if (std::find(roundData->waveTypes.begin(), roundData->waveTypes.end(), waveData->waveID) != roundData->waveTypes.end())
						{
							(*waveData).wavestate = WAVE_OPEN;
							(*waveData).m_flNextWave = gpGlobals->time + waveData->startTime;
							(*waveData).m_flEndWave = gpGlobals->time + waveData->endTime;
						}
					}
				}
				else
				{	
#if 0
					//close all wave use map trigger
					for (auto waveData = g_ZBSWaveData.begin(); waveData != g_ZBSWaveData.end(); waveData++)
					{
						(*waveData).wavestate = WAVE_END;
						(*waveData).m_flNextWave = invalid_time_point;
						(*waveData).m_flEndWave = invalid_time_point;
					}
					//abandon
					for (auto trg = g_ZBSTriggerData.begin(); trg != g_ZBSTriggerData.end(); trg++)
					{
						if (std::find(roundData->waveTypes.begin(), roundData->waveTypes.end(), trg->id) != roundData->waveTypes.end())
						{
							for (auto waveData = g_ZBSWaveData.begin(); waveData != g_ZBSWaveData.end(); waveData++)
							{
								if (std::find(roundData->waveTypes.begin(), roundData->waveTypes.end(), trg->waveid) == roundData->waveTypes.end())
								{
									//close wave
									(*waveData).wavestate = WAVE_END;
									(*waveData).m_flNextWave = invalid_time_point;
									(*waveData).m_flEndWave = invalid_time_point;
								}
								else
								{
									//open wave need? no, use map trigger
								}
							}
						}
					}
#endif
				}	
				break;
			}
		}

	}
}

BOOL CMod_ZombieScenario::FRoundStarted()
{
	return !IsFreezePeriod();
}

BOOL CMod_ZombieScenario::IsSpawnPointValid(CBaseEntity* pPlayer, Vector origin)
{
	CBaseEntity* ent = NULL;

	while ((ent = UTIL_FindEntityInSphere(ent, origin, 64)) != NULL)
	{
		// if ent is a client, don't spawn on 'em
		if (ent->IsPlayer() && ent != pPlayer)
			return FALSE;
	}

	return TRUE;
}

edict_t* CMod_ZombieScenario::GetPlayerSpawnSpot(CBasePlayer* pPlayer)
{
	CBaseEntity* pEntity = NULL;
	edict_t* pentSpawnSpot;

	pPlayer->SelectSpawnSpot("playerspawn", pEntity);

	if (pEntity)
	{
		pPlayer->pev->origin = pEntity->pev->origin + Vector(0, 0, 1);
		pPlayer->pev->v_angle = g_vecZero;
		pPlayer->pev->velocity = g_vecZero;
		pPlayer->pev->angles = pEntity->pev->angles;
		pPlayer->pev->punchangle = g_vecZero;
		pPlayer->pev->fixangle = TRUE;

		pentSpawnSpot = pEntity->edict();
	}
	else
		pentSpawnSpot = CHalfLifeMultiplay::GetPlayerSpawnSpot(pPlayer);


	if (!g_ZBSSpawnData.empty() && !g_ZBSRoundData.empty())
	{
		int iPlayerSpawn = 0;
		for (auto roundData = g_ZBSRoundData.begin(); roundData != g_ZBSRoundData.end(); roundData++)
		{
			if ((m_iNumCTWins + 1) == roundData->round)
			{
				iPlayerSpawn = roundData->PlayerSpawn;
				break;
			}
		}

		for (auto iter = g_ZBSSpawnData.begin(); iter != g_ZBSSpawnData.end(); iter++)
		{
			if (iPlayerSpawn != iter->spawnPoint)
				continue;

			if (IsSpawnPointValid(pPlayer, iter->origin) && iter->team != 1)
			{
				pPlayer->pev->origin = iter->origin;
				pPlayer->pev->angles = iter->angle;
				pPlayer->pev->velocity = g_vecZero;
				pPlayer->pev->v_angle = g_vecZero;
				pPlayer->pev->punchangle = g_vecZero;
				pPlayer->pev->fixangle = 1;
				break;
			}
		}
	}
	return pentSpawnSpot;
}

CZombieSpawn *CMod_ZombieScenario::SelectZombieSpawnPoint()
{
	size_t iSize = m_vecZombieSpawns.size();
	if (!iSize)
		return nullptr;
	return m_vecZombieSpawns[RANDOM_LONG(0, iSize - 1)];
}

CZombieSpawn* CMod_ZombieScenario::SelectZombieSpawnPoint(int iId)
{
	size_t iSize = m_vecZombieSpawns.size();
	if (!iSize)
		return nullptr;

	for (auto pSpawn = m_vecZombieSpawns.begin(); pSpawn != m_vecZombieSpawns.end(); pSpawn++)
	{
		if ((*pSpawn)->m_iId == iId)
			return *pSpawn;
	}

	return nullptr;
}

int randomByWeight(std::map<int, int> _mapWeight)
{
	int nums;

	// 权重累加值
	int accumulate = 0;
	for (auto iter : _mapWeight)
		accumulate += iter.second;

	// 生成权重随机值
	std::srand((unsigned)time(NULL));
	int randNum = std::rand() % accumulate;
	int sum = 0;
	for (auto iter : _mapWeight)
	{
		// 与传入的权重对比, 权重越大则条件越容易成立
		sum += iter.second;
		if (sum > randNum)
		{
			// 拿到权重退出
			nums = iter.first;
			break;
		}
	}
	

	return nums;

}

CBaseEntity* CMod_ZombieScenario::MakeZombieNPC(int waveid, int spawnid, std::map<int, int> vecZombi)
{
	int priority = 0;
	int ZombiType = randomByWeight(vecZombi);

	if (!g_ZBSZombiTypeData.empty())
	{
		for (auto iter = g_ZBSZombiTypeData.begin(); iter != g_ZBSZombiTypeData.end(); iter++)
		{
			if (iter->id == ZombiType)
			{
				priority = iter->priority;
				break;
			}
		}
	}

#ifndef __AUTO_RESIZE__
	if (MonsterManager().EntityCount() >= MonsterManager().GetMaxNumOfEntity() && !priority)
		return nullptr;
#endif
	CMonster* monster = GetClassPtr<CMonster>(nullptr);

	if (!monster)
		return nullptr;

	edict_t* pent = monster->edict();

	Vector origin, angles;
	

	CZombieSpawn* sp = SelectZombieSpawnPoint();
	if (sp)
	{
		origin = sp->pev->origin;
		angles = sp->pev->angles;
	}
	else
	{
		Vector backup_v_angle = monster->pev->v_angle;
		CSDM_DoRandomSpawn(monster);
		monster->pev->v_angle = backup_v_angle;
	}

	for (auto iter = g_ZBSSpawnData.begin(); iter != g_ZBSSpawnData.end(); iter++)
	{
		if (iter->spawnPoint == spawnid && iter->team == 1)
		{
			sp = SelectZombieSpawnPoint(spawnid);

			if (!iter->origin.IsZero())
			{
				monster->pev->origin = iter->origin;
				monster->pev->angles = iter->angle;
			}
			else
			{
				if (sp)
				{
					monster->pev->origin = sp->pev->origin;
					monster->pev->angles = sp->pev->angles;
				}
				else
				{
					monster->pev->origin = sp->pev->origin;
					monster->pev->angles = sp->pev->angles;
				}
			}

			break;
			//monster->m_iTeam = iter->team;
		}
	}

	if (!g_ZBSZombiTypeData.empty())
	{
		for (auto iter = g_ZBSZombiTypeData.begin(); iter != g_ZBSZombiTypeData.end(); iter++)
		{
			if (iter->id == ZombiType)
			{
				monster->m_pMonsterStrategy.reset(new CMonsterModStrategy_ZBS(monster, this));
				monster->SetClass(iter->appearance.c_str());
				monster->SetModel(iter->appearance.c_str());
				pent->v.spawnflags |= SF_NORESPAWN;
				DispatchSpawn(pent);

				//default setting
				float damageAdd = (0.2f * m_iNumCTWins + 1);
				float attackRangeAdd = 0;
				float healthAdd = m_iNumCTWins * 15;
				float armorAdd = 0;
				float speedAdd = (m_iNumCTWins / 3) * 15;
				int moneyAdd = 0;

				for (auto roundData = g_ZBSRoundData.begin(); roundData != g_ZBSRoundData.end(); roundData++)
				{
					if (roundData->round == (m_iNumCTWins + 1))
					{
						damageAdd = roundData->damageAdd;
						attackRangeAdd = roundData->attackRangeAdd;
						healthAdd = roundData->healthAdd;
						armorAdd = roundData->armorAdd;
						speedAdd = roundData->speedAdd;
						moneyAdd = roundData->moneyAdd;
						break;
					}
				}

				monster->pev->max_health = iter->health + healthAdd;
				monster->pev->health = monster->pev->max_health;
				monster->pev->armortype = ARMOR_TYPE_HELMET;
				monster->pev->armorvalue = iter->armor + healthAdd;
				monster->pev->maxspeed = iter->speed + speedAdd;

				monster->m_flAttackDamage = iter->attackDamage + damageAdd;
				monster->m_flAttackDist = iter->attackRange + attackRangeAdd;
				monster->m_flAttackRate = iter->attackInterval;
				monster->m_iKillBonusMoney = iter->bonusMoney + moneyAdd;
				
				monster->m_iPriority = iter->priority;
				monster->m_iWaveID = waveid;
				return monster;
				break;
			}
		}
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
        return 32000;
    return IBaseMod::QueryModConfig(key, defaultValue, invoker);
}

}