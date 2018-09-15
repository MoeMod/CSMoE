#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "globals.h"

#include "mod_zbs.h"

#include "zbs/zs_subs.h"
#include "player/csdm_randomspawn.h"

#include <algorithm>

CMod_ZombieScenario::CMod_ZombieScenario()
{
	m_iRoundTimeSecs = m_iIntroRoundTime = 20 + 2; // keep it from ReadMultiplayCvars
	WaitingSound();
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

void CMod_ZombieScenario::InitHUD(CBasePlayer *pPlayer)
{
	pPlayer->HumanLevel_UpdateHUD();
	return IBaseMod::InitHUD(pPlayer);
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
	m_iRoundTimeSecs = m_iIntroRoundTime = 20 + 2; // keep it from ReadMultiplayCvars

	return IBaseMod_RemoveObjects::CheckMapConditions();
}

bool CMod_ZombieScenario::CanPlayerBuy(CBasePlayer *player, bool display)
{
	// is the player alive?
	if (player->pev->deadflag != DEAD_NO)
	{
		return false;
	}

	// is the player in a buy zone?
	if (!(player->m_signals.GetState() & SIGNAL_BUY))
	{
		return false;
	}

	return true;
}

void CMod_ZombieScenario::RestartRound()
{
	ClearZombieNPC();
	WaitingSound();
	IBaseMod::RestartRound();
}

void CMod_ZombieScenario::PlayerSpawn(CBasePlayer *pPlayer)
{
	IBaseMod::PlayerSpawn(pPlayer);
	pPlayer->pev->health += pPlayer->HumanLevel_GetHealthBonus();
}

void CMod_ZombieScenario::WaitingSound()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "spk zombi_start\n");
	}
}

void CMod_ZombieScenario::Think()
{
	TeamCheck();

	if (m_fTeamCount != 0.0f && m_fTeamCount <= gpGlobals->time)
	{
		//if (m_iNumTerroristWins)
			//g_fGameOver = TRUE; // Game over, changelevel in CheckGameOver().
		//else
			RestartRound();
	}

	if (CheckGameOver())   // someone else quit the game already
		return;

	if (IsFreezePeriod())
	{
		static int iLastCountDown = -1;
		int iCountDown = TimeRemaining();

		if (iCountDown > 0)
		{
			if (iCountDown != iLastCountDown)
			{
				iLastCountDown = iCountDown;
				if (iCountDown > 0 && iCountDown < 20)
				{
					UTIL_ClientPrintAll(HUD_PRINTCENTER, "Waiting for Round Start: %s1 sec(s)", UTIL_dtos1(iCountDown)); // #CSO_ZBS_StartCount
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
		m_tmNextPeriodicThink = gpGlobals->time + 1.0f;

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

	//return IBaseMod::Think();

	if (FRoundStarted() && !m_bRoundTerminating)
	{
		// spawn zombie
		if (gpGlobals->time > m_flNextSpawnNPC)
		{
			MakeZombieNPC();
			m_flNextSpawnNPC = gpGlobals->time + 1.0f;
		}
	}

	if (TimeRemaining() <= 0 && !m_bRoundTerminating)
		HumanWin();
}

BOOL CMod_ZombieScenario::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pEntity);
	if (pPlayer != NULL)
		pPlayer->HumanLevel_Reset();

	return IBaseMod::ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void CMod_ZombieScenario::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!NumAliveCT)
	{
		ZombieWin();
	}

}

void CMod_ZombieScenario::HumanWin()
{
	//Broadcast("ctwin");
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "spk win_human\n");
	}
	EndRoundMessage("Round Clear!!!", ROUND_CTS_WIN);
	TerminateRound(5, WINSTATUS_CTS);

	++m_iNumCTWins;
	UpdateTeamScores();
	ClearZombieNPC();

	if (m_iNumCTWins >= m_iMaxRoundsWon)
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "Congratulations! You've cleared all the Rounds."); // #CSO_CongAllRoundClear
	}
}

void CMod_ZombieScenario::ZombieWin()
{
	//Broadcast("terwin");
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(iIndex));

		if (!player)
			continue;

		player->HumanLevel_Reset();
		CLIENT_COMMAND(player->edict(), "spk win_zombi\n");
	}
	EndRoundMessage("Round Failed...", ROUND_TERRORISTS_WIN);
	TerminateRound(5, WINSTATUS_TERRORISTS);

	++m_iNumTerroristWins;
	UpdateTeamScores();
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
	int iCountDown = gpGlobals->time - m_fRoundCount;
	if (iCountDown <= 20)
		return false;

	return true;
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
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING("monster_entity"));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in MakeZombieNPC()!\n");
		return nullptr;
	}

	CBaseEntity *monster = CBaseEntity::Instance(pent);

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

	pent->v.spawnflags |= SF_NORESPAWN;
	
	DispatchSpawn(pent);
	monster->pev->max_health = 100;
	monster->pev->health = monster->pev->max_health;

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