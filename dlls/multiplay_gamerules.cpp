#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "skill.h"
#include "game.h"
#include "items.h"
#include "hltv.h"
#include "client.h"
#include "shake.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"
#include "world.h"

#include "debug.h"

#include "pm_shared.h"
#include "utllinkedlist.h"

// CSBOT and Nav
#include "game_shared/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game_shared/bot/bot_util.h"
#include "game_shared/bot/simple_state_machine.h"

#include "game_shared/steam_util.h"

#include "game_shared/bot/bot_manager.h"
#include "game_shared/bot/bot_constants.h"
#include "game_shared/bot/bot.h"

#include "game_shared/shared_util.h"
#include "game_shared/bot/bot_profile.h"

#include "game_shared/bot/nav.h"
#include "game_shared/bot/improv.h"
#include "game_shared/bot/nav_node.h"
#include "game_shared/bot/nav_area.h"
#include "game_shared/bot/nav_file.h"
#include "game_shared/bot/nav_path.h"

#include "airtank.h"
#include "h_ai.h"
#include "h_cycler.h"
#include "h_battery.h"

// Hostage
#include "hostage/hostage.h"
#include "hostage/hostage_localnav.h"

#include "bot/cs_bot.h"

// Tutor
#include "tutor.h"
#include "tutor_base_states.h"
#include "tutor_base_tutor.h"
#include "tutor_cs_states.h"
#include "tutor_cs_tutor.h"

#include "gamerules.h"
#include "career_tasks.h"
#include "maprules.h"

#include <ctype.h>

/*
* Globals initialization
*/
#define COM_TOKEN_LEN				1500
static char mp_com_token[ COM_TOKEN_LEN ];
cvar_t *sv_clienttrace = NULL;

CCStrikeGameMgrHelper g_GameMgrHelper;
CHalfLifeMultiplay *g_pMPGameRules = NULL;

bool IsBotSpeaking()
{
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (pPlayer == NULL || !pPlayer->IsBot())
			continue;

		CCSBot *pBot = static_cast<CCSBot *>(pPlayer);

		if (pBot->IsUsingVoice())
			return true;
	}

	return false;
}

void SV_Continue_f()
{
	CHalfLifeMultiplay *mp = g_pGameRules;

	if (mp->IsCareer() && mp->m_fTeamCount > 100000.0)
	{
		mp->m_fTeamCount = gpGlobals->time;

		// go continue
		MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
			WRITE_STRING("GOGOGO");
		MESSAGE_END();

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

			if (pPlayer && !pPlayer->IsBot())
			{
				// at the end of the round is showed window with the proposal surrender or continue
				// now of this time HUD is completely hidden
				// we must to restore HUD after entered continued
				pPlayer->m_iHideHUD &= ~HIDEHUD_ALL;
			}
		}
	}
}

void SV_Tutor_Toggle_f()
{
	CVAR_SET_FLOAT("tutor_enable", (CVAR_GET_FLOAT("tutor_enable") <= 0.0));
}

void SV_Career_Restart_f()
{
	CHalfLifeMultiplay *mp = g_pGameRules;

	if (mp->IsCareer())
	{
		mp->CareerRestart();
	}
}

void SV_Career_EndRound_f()
{
	CHalfLifeMultiplay *mp = g_pGameRules;

	if (!mp->IsCareer() || !mp->IsInCareerRound())
	{
		return;
	}

	CBasePlayer *localPlayer = UTIL_GetLocalPlayer();

	if (localPlayer != NULL)
	{
		SERVER_COMMAND("kill\n");

		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

			if (!player || FNullEnt(player->pev))
				continue;

			if (player->IsBot() && player->m_iTeam == localPlayer->m_iTeam)
			{
				SERVER_COMMAND(UTIL_VarArgs("bot_kill \"%s\"\n", STRING(player->pev->netname)));
			}
		}
	}
}

bool CHalfLifeMultiplay::IsInCareerRound()
{
	return IsMatchStarted() ? false : true;
}

void SV_CareerAddTask_f()
{
	if (CMD_ARGC() != 7)
		return;

	const char *taskName = CMD_ARGV(1);
	const char *weaponName = CMD_ARGV(2);

	int reps = Q_atoi(CMD_ARGV(3));
	bool mustLive = Q_atoi(CMD_ARGV(4)) != 0;
	bool crossRounds = Q_atoi(CMD_ARGV(5)) != 0;
	bool isComplete = Q_atoi(CMD_ARGV(6)) != 0;

	if (TheCareerTasks != NULL)
	{
		TheCareerTasks->AddTask(taskName, weaponName, reps, mustLive, crossRounds, isComplete);
	}
}

void SV_CareerMatchLimit_f()
{
	if (CMD_ARGC() != 3)
	{
		return;
	}

	CHalfLifeMultiplay *mp = g_pGameRules;

	if (mp->IsCareer())
	{
		mp->SetCareerMatchLimit(Q_atoi(CMD_ARGV(1)), Q_atoi(CMD_ARGV(2)));
	}
}

void CHalfLifeMultiplay::SetCareerMatchLimit(int minWins, int winDifference)
{
	if (!IsCareer())
	{
		return;
	}

	if (!m_iCareerMatchWins)
	{
		m_iCareerMatchWins = minWins;
		m_iRoundWinDifference = winDifference;
	}
}

BOOL CHalfLifeMultiplay::IsCareer()
{
	return IS_CAREER_MATCH();
}

void CHalfLifeMultiplay::ServerDeactivate()
{
	if (!IsCareer())
	{
		return;
	}

	CVAR_SET_FLOAT("pausable", 0);
	CVAR_SET_FLOAT("mp_windifference", 1);
	UTIL_LogPrintf("Career End\n");
}

void CMapInfo::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "buying"))
	{
		m_iBuyingStatus = Q_atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bombradius"))
	{
		m_flBombRadius = Q_atoi(pkvd->szValue);

		if (m_flBombRadius > MAX_BOMB_RADIUS)
			m_flBombRadius = MAX_BOMB_RADIUS;

		pkvd->fHandled = TRUE;
	}
}

void CMapInfo::Spawn()
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

LINK_ENTITY_TO_CLASS(info_map_parameters, CMapInfo);

bool CCStrikeGameMgrHelper::CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pSender)
{
	if (!pSender->IsPlayer() || pListener->m_iTeam != pSender->m_iTeam)
	{
		return false;
	}

	BOOL bListenerAlive = pListener->IsAlive();
	BOOL bSenderAlive = pSender->IsAlive();

	if (pListener->IsObserver())
	{
		return true;
	}

	if (bListenerAlive)
	{
		if (!bSenderAlive)
			return false;
	}
	else
	{
		if (bSenderAlive)
			return true;
	}

	return (bListenerAlive == bSenderAlive);
}

void Broadcast(const char *sentence)
{
	char text[32];

	if (!sentence)
	{
		return;
	}

	Q_strcpy(text, "%!MRAD_");
	Q_strcat(text, UTIL_VarArgs("%s", sentence));

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgSendAudio);
		WRITE_BYTE(0);
		WRITE_STRING(text);
		WRITE_SHORT(100);
	MESSAGE_END();
}

const char * GetTeam(int teamNo)
{
	switch (teamNo)
	{
	case TERRORIST:
		return "TERRORIST";
	case CT:
		return "CT";
	case SPECTATOR:
		return "SPECTATOR";
	default:
		break;
	}

	return "";
}

void EndRoundMessage(const char *sentence, int event)
{
	CHalfLifeMultiplay *mp = g_pGameRules;
	const char *team = NULL;
	const char *message = &(sentence[1]);
	int teamTriggered = 1;

	UTIL_ClientPrintAll(HUD_PRINTCENTER, sentence);

	switch (event)
	{
	case ROUND_TARGET_BOMB:
	case ROUND_VIP_ASSASSINATED:
	case ROUND_TERRORISTS_ESCAPED:
	case ROUND_TERRORISTS_WIN:
	case ROUND_HOSTAGE_NOT_RESCUED:
	case ROUND_VIP_NOT_ESCAPED:
		team = GetTeam(TERRORIST);
		// tell bots the terrorists won the round
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_TERRORISTS_WIN);
		}
		break;
	case ROUND_VIP_ESCAPED:
	case ROUND_CTS_PREVENT_ESCAPE:
	case ROUND_ESCAPING_TERRORISTS_NEUTRALIZED:
	case ROUND_BOMB_DEFUSED:
	case ROUND_CTS_WIN:
	case ROUND_ALL_HOSTAGES_RESCUED:
	case ROUND_TARGET_SAVED:
	case ROUND_TERRORISTS_NOT_ESCAPED:
		team = GetTeam(CT);
		// tell bots the CTs won the round
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_CTS_WIN);
		}
		break;
	default:
		teamTriggered = false;
		// tell bots the round was a draw
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_ROUND_DRAW);
		}
		break;
	}

	if (mp != NULL)
	{
		if (teamTriggered)
		{
			UTIL_LogPrintf("Team \"%s\" triggered \"%s\" (CT \"%i\") (T \"%i\")\n", team, message, mp->m_iNumCTWins, mp->m_iNumTerroristWins);
		}
		else
		{
			UTIL_LogPrintf("World triggered \"%s\" (CT \"%i\") (T \"%i\")\n", message, mp->m_iNumCTWins, mp->m_iNumTerroristWins);
		}
	}

	UTIL_LogPrintf("World triggered \"Round_End\"\n");
}

void ReadMultiplayCvars(CHalfLifeMultiplay *mp)
{
	mp->m_iRoundTime = (int)(CVAR_GET_FLOAT("mp_roundtime") * 60);
	mp->m_iC4Timer = (int)CVAR_GET_FLOAT("mp_c4timer");
	mp->m_iIntroRoundTime = (int)CVAR_GET_FLOAT("mp_freezetime");
	mp->m_iLimitTeams = (int)CVAR_GET_FLOAT("mp_limitteams");


	if (mp->m_iRoundTime > 540)
	{
		CVAR_SET_FLOAT("mp_roundtime", 9);
		mp->m_iRoundTime = 540;
	}
	else if (mp->m_iRoundTime < 60)
	{
		CVAR_SET_FLOAT("mp_roundtime", 1);
		mp->m_iRoundTime = 60;
	}

	if (mp->m_iIntroRoundTime > 60)
	{
		CVAR_SET_FLOAT("mp_freezetime", 60);
		mp->m_iIntroRoundTime = 60;
	}
	else if (mp->m_iIntroRoundTime < 0)
	{
		CVAR_SET_FLOAT("mp_freezetime", 0);
		mp->m_iIntroRoundTime = 0;
	}

	if (mp->m_iC4Timer > 90)
	{
		CVAR_SET_FLOAT("mp_c4timer", 90);
		mp->m_iC4Timer = 90;
	}
	else if (mp->m_iC4Timer < 10)
	{
		CVAR_SET_FLOAT("mp_c4timer", 10);
		mp->m_iC4Timer = 10;
	}

	if (mp->m_iLimitTeams > 20)
	{
		CVAR_SET_FLOAT("mp_limitteams", 20);
		mp->m_iLimitTeams = 20;
	}
	else if (mp->m_iLimitTeams < 0)
	{
		CVAR_SET_FLOAT("mp_limitteams", 0);
		mp->m_iLimitTeams = 0;
	}
}

CHalfLifeMultiplay::CHalfLifeMultiplay()
{
	m_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);
	RefreshSkillData();

	m_flIntermissionEndTime = 0;
	m_flIntermissionStartTime = 0;
	m_fTeamCount = 0;

	m_iAccountCT = 0;
	m_iAccountTerrorist = 0;
	m_iHostagesRescued = 0;
	m_iRoundWinStatus = WINNER_NONE;
	m_iNumCTWins = 0;
	m_iNumTerroristWins = 0;
	m_pVIP = NULL;
	m_iNumCT = 0;
	m_iNumTerrorist = 0;
	m_iNumSpawnableCT = 0;
	m_iNumSpawnableTerrorist = 0;
	m_bMapHasCameras = MAP_HAS_CAMERAS_INIT;
	g_fGameOver = FALSE;

	m_iLoserBonus = REWARD_LOSER_BONUS_DEFAULT;
	m_iNumConsecutiveCTLoses = 0;
	m_iNumConsecutiveTerroristLoses = 0;
	m_iC4Guy = 0;
	m_bBombDefused = false;
	m_bTargetBombed = false;
	m_bFreezePeriod = TRUE;
	m_bLevelInitialized = false;
	m_tmNextPeriodicThink = 0;
	m_bFirstConnected = 0;
	m_bCompleteReset = false;
	m_flRequiredEscapeRatio = 0.5;
	m_iNumEscapers = 0;
	m_bCTCantBuy = false;
	m_bTCantBuy = false;
	m_flBombRadius = 500.0;
	m_iTotalGunCount = 0;
	m_iTotalGrenadeCount = 0;
	m_iTotalArmourCount = 0;
	m_iConsecutiveVIP = 0;
	m_iUnBalancedRounds = 0;
	m_iNumEscapeRounds = 0;
	m_bRoundTerminating = false;

	g_iHostageNumber = 0;
	m_bBombDropped = FALSE;

	m_iMaxRounds = (int)CVAR_GET_FLOAT("mp_maxrounds");

	if (m_iMaxRounds < 0)
	{
		m_iMaxRounds = 0;
		CVAR_SET_FLOAT("mp_maxrounds", 0);
	}

	m_iTotalRoundsPlayed = 0;
	m_iMaxRoundsWon = (int)CVAR_GET_FLOAT("mp_winlimit");

	if (m_iMaxRoundsWon < 0)
	{
		m_iMaxRoundsWon = 0;
		CVAR_SET_FLOAT("mp_winlimit", 0);
	}

	Q_memset(m_iMapVotes, 0, sizeof(m_iMapVotes));

	m_iLastPick = 1;
	m_bMapHasEscapeZone = false;
	m_iMapHasVIPSafetyZone = 0;
	m_bMapHasBombZone = false;
	m_bMapHasRescueZone = false;
	m_iStoredSpectValue = (int)allow_spectators.value;

	for (int j = 0; j < MAX_VIP_QUEUES; ++j)
	{
		VIPQueue[j] = NULL;
	}

	CVAR_SET_FLOAT("cl_himodels", 0);
	ReadMultiplayCvars(this);

	m_iIntroRoundTime += 2;
	m_fMaxIdlePeriod = m_iRoundTime * 2;

	float flAutoKickIdle = CVAR_GET_FLOAT("mp_autokick_timeout");

	if (flAutoKickIdle > 0.0)
	{
		m_fMaxIdlePeriod = flAutoKickIdle;
	}

	m_bInCareerGame = false;
	m_iRoundTimeSecs = m_iIntroRoundTime;

	if (IS_DEDICATED_SERVER())
	{
		CVAR_SET_FLOAT("pausable", 0);
	}
	else if (IsCareer())
	{
		CVAR_SET_FLOAT("pausable", 1);
		CVAR_SET_FLOAT("sv_aim", 0);
		CVAR_SET_FLOAT("sv_maxspeed", 322);
		CVAR_SET_FLOAT("sv_cheats", 0);
		CVAR_SET_FLOAT("mp_windifference", 2);

		m_bInCareerGame = true;
		UTIL_LogPrintf("Career Start\n");
	}
	else
	{
		CVAR_SET_FLOAT("pausable", 0);

		const char *lservercfgfile = CVAR_GET_STRING("lservercfgfile");

		if (lservercfgfile && *lservercfgfile != '\0')
		{
			ALERT(at_console, "Executing listen server config file\n");

			char szCommand[256];
			Q_sprintf(szCommand, "exec %s\n", lservercfgfile);
			SERVER_COMMAND(szCommand);
		}
	}

	m_fRoundCount = 0;
	m_fIntroRoundCount = 0;

#ifndef CSTRIKE
	InstallBotControl();
#endif // CSTRIKE

	InstallHostageManager();

	m_bSkipSpawn = m_bInCareerGame;

	static bool installedCommands = false;

	if (!installedCommands)
	{
		if (g_bIsCzeroGame)
		{
			ADD_SERVER_COMMAND("career_continue", SV_Continue_f);
			ADD_SERVER_COMMAND("career_matchlimit", SV_CareerMatchLimit_f);
			ADD_SERVER_COMMAND("career_add_task", SV_CareerAddTask_f);
			ADD_SERVER_COMMAND("career_endround", SV_Career_EndRound_f);
			ADD_SERVER_COMMAND("career_restart", SV_Career_Restart_f);
			ADD_SERVER_COMMAND("tutor_toggle", SV_Tutor_Toggle_f);
		}
		ADD_SERVER_COMMAND("perf_test", loopPerformance);
		ADD_SERVER_COMMAND("print_ent", printEntities);

		installedCommands = true;
	}

	m_fCareerRoundMenuTime = 0;
	m_fCareerMatchMenuTime = 0;
	m_iCareerMatchWins = 0;

	m_iRoundWinDifference = (int)CVAR_GET_FLOAT("mp_windifference");
	CCareerTaskManager::Create();

	if (m_iRoundWinDifference < 1)
	{
		m_iRoundWinDifference = 1;
		CVAR_SET_FLOAT("mp_windifference", 1);
	}

	sv_clienttrace = CVAR_GET_POINTER("sv_clienttrace");

	if (g_bIsCzeroGame)
		InstallTutor(CVAR_GET_FLOAT("tutor_enable") != 0.0f);

	g_pMPGameRules = this;
}

void CHalfLifeMultiplay::RefreshSkillData()
{
	CGameRules::RefreshSkillData();

	gSkillData.plrDmg9MM = 12;
	gSkillData.plrDmgMP5 = 12;
	gSkillData.suitchargerCapacity = 30;
	gSkillData.plrDmg357 = 40;
	gSkillData.plrDmgM203Grenade = 100;
	gSkillData.plrDmgBuckshot = 20;
	gSkillData.plrDmgCrossbowClient = 20;
	gSkillData.plrDmgRPG = 120;
}

void CHalfLifeMultiplay::RemoveGuns()
{
	CBaseEntity *toremove = NULL;

	while ((toremove = UTIL_FindEntityByClassname(toremove, "weaponbox")) != NULL)
		((CWeaponBox *)toremove)->Kill();

	toremove = NULL;

	while ((toremove = UTIL_FindEntityByClassname(toremove, "weapon_shield")) != NULL)
	{
		toremove->SetThink(&CBaseEntity::SUB_Remove);
		toremove->pev->nextthink = gpGlobals->time + 0.1;
	}
}

void CHalfLifeMultiplay::UpdateTeamScores()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore);
		WRITE_STRING("CT");
		WRITE_SHORT(m_iNumCTWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore);
		WRITE_STRING("TERRORIST");
		WRITE_SHORT(m_iNumTerroristWins);
	MESSAGE_END();
}

void CHalfLifeMultiplay::CleanUpMap()
{
	// Recreate all the map entities from the map data (preserving their indices),
	// then remove everything else except the players.

	CBaseEntity *torestart = NULL;
	CBaseEntity *toremove = NULL;

	torestart = UTIL_FindEntityByClassname(NULL, "cycler_sprite");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "cycler_sprite");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "light");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "light");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_breakable");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_breakable");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_door");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_door");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_water");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_water");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_door_rotating");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_door_rotating");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_tracktrain");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_tracktrain");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_vehicle");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_vehicle");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "func_train");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "func_train");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "armoury_entity");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "armoury_entity");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "ambient_generic");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "ambient_generic");
	}

	torestart = UTIL_FindEntityByClassname(NULL, "env_sprite");
	while (torestart != NULL)
	{
		torestart->Restart();
		torestart = UTIL_FindEntityByClassname(torestart, "env_sprite");
	}

	// Remove grenades and C4
	int icount = 0;
	toremove = UTIL_FindEntityByClassname(NULL, "grenade");
	while (toremove != NULL && icount < 20)
	{
		UTIL_Remove(toremove);
		toremove = UTIL_FindEntityByClassname(toremove, "grenade");
		++icount;
	}

	// Remove defuse kit
	// Old code only removed 4 kits and stopped.
	toremove = UTIL_FindEntityByClassname(NULL, "item_thighpack");
	while (toremove != NULL)
	{
		UTIL_Remove(toremove);
		toremove = UTIL_FindEntityByClassname(toremove, "item_thighpack");
	}

	RemoveGuns();
	PLAYBACK_EVENT((FEV_GLOBAL | FEV_RELIABLE), 0, m_usResetDecals);
}

void CHalfLifeMultiplay::GiveC4()
{
	int iTeamCount;
	int iTemp = 0;
	int humansPresent = 0;

	iTeamCount = m_iNumTerrorist;
	++m_iC4Guy;

	bool giveToHumans = (cv_bot_defer_to_human.value > 0.0);

	if (giveToHumans)
	{
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

			if (!player || FNullEnt(player->edict()))
				continue;

			if (player->pev->deadflag != DEAD_NO || player->m_iTeam != TERRORIST)
				continue;

			if (!player->IsBot())
				++humansPresent;
		}

		if (humansPresent)
			iTeamCount = humansPresent;
		else
			giveToHumans = false;
	}

	// if we've looped past the last Terrorist.. then give the C4 to the first one.
	if (m_iC4Guy > iTeamCount)
	{
		m_iC4Guy = 1;
	}

	// Give the C4 to the specified T player..
	CBaseEntity *pPlayer = NULL;
	while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (FNullEnt(pPlayer->edict()))
			break;

		if (!pPlayer->IsPlayer())
			continue;

		if (pPlayer->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);

		if (player->pev->deadflag != DEAD_NO || player->m_iTeam != TERRORIST || (giveToHumans && player->IsBot()))
			continue;

		if (++iTemp == m_iC4Guy)
		{
			player->m_bHasC4 = true;
			player->GiveNamedItem("weapon_c4");
			player->SetBombIcon();
			player->pev->body = 1;

			player->m_flDisplayHistory |= DHF_BOMB_RETRIEVED;
			player->HintMessage("#Hint_you_have_the_bomb", FALSE, TRUE);

			// Log this information
			UTIL_LogPrintf("\"%s<%i><%s><TERRORIST>\" triggered \"Spawned_With_The_Bomb\"\n", STRING(player->pev->netname), GETPLAYERUSERID(player->edict()), GETPLAYERAUTHID(player->edict()));
			m_bBombDropped = FALSE;
		}
	}

	// if there are no players with a bomb
	if (!IsThereABomber())
	{
		m_iC4Guy = 0;
		pPlayer = NULL;

		while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
		{
			if (FNullEnt(pPlayer->edict()))
				break;

			if (!pPlayer->IsPlayer())
				continue;

			if (pPlayer->pev->flags == FL_DORMANT)
				continue;

			CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);

			if (player->pev->deadflag != DEAD_NO || player->m_iTeam != TERRORIST)
				continue;

			player->m_bHasC4 = true;
			player->GiveNamedItem("weapon_c4");
			player->SetBombIcon(FALSE);
			player->pev->body = 1;

			player->m_flDisplayHistory |= DHF_BOMB_RETRIEVED;
			player->HintMessage("#Hint_you_have_the_bomb", FALSE, TRUE);

			// Log this information
			UTIL_LogPrintf("\"%s<%i><%s><TERRORIST>\" triggered \"Spawned_With_The_Bomb\"\n", STRING(player->pev->netname), GETPLAYERUSERID(player->edict()), GETPLAYERAUTHID(player->edict()));
			m_bBombDropped = FALSE;
			return;
		}
	}
}

void CHalfLifeMultiplay::TerminateRound(float tmDelay, int iWinStatus)
{
	m_iRoundWinStatus = iWinStatus;
	m_fTeamCount = gpGlobals->time + tmDelay;
	m_bRoundTerminating = true;
}

void CHalfLifeMultiplay::QueueCareerRoundEndMenu(float tmDelay, int iWinStatus)
{
	if (TheCareerTasks == NULL)
		return;

	if (m_fCareerMatchMenuTime != 0.0f)
		return;

	m_fCareerRoundMenuTime = tmDelay + gpGlobals->time;
	bool humansAreCTs = (Q_strcmp(humans_join_team.string, "CT") == 0);

	if (humansAreCTs)
	{
		CBaseEntity *hostage = NULL;

		int numHostagesInMap = 0;
		int numHostagesFollowingHumans = 0;
		int numHostagesAlive = 0;

		while ((hostage = UTIL_FindEntityByClassname(hostage, "hostage_entity")) != NULL)
		{
			++numHostagesInMap;

			CHostage *pHostage = static_cast<CHostage *>(hostage);

			if (pHostage->pev->takedamage != DAMAGE_YES)
			{
				continue;
			}

			CBasePlayer *pLeader = NULL;

			if (pHostage->IsFollowingSomeone())
				pLeader = static_cast<CBasePlayer *>(pHostage->GetLeader());

			if (pLeader == NULL)
			{
				++numHostagesAlive;
			}
			else
			{
				if (!pLeader->IsBot())
				{
					++numHostagesFollowingHumans;
					TheCareerTasks->HandleEvent(EVENT_HOSTAGE_RESCUED, pLeader, 0);
				}
			}
		}

		if (!numHostagesAlive)
		{
			if ((numHostagesInMap * 0.5) <= (numHostagesFollowingHumans + m_iHostagesRescued))
			{
				TheCareerTasks->HandleEvent(EVENT_ALL_HOSTAGES_RESCUED);
			}
		}
	}

	switch (iWinStatus)
	{
	case WINSTATUS_CTS:
		TheCareerTasks->HandleEvent(humansAreCTs ? EVENT_ROUND_WIN : EVENT_ROUND_LOSS);
		break;
	case WINSTATUS_TERRORISTS:
		TheCareerTasks->HandleEvent(humansAreCTs ? EVENT_ROUND_LOSS : EVENT_ROUND_WIN);
		break;
	default:
		TheCareerTasks->HandleEvent(EVENT_ROUND_DRAW);
		break;
	}

	if (m_fCareerMatchMenuTime == 0.0f && m_iCareerMatchWins)
	{
		bool canTsWin = true;
		bool canCTsWin = true;

		if (m_iNumCTWins < m_iCareerMatchWins || (m_iNumCTWins - m_iNumTerroristWins < m_iRoundWinDifference))
			canCTsWin = false;

		if (m_iNumTerroristWins < m_iCareerMatchWins || (m_iNumTerroristWins - m_iNumCTWins < m_iRoundWinDifference))
			canTsWin = false;

		if (!TheCareerTasks->AreAllTasksComplete())
		{
			if (humansAreCTs)
				return;

			canTsWin = false;
		}

		if (canCTsWin || canTsWin)
		{
			m_fCareerRoundMenuTime = 0;
			m_fCareerMatchMenuTime = gpGlobals->time + 3.0f;
		}
	}
}

// Check if the scenario has been won/lost.

void CHalfLifeMultiplay::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	// the icc compiler will cut out all of the code which refers to it
	int scenarioFlags = 0;

	// Initialize the player counts..
	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	// other player's check
	bool bNeededPlayers = false;
	if (!(scenarioFlags & SCENARIO_BLOCK_NEED_PLAYERS) && NeededPlayersCheck(bNeededPlayers))
		return;

	// Assasination/VIP scenarion check
	if (!(scenarioFlags & SCENARIO_BLOCK_VIP_ESCAPRE) && VIPRoundEndCheck(bNeededPlayers))
		return;

	// Prison escape check
	if (!(scenarioFlags & SCENARIO_BLOCK_PRISON_ESCAPRE) && PrisonRoundEndCheck(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT, bNeededPlayers))
		return;

	// Bomb check
	if (!(scenarioFlags & SCENARIO_BLOCK_BOMB) && BombRoundEndCheck(bNeededPlayers))
		return;

	// Team Extermination check
	// CounterTerrorists won by virture of elimination
	if (!(scenarioFlags & SCENARIO_BLOCK_TEAM_EXTERMINATION) && TeamExterminationCheck(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT, bNeededPlayers))
		return;

	// Hostage rescue check
	if (!(scenarioFlags & SCENARIO_BLOCK_HOSTAGE_RESCUE) && HostageRescueRoundEndCheck(bNeededPlayers))
		return;

	// scenario not won - still in progress
}

void CHalfLifeMultiplay::InitializePlayerCounts(int &NumAliveTerrorist, int &NumAliveCT, int &NumDeadTerrorist, int &NumDeadCT)
{
	NumAliveTerrorist = NumAliveCT = NumDeadCT = NumDeadTerrorist = 0;
	m_iNumTerrorist = m_iNumCT = m_iNumSpawnableTerrorist = m_iNumSpawnableCT = 0;
	m_iHaveEscaped = 0;

	// initialize count dead/alive players

	// Count how many dead players there are on each team.
	CBaseEntity *pPlayer = NULL;
	while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (FNullEnt(pPlayer->edict()))
		{
			break;
		}

		CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);

		if (pPlayer->pev->flags == FL_DORMANT)
		{
			continue;
		}

		// TODO: check it out, for what here used player->IsBot() ?
		// maybe body this conditions is located under the wrapper #ifdef 0
		// if (player->IsBot())
		// {
		//	#ifdef 0
		//		....
		//	#endif
		// }

		switch (player->m_iTeam)
		{
		case CT:
		{
			++m_iNumCT;

			if (player->m_iMenu != Menu_ChooseAppearance)
			{
				++m_iNumSpawnableCT;
				//player->IsBot();
			}

			//player->IsBot();
			if (player->pev->deadflag != DEAD_NO)
				++NumDeadCT;
			else
				++NumAliveCT;

			break;
		}
		case TERRORIST:
		{
			++m_iNumTerrorist;

			if (player->m_iMenu != Menu_ChooseAppearance)
			{
				++m_iNumSpawnableTerrorist;
				//player->IsBot();
			}
			//player->IsBot();

			if (player->pev->deadflag != DEAD_NO)
				++NumDeadTerrorist;
			else
				++NumAliveTerrorist;

			// Check to see if this guy escaped.
			if (player->m_bEscaped)
				++m_iHaveEscaped;

			break;
		}
		default:
			break;
		}
	}
}

bool CHalfLifeMultiplay::NeededPlayersCheck(bool &bNeededPlayers)
{
	// We needed players to start scoring
	// Do we have them now?
	// start the game, after the players entered in game
	if (!m_iNumSpawnableTerrorist || !m_iNumSpawnableCT)
	{
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "#Game_scoring");
		bNeededPlayers = true;
		m_bFirstConnected = false;
	}

	if (!m_bFirstConnected && m_iNumSpawnableTerrorist != 0 && m_iNumSpawnableCT != 0)
	{
		if (IsCareer())
		{
			CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(gpGlobals->maxClients));

			if (!player || !player->IsBot())
			{
				return true;
			}
		}

		// Start the round immediately when the first person joins
		UTIL_LogPrintf("World triggered \"Game_Commencing\"\n");

		// Make sure we are not on the FreezePeriod.
		m_bFreezePeriod = FALSE;
		m_bCompleteReset = true;

		EndRoundMessage("#Game_Commencing", ROUND_END_DRAW);
		TerminateRound(IsCareer() ? 0 : 3, WINSTATUS_DRAW);

		m_bFirstConnected = true;
		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_GAME_COMMENCE);
		}
		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::VIPRoundEndCheck(bool bNeededPlayers)
{
	// checks to scenario Escaped VIP on map with vip safety zones
	if (m_iMapHasVIPSafetyZone == MAP_HAVE_VIP_SAFETYZONE_YES && m_pVIP != NULL)
	{
		if (m_pVIP->m_bEscaped)
		{
			Broadcast("ctwin");
			m_iAccountCT += REWARD_VIP_ESCAPED;

			if (!bNeededPlayers)
			{
				++m_iNumCTWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
				WRITE_BYTE(9);	// command length in bytes
				WRITE_BYTE(DRC_CMD_EVENT);	// VIP rescued
				WRITE_SHORT(ENTINDEX(m_pVIP->edict()));	// index number of primary entity
				WRITE_SHORT(0);	// index number of secondary entity
				WRITE_LONG(15 | DRC_FLAG_FINAL);	// eventflags (priority and flags)
			MESSAGE_END();

			EndRoundMessage("#VIP_Escaped", ROUND_VIP_ESCAPED);

			// tell the bots the VIP got out
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_VIP_ESCAPED);
			}
			TerminateRound(5, WINSTATUS_CTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
			}

			return true;
		}
		// The VIP is dead
		else if (m_pVIP->pev->deadflag != DEAD_NO)
		{
			Broadcast("terwin");
			m_iAccountTerrorist += REWARD_VIP_ASSASSINATED;

			if (!bNeededPlayers)
			{
				++m_iNumTerroristWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#VIP_Assassinated", ROUND_VIP_ASSASSINATED);

			// tell the bots the VIP was killed
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_VIP_ASSASSINATED);
			}
			TerminateRound(5, WINSTATUS_TERRORISTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
			}

			return true;
		}
	}

	return false;
}

bool CHalfLifeMultiplay::PrisonRoundEndCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, bool bNeededPlayers)
{
	// checks to scenario Escaped Terrorist's
	if (m_bMapHasEscapeZone)
	{
		float flEscapeRatio = (float)m_iHaveEscaped / (float)m_iNumEscapers;

		if (flEscapeRatio >= m_flRequiredEscapeRatio)
		{
			Broadcast("terwin");
			m_iAccountTerrorist += REWARD_TERRORISTS_ESCAPED;

			if (!bNeededPlayers)
			{
				++m_iNumTerroristWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#Terrorists_Escaped", ROUND_TERRORISTS_ESCAPED);
			TerminateRound(5, WINSTATUS_TERRORISTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
			}

			return true;
		}
		else if (NumAliveTerrorist == 0 && flEscapeRatio < m_flRequiredEscapeRatio)
		{
			Broadcast("ctwin");

			// CTs are rewarded based on how many terrorists have escaped...
			m_iAccountCT += (1 - flEscapeRatio) * REWARD_CTS_PREVENT_ESCAPE;

			if (!bNeededPlayers)
			{
				++m_iNumCTWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#CTs_PreventEscape", ROUND_CTS_PREVENT_ESCAPE);
			TerminateRound(5, WINSTATUS_CTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
			}

			return true;
		}
		else if (NumAliveTerrorist == 0 && NumDeadTerrorist != 0 && m_iNumSpawnableCT > 0)
		{
			Broadcast("ctwin");

			// CTs are rewarded based on how many terrorists have escaped...
			m_iAccountCT += (1 - flEscapeRatio) * REWARD_ESCAPING_TERRORISTS_NEUTRALIZED;

			if (!bNeededPlayers)
			{
				++m_iNumCTWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#Escaping_Terrorists_Neutralized", ROUND_ESCAPING_TERRORISTS_NEUTRALIZED);
			TerminateRound(5, WINSTATUS_CTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
			}

			return true;
		}
		// else return true;
	}

	return false;
}

bool CHalfLifeMultiplay::BombRoundEndCheck(bool bNeededPlayers)
{
	// Check to see if the bomb target was hit or the bomb defused.. if so, then let's end the round!
	if (m_bTargetBombed && m_bMapHasBombTarget)
	{
		Broadcast("terwin");
		m_iAccountTerrorist += REWARD_TARGET_BOMB;

		if (!bNeededPlayers)
		{
			++m_iNumTerroristWins;
			// Update the clients team score
			UpdateTeamScores();
		}

		EndRoundMessage("#Target_Bombed", ROUND_TARGET_BOMB);
		TerminateRound(5, WINSTATUS_TERRORISTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
		}

		return true;
	}
	else if (m_bBombDefused && m_bMapHasBombTarget)
	{
		Broadcast("ctwin");
		m_iAccountCT += REWARD_BOMB_DEFUSED;
		m_iAccountTerrorist += REWARD_BOMB_PLANTED;

		if (!bNeededPlayers)
		{
			++m_iNumCTWins;
			// Update the clients team score
			UpdateTeamScores();
		}

		EndRoundMessage("#Bomb_Defused", ROUND_BOMB_DEFUSED);
		TerminateRound(5, WINSTATUS_CTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
		}

		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::TeamExterminationCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, bool bNeededPlayers)
{
	if ((m_iNumCT > 0 && m_iNumSpawnableCT > 0) && (m_iNumTerrorist > 0 && m_iNumSpawnableTerrorist > 0))
	{
		if (NumAliveTerrorist == 0 && NumDeadTerrorist != 0 && NumAliveCT > 0)
		{
			CBaseEntity *temp = NULL;
			bool nowin = false;

			while ((temp = UTIL_FindEntityByClassname(temp, "grenade")) != NULL)
			{
				CGrenade *C4 = static_cast<CGrenade *>(temp);

				if (C4->m_bIsC4 && !C4->m_bJustBlew)
				{
					nowin = true;
					break;
				}
			}

			if (!nowin)
			{
				Broadcast("ctwin");
				m_iAccountCT += m_bMapHasBombTarget ? REWARD_BOMB_DEFUSED : REWARD_CTS_WIN;

				if (!bNeededPlayers)
				{
					++m_iNumCTWins;
					// Update the clients team score
					UpdateTeamScores();
				}

				EndRoundMessage("#CTs_Win", ROUND_CTS_WIN);
				TerminateRound(5, WINSTATUS_CTS);

				if (IsCareer())
				{
					QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
				}

				return true;
			}
		}

		// Terrorists WON
		else if (NumAliveCT == 0 && NumDeadCT != 0)
		{
			Broadcast("terwin");
			m_iAccountTerrorist += m_bMapHasBombTarget ? REWARD_BOMB_EXPLODED : REWARD_TERRORISTS_WIN;

			if (!bNeededPlayers)
			{
				++m_iNumTerroristWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#Terrorists_Win", ROUND_TERRORISTS_WIN);
			TerminateRound(5, WINSTATUS_TERRORISTS);

			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
			}

			return true;
		}
	}
	else if (NumAliveCT == 0 && NumAliveTerrorist == 0)
	{
		EndRoundMessage("#Round_Draw", ROUND_END_DRAW);
		Broadcast("rounddraw");
		TerminateRound(5, WINSTATUS_DRAW);

		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::HostageRescueRoundEndCheck(bool bNeededPlayers)
{
	// Check to see if 50% of the hostages have been rescued.
	CBaseEntity *hostage = NULL;
	int iHostages = 0;

	// Assume that all hostages are either rescued or dead..
	bool bHostageAlive = false;

	while ((hostage = UTIL_FindEntityByClassname(hostage, "hostage_entity")) != NULL)
	{
		++iHostages;

		// We've found a live hostage. don't end the round
		if (hostage->pev->takedamage == DAMAGE_YES)
		{
			bHostageAlive = true;
		}
	}

	// There are no hostages alive.. check to see if the CTs have rescued atleast 50% of them.
	if (!bHostageAlive && iHostages > 0)
	{
		if (m_iHostagesRescued >= (iHostages * 0.5))
		{
			Broadcast("ctwin");
			m_iAccountCT += REWARD_ALL_HOSTAGES_RESCUED;

			if (!bNeededPlayers)
			{
				++m_iNumCTWins;
				// Update the clients team score
				UpdateTeamScores();
			}

			EndRoundMessage("#All_Hostages_Rescued", ROUND_ALL_HOSTAGES_RESCUED);

			// tell the bots all the hostages have been rescued
			if (TheBots != NULL)
			{
				TheBots->OnEvent(EVENT_ALL_HOSTAGES_RESCUED);
			}

			if (IsCareer())
			{
				if (TheCareerTasks != NULL)
				{
					TheCareerTasks->HandleEvent(EVENT_ALL_HOSTAGES_RESCUED);
				}
			}

			TerminateRound(5, WINSTATUS_CTS);
			if (IsCareer())
			{
				QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
			}

			return true;
		}
	}

	return false;
}

void CHalfLifeMultiplay::SwapAllPlayers()
{
	CBaseEntity *pPlayer = NULL;

	while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (FNullEnt(pPlayer->edict()))
			break;

		if (pPlayer->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);
		player->SwitchTeam();
	}

	// Swap Team victories
	int iTemp;

	iTemp = m_iNumTerroristWins;
	m_iNumTerroristWins = m_iNumCTWins;
	m_iNumCTWins = iTemp;

	// Update the clients team score
	UpdateTeamScores();
}

void CHalfLifeMultiplay::BalanceTeams()
{
	int iTeamToSwap = UNASSIGNED;
	int iNumToSwap;

	// The ratio for teams is different for Assasination maps
	if (m_iMapHasVIPSafetyZone == MAP_HAVE_VIP_SAFETYZONE_YES)
	{
		int iDesiredNumCT, iDesiredNumTerrorist;

		// uneven number of players
		if ((m_iNumCT + m_iNumTerrorist) % 2 != 0)
			iDesiredNumCT = (int)((m_iNumCT + m_iNumTerrorist) * 0.55f) + 1;
		else
			iDesiredNumCT = (int)((m_iNumCT + m_iNumTerrorist) / 2);

		iDesiredNumTerrorist = (m_iNumCT + m_iNumTerrorist) - iDesiredNumCT;

		if (m_iNumCT < iDesiredNumCT)
		{
			iTeamToSwap = TERRORIST;
			iNumToSwap = iDesiredNumCT - m_iNumCT;
		}
		else if (m_iNumTerrorist < iDesiredNumTerrorist)
		{
			iTeamToSwap = CT;
			iNumToSwap = iDesiredNumTerrorist - m_iNumTerrorist;
		}
		else
		{
			return;
		}
	}
	else
	{
		if (m_iNumCT > m_iNumTerrorist)
		{
			iTeamToSwap = CT;
			iNumToSwap = (m_iNumCT - m_iNumTerrorist) / 2;
		}
		else if (m_iNumTerrorist > m_iNumCT)
		{
			iTeamToSwap = TERRORIST;
			iNumToSwap = (m_iNumTerrorist - m_iNumCT) / 2;
		}
		else
		{
			// Teams are even.. Get out of here.
			return;
		}
	}

	// Don't swap more than 4 players at a time.. This is a naive method of avoiding infinite loops.
	if (iNumToSwap > 4)
		iNumToSwap = 4;

	// last person to join the server
	int iHighestUserID = 0;
	CBasePlayer *toSwap = NULL;

	for (int i = 1; i <= iNumToSwap; ++i)
	{
		iHighestUserID = 0;
		toSwap = NULL;

		CBaseEntity *pPlayer = NULL;
		while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
		{
			if (FNullEnt(pPlayer->edict()))
				break;

			if (pPlayer->pev->flags == FL_DORMANT)
				continue;

			CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);

			if (player->m_iTeam == iTeamToSwap && GETPLAYERUSERID(player->edict()) > iHighestUserID && m_pVIP != player)
			{
				iHighestUserID = GETPLAYERUSERID(player->edict());
				toSwap = player;
			}
		}

		if (toSwap != NULL)
		{
			toSwap->SwitchTeam();
		}
	}
}

void CHalfLifeMultiplay::CheckMapConditions()
{
	// Check to see if this map has a bomb target in it
	if (UTIL_FindEntityByClassname(NULL, "func_bomb_target"))
	{
		m_bMapHasBombTarget = true;
		m_bMapHasBombZone = true;
	}
	else if (UTIL_FindEntityByClassname(NULL, "info_bomb_target"))
	{
		m_bMapHasBombTarget = true;
		m_bMapHasBombZone = false;
	}
	else
	{
		m_bMapHasBombTarget = false;
		m_bMapHasBombZone = false;
	}

	// Check to see if this map has hostage rescue zones
	m_bMapHasRescueZone = (UTIL_FindEntityByClassname(NULL, "func_hostage_rescue") != NULL);

	// See if the map has func_buyzone entities
	// Used by CBasePlayer::HandleSignals() to support maps without these entities
	m_bMapHasBuyZone = (UTIL_FindEntityByClassname(NULL, "func_buyzone") != NULL);

	// GOOSEMAN : See if this map has func_escapezone entities
	m_bMapHasEscapeZone = (UTIL_FindEntityByClassname(NULL, "func_escapezone") != NULL);

	// Check to see if this map has VIP safety zones
	if ((UTIL_FindEntityByClassname(NULL, "func_vip_safetyzone")) != NULL)
		m_iMapHasVIPSafetyZone = MAP_HAVE_VIP_SAFETYZONE_YES;
	else
		m_iMapHasVIPSafetyZone = MAP_HAVE_VIP_SAFETYZONE_NO;
}

void CHalfLifeMultiplay::RestartRound()
{
	// tell bots that the round is restarting
	if (TheBots != NULL)
	{
		TheBots->RestartRound();
	}

	if (g_pHostages != NULL)
	{
		g_pHostages->RestartRound();
	}

	++m_iTotalRoundsPlayed;
	ClearBodyQue();

	// Hardlock the player accelaration to 5.0
	CVAR_SET_FLOAT("sv_accelerate", 5.0);
	CVAR_SET_FLOAT("sv_friction", 4.0);
	CVAR_SET_FLOAT("sv_stopspeed", 75);

	// Tabulate the number of players on each team.
	m_iNumCT = CountTeamPlayers(CT);
	m_iNumTerrorist = CountTeamPlayers(TERRORIST);

	// reset the dropped bomb on everyone's radar
	if (m_bMapHasBombTarget)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgBombPickup);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgShowTimer);
		MESSAGE_END();
	}

	m_bBombDropped = FALSE;

	// reset all players health for HLTV
	MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
		WRITE_BYTE(0);				// 0 = all players
		WRITE_BYTE(100 | DRC_FLAG_FACEPLAYER);	// 100 health + msg flag
	MESSAGE_END();

	// reset all players FOV for HLTV
	MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
		WRITE_BYTE(0);		// all players
		WRITE_BYTE(0);		// to default FOV value
	MESSAGE_END();

	if (CVAR_GET_FLOAT("mp_autoteambalance") != 0.0f && m_iUnBalancedRounds >= 1)
	{
		BalanceTeams();
	}

	if ((m_iNumCT - m_iNumTerrorist) >= 2 || (m_iNumTerrorist - m_iNumCT) >= 2)
	{
		++m_iUnBalancedRounds;
	}
	else
		m_iUnBalancedRounds = 0;

	// Warn the players of an impending auto-balance next round...
	if (CVAR_GET_FLOAT("mp_autoteambalance") != 0.0f && m_iUnBalancedRounds == 1)
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Auto_Team_Balance_Next_Round");
	}

	if (m_bCompleteReset)
	{
		// bounds check
		if (timelimit.value < 0)
		{
			CVAR_SET_FLOAT("mp_timelimit", 0);
		}

		g_flResetTime = gpGlobals->time;

		// Reset timelimit
		if (timelimit.value)
			g_flTimeLimit = gpGlobals->time + (timelimit.value * 60);

		// Reset total # of rounds played
		m_iTotalRoundsPlayed = 0;
		m_iMaxRounds = (int)CVAR_GET_FLOAT("mp_maxrounds");

		if (m_iMaxRounds < 0)
		{
			m_iMaxRounds = 0;
			CVAR_SET_FLOAT("mp_maxrounds", 0);
		}

		m_iMaxRoundsWon = (int)CVAR_GET_FLOAT("mp_winlimit");

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}

		// Reset score info
		m_iNumTerroristWins = 0;
		m_iNumCTWins = 0;
		m_iNumConsecutiveTerroristLoses = 0;
		m_iNumConsecutiveCTLoses = 0;

		// Reset team scores
		UpdateTeamScores();

		// Reset the player stats
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			CBasePlayer *plr = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

			if (plr && !FNullEnt(plr->pev))
				plr->Reset();
		}

		if (TheBots != NULL)
		{
			TheBots->OnEvent(EVENT_NEW_MATCH);
		}
	}

	m_bFreezePeriod = TRUE;
	m_bRoundTerminating = false;

	ReadMultiplayCvars(this);

	float flAutoKickIdle = CVAR_GET_FLOAT("mp_autokick_timeout");

	// set the idlekick max time (in seconds)
	if (flAutoKickIdle > 0)
		m_fMaxIdlePeriod = flAutoKickIdle;
	else
		m_fMaxIdlePeriod = (m_iRoundTime * 2);

	// This makes the round timer function as the intro timer on the client side
	m_iRoundTimeSecs = m_iIntroRoundTime;

	// Check to see if there's a mapping info paramater entity
	CMapInfo *mi = (CMapInfo *)UTIL_FindEntityByClassname(NULL, "info_map_parameters");
	if (mi != NULL)
	{
		switch (mi->m_iBuyingStatus)
		{
		case BUYING_EVERYONE:
			m_bCTCantBuy = false;
			m_bTCantBuy = false;

			ALERT(at_console, "EVERYONE CAN BUY!\n");
			break;
		case BUYING_ONLY_CTS:
			m_bCTCantBuy = false;
			m_bTCantBuy = true;

			ALERT(at_console, "Only CT's can buy!!\n");
			break;
		case BUYING_ONLY_TERRORISTS:
			m_bCTCantBuy = true;
			m_bTCantBuy = false;

			ALERT(at_console, "Only T's can buy!!\n");
			break;
		case BUYING_NO_ONE:
			m_bCTCantBuy = true;
			m_bTCantBuy = true;

			ALERT(at_console, "No one can buy!!\n");
			break;
		default:
			m_bCTCantBuy = false;
			m_bTCantBuy = false;
			break;
		}

		m_flBombRadius = mi->m_flBombRadius;
	}

	CheckMapConditions();

	if (m_bMapHasEscapeZone)
	{
		// Will increase this later when we count how many Ts are starting
		m_iNumEscapers = m_iHaveEscaped = 0;

		if (m_iNumEscapeRounds >= 3)
		{
			SwapAllPlayers();
			m_iNumEscapeRounds = 0;
		}

		// Increment the number of rounds played... After 8 rounds, the players will do a whole sale switch..
		++m_iNumEscapeRounds;
	}

	if (m_iMapHasVIPSafetyZone == MAP_HAVE_VIP_SAFETYZONE_YES)
	{
		PickNextVIP();
		++m_iConsecutiveVIP;
	}

	int acct_tmp = 0;
	CBaseEntity *hostage = NULL;

	while ((hostage = UTIL_FindEntityByClassname(hostage, "hostage_entity")) != NULL)
	{
		if (acct_tmp >= 2000)
			break;

		CHostage *temp = static_cast<CHostage *>(hostage);

		if (hostage->pev->solid != SOLID_NOT)
		{
			acct_tmp += 150;

			if (hostage->pev->deadflag == DEAD_DEAD)
			{
				hostage->pev->deadflag = DEAD_RESPAWNABLE;
			}
		}

		temp->RePosition();
	}

	// Scale up the loser bonus when teams fall into losing streaks
	if (m_iRoundWinStatus == WINSTATUS_TERRORISTS)	// terrorists won
	{
		// check to see if they just broke a losing streak
		if (m_iNumConsecutiveTerroristLoses > 1)
		{
			// this is the default losing bonus
			m_iLoserBonus = REWARD_LOSER_BONUS_MIN;
		}

		m_iNumConsecutiveTerroristLoses = 0;	// starting fresh
		m_iNumConsecutiveCTLoses++;		// increment the number of wins the CTs have had
	}
	else if (m_iRoundWinStatus == WINSTATUS_CTS)
	{
		// check to see if they just broke a losing streak
		if (m_iNumConsecutiveCTLoses > 1)
		{
			// this is the default losing bonus
			m_iLoserBonus = REWARD_LOSER_BONUS_MIN;
		}

		m_iNumConsecutiveCTLoses = 0;		// starting fresh
		m_iNumConsecutiveTerroristLoses++;	// increment the number of wins the Terrorists have had
	}

	// check if the losing team is in a losing streak & that the loser bonus hasen't maxed out.
	if (m_iNumConsecutiveTerroristLoses > 1 && m_iLoserBonus < REWARD_LOSER_BONUS_MAX)
	{
		// help out the team in the losing streak
		m_iLoserBonus += REWARD_LOSER_BONUS_ADD;
	}
	else if (m_iNumConsecutiveCTLoses > 1 && m_iLoserBonus < REWARD_LOSER_BONUS_MAX)
	{
		// help out the team in the losing streak
		m_iLoserBonus += REWARD_LOSER_BONUS_ADD;
	}

	// assign the wining and losing bonuses
	if (m_iRoundWinStatus == WINSTATUS_TERRORISTS)	// terrorists won
	{
		m_iAccountTerrorist += acct_tmp;
		m_iAccountCT += m_iLoserBonus;
	}
	else if (m_iRoundWinStatus == WINSTATUS_CTS)	// CT Won
	{
		m_iAccountCT += acct_tmp;

		if (!m_bMapHasEscapeZone)
		{
			// only give them the bonus if this isn't an escape map
			m_iAccountTerrorist += m_iLoserBonus;
		}
	}

	// Update CT account based on number of hostages rescued
	m_iAccountCT += m_iHostagesRescued * REWARD_RESCUED_HOSTAGE;

	// Update individual players accounts and respawn players

	// the round time stamp must be set before players are spawned
	m_fIntroRoundCount = m_fRoundCount = gpGlobals->time;

	// Adrian - No cash for anyone at first rounds! ( well, only the default. )
	if (m_bCompleteReset)
	{
		// No extra cash!.
		m_iAccountTerrorist = m_iAccountCT = 0;

		// We are starting fresh. So it's like no one has ever won or lost.
		m_iNumTerroristWins = 0;
		m_iNumCTWins = 0;
		m_iNumConsecutiveTerroristLoses = 0;
		m_iNumConsecutiveCTLoses = 0;
		m_iLoserBonus = REWARD_LOSER_BONUS_DEFAULT;
	}

	// tell bots that the round is restarting
	CBaseEntity *pPlayer = NULL;
	while ((pPlayer = UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (FNullEnt(pPlayer->edict()))
			break;

		if (pPlayer->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);

		player->m_iNumSpawns = 0;
		player->m_bTeamChanged = false;

		if (!player->IsPlayer())
		{
			player->SyncRoundTimer();
		}

		if (player->m_iTeam == CT)
		{
			if (!player->m_bReceivesNoMoneyNextRound)
			{
				player->AddAccount(m_iAccountCT);
			}
		}
		else if (player->m_iTeam == TERRORIST)
		{
			// Add another potential escaper to the mix!
			++m_iNumEscapers;

			if (!player->m_bReceivesNoMoneyNextRound)
			{
				player->AddAccount(m_iAccountTerrorist);
			}

			// If it's a prison scenario then remove the Ts guns
			if (m_bMapHasEscapeZone)
			{
				// this will cause them to be reset with default weapons, armor, and items
				player->m_bNotKilled = false;
			}
		}

		if (player->m_iTeam != UNASSIGNED && player->m_iTeam != SPECTATOR)
		{
			// drop the c4 if the player is carrying it
			if (player->m_bHasC4)
			{
				player->DropPlayerItem("weapon_c4");
			}

			player->RoundRespawn();
		}

		// Gooseman : The following code fixes the HUD icon bug
		// by removing the C4 and DEFUSER icons from the HUD regardless
		// for EVERY player (regardless of what team they're on)
	}

	// Respawn entities (glass, doors, etc..)
	CleanUpMap();

	// Give C4 to the terrorists
	if (m_bMapHasBombTarget)
	{
		GiveC4();
	}

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_BUY_TIME_START);
	}

	// Reset game variables
	m_flIntermissionEndTime = 0;
	m_flIntermissionStartTime = 0;
	m_fTeamCount = 0.0;
	m_iAccountTerrorist = m_iAccountCT = 0;
	m_iHostagesRescued = 0;
	m_iHostagesTouched = 0;
	m_iRoundWinStatus = WINNER_NONE;
	m_bTargetBombed = m_bBombDefused = false;
	m_bLevelInitialized = false;
	m_bCompleteReset = false;
}

BOOL CHalfLifeMultiplay::IsThereABomber()
{
	CBasePlayer *pPlayer = NULL;

	while ((pPlayer = (CBasePlayer *)UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (FNullEnt(pPlayer->edict()))
			break;

		if (pPlayer->m_iTeam != CT && pPlayer->IsBombGuy())
		{
			// There you are.
			return TRUE;
		}
	}

	// Didn't find a bomber.
	return FALSE;
}

BOOL CHalfLifeMultiplay::IsThereABomb()
{
	CGrenade *pC4 = NULL;
	CBaseEntity *pWeaponC4 = NULL;
	BOOL bFoundBomb = FALSE;

	while ((pWeaponC4 = UTIL_FindEntityByClassname(pWeaponC4, "grenade")) != NULL)
	{
		if (!pWeaponC4)
			continue;

		pC4 = (CGrenade *)pWeaponC4;

		if (pC4->m_bIsC4)
		{
			bFoundBomb = TRUE;
			break;
		}
	}

	if (bFoundBomb || (UTIL_FindEntityByClassname(NULL, "weapon_c4")) != NULL)
	{
		return TRUE;
	}

	return FALSE;

}

BOOL CHalfLifeMultiplay::TeamFull(int team_id)
{
	switch (team_id)
	{
	case TERRORIST:
		return (m_iNumTerrorist >= m_iSpawnPointCount_Terrorist);

	case CT:
		return (m_iNumCT >= m_iSpawnPointCount_CT);
	}

	return FALSE;
}

// checks to see if the desired team is stacked, returns true if it is

BOOL CHalfLifeMultiplay::TeamStacked(int newTeam_id, int curTeam_id)
{
	// players are allowed to change to their own team
	if (newTeam_id == curTeam_id)
		return FALSE;

	if (!m_iLimitTeams)
		return FALSE;

	switch (newTeam_id)
	{
	case TERRORIST:
		if (curTeam_id != UNASSIGNED && curTeam_id != SPECTATOR)
			return ((m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeams - 1));
		else
			return ((m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeams));
	case CT:
		if (curTeam_id != UNASSIGNED && curTeam_id != SPECTATOR)
			return ((m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeams - 1));
		else
			return ((m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeams));
	}

	return FALSE;
}

void CHalfLifeMultiplay::StackVIPQueue()
{
	for (int i = MAX_VIP_QUEUES - 2; i > 0; --i)
	{
		if (VIPQueue[i - 1])
		{
			if (!VIPQueue[i])
			{
				VIPQueue[i] = VIPQueue[i + 1];
				VIPQueue[i + 1] = NULL;
			}
		}
		else
		{
			VIPQueue[i - 1] = VIPQueue[i];
			VIPQueue[i] = VIPQueue[i + 1];
			VIPQueue[i + 1] = NULL;
		}
	}
}

bool CHalfLifeMultiplay::IsVIPQueueEmpty()
{
	for (int i = 0; i < MAX_VIP_QUEUES; ++i)
	{
		CBasePlayer *toCheck = VIPQueue[i];

		if (toCheck != NULL && toCheck->m_iTeam != CT)
		{
			VIPQueue[i] = NULL;
		}
	}

	StackVIPQueue();
	return (VIPQueue[0] == NULL && VIPQueue[1] == NULL && VIPQueue[2] == NULL && VIPQueue[3] == NULL && VIPQueue[4] == NULL);
}

bool CHalfLifeMultiplay::AddToVIPQueue(CBasePlayer *toAdd)
{
	for (int i = 0; i < MAX_VIP_QUEUES; ++i)
	{
		CBasePlayer *toCheck = VIPQueue[i];

		if (toCheck != NULL && toCheck->m_iTeam != CT)
		{
			VIPQueue[i] = NULL;
		}
	}

	StackVIPQueue();

	if (toAdd->m_iTeam == CT)
	{
		int j;
		for (j = 0; j < MAX_VIP_QUEUES; ++j)
		{
			if (VIPQueue[j] == toAdd)
			{
				ClientPrint(toAdd->pev, HUD_PRINTCENTER, "#Game_in_position", UTIL_dtos1(j + 1));
				return FALSE;
			}
		}

		for (j = 0; j < MAX_VIP_QUEUES; ++j)
		{
			if (!VIPQueue[j])
			{
				VIPQueue[j] = toAdd;

				StackVIPQueue();
				ClientPrint(toAdd->pev, HUD_PRINTCENTER, "#Game_added_position", UTIL_dtos1(j + 1));
				return TRUE;
			}
		}

		ClientPrint(toAdd->pev, HUD_PRINTCENTER, "#All_VIP_Slots_Full");
	}

	return FALSE;
}

void CHalfLifeMultiplay::ResetCurrentVIP()
{
	char *infobuffer = GET_INFO_BUFFER(m_pVIP->edict());
	int numSkins = g_bIsCzeroGame ? CZ_NUM_SKIN : CS_NUM_SKIN;

	switch (RANDOM_LONG(0, numSkins))
	{
	case 1:
		m_pVIP->m_iModelName = MODEL_GSG9;
		SET_CLIENT_KEY_VALUE(m_pVIP->entindex(), infobuffer, "model", "gsg9");
		break;
	case 2:
		m_pVIP->m_iModelName = MODEL_SAS;
		SET_CLIENT_KEY_VALUE(m_pVIP->entindex(), infobuffer, "model", "sas");
		break;
	case 3:
		m_pVIP->m_iModelName = MODEL_GIGN;
		SET_CLIENT_KEY_VALUE(m_pVIP->entindex(), infobuffer, "model", "gign");
		break;
	case 4:
		if (g_bIsCzeroGame)
		{
			m_pVIP->m_iModelName = MODEL_SPETSNAZ;
			SET_CLIENT_KEY_VALUE(m_pVIP->entindex(), infobuffer, "model", "spetsnaz");
			break;
		}
	default:
		m_pVIP->m_iModelName = MODEL_URBAN;
		SET_CLIENT_KEY_VALUE(m_pVIP->entindex(), infobuffer, "model", "urban");
		break;
	}

	m_pVIP->m_bIsVIP = false;
	m_pVIP->m_bNotKilled = false;
}

void CHalfLifeMultiplay::PickNextVIP()
{
	if (!IsVIPQueueEmpty())
	{
		// Remove the current VIP from his VIP status and make him a regular CT.
		if (m_pVIP != NULL)
		{
			ResetCurrentVIP();
		}

		for (int i = 0; i < MAX_VIP_QUEUES; ++i)
		{
			if (VIPQueue[i] != NULL)
			{
				m_pVIP = VIPQueue[i];
				m_pVIP->MakeVIP();

				VIPQueue[i] = NULL;	// remove this player from the VIP queue
				StackVIPQueue();		// and re-organize the queue
				m_iConsecutiveVIP = 0;
				return;
			}
		}
	}
	// If it's been the same VIP for 3 rounds already.. then randomly pick a new one
	else if (m_iConsecutiveVIP >= 3)
	{
		++m_iLastPick;

		if (m_iLastPick > m_iNumCT)
			m_iLastPick = 1;

		int iCount = 1;

		CBaseEntity *pPlayer = NULL;
		CBasePlayer *player = NULL;
		CBasePlayer *pLastPlayer = NULL;

		pPlayer = UTIL_FindEntityByClassname(pPlayer, "player");

		while ((pPlayer != NULL) && (!FNullEnt(pPlayer->edict())))
		{
			if (!(pPlayer->pev->flags & FL_DORMANT))
			{
				player = GetClassPtr<CBasePlayer>(pPlayer->pev);

				if (player->m_iTeam == CT && iCount == m_iLastPick)
				{
					if (player == m_pVIP && pLastPlayer != NULL)
						player = pLastPlayer;

					// Remove the current VIP from his VIP status and make him a regular CT.
					if (m_pVIP != NULL)
					{
						ResetCurrentVIP();
					}

					player->MakeVIP();
					m_iConsecutiveVIP = 0;

					return;
				}
				else if (player->m_iTeam == CT)
					++iCount;

				if (player->m_iTeam != SPECTATOR)
					pLastPlayer = player;
			}

			pPlayer = UTIL_FindEntityByClassname(pPlayer, "player");
		}
	}
	// There is no VIP and there is no one waiting to be the VIP.. therefore just pick the first CT player we can find.
	else if (m_pVIP == NULL)
	{
		CBaseEntity *pPlayer = NULL;
		CBasePlayer *player = NULL;

		pPlayer = UTIL_FindEntityByClassname(pPlayer, "player");
		while ((pPlayer != NULL) && (!FNullEnt(pPlayer->edict())))
		{
			if (pPlayer->pev->flags != FL_DORMANT)
			{
				player = GetClassPtr<CBasePlayer>(pPlayer->pev);

				if (player->m_iTeam == CT)
				{
					player->MakeVIP();
					m_iConsecutiveVIP = 0;
					return;
				}
			}

			pPlayer = UTIL_FindEntityByClassname(pPlayer, "player");
		}
	}
}

void CHalfLifeMultiplay::Think()
{
	MonitorTutorStatus();

	m_VoiceGameMgr.Update(gpGlobals->frametime);

	if (sv_clienttrace->value != 1.0f)
	{
		CVAR_SET_FLOAT("sv_clienttrace", 1);
	}

	if (!m_fRoundCount)
	{
		// intialize the timer time stamps, this happens once only
		m_fIntroRoundCount = m_fRoundCount = gpGlobals->time;
	}

	if (m_flForceCameraValue != forcecamera.value
		|| m_flForceChaseCamValue != forcechasecam.value
		|| m_flFadeToBlackValue != fadetoblack.value)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgForceCam);
			WRITE_BYTE(forcecamera.value != 0);
			WRITE_BYTE(forcechasecam.value != 0);
			WRITE_BYTE(fadetoblack.value != 0);
		MESSAGE_END();

		m_flForceCameraValue = forcecamera.value;
		m_flForceChaseCamValue = forcechasecam.value;
		m_flFadeToBlackValue = fadetoblack.value;
	}

	// Check game rules
	if (CheckGameOver())
		return;

	// have we hit the timelimit?
	if (CheckTimeLimit())
		return;

	if (!IsCareer())
	{
		// have we hit the max rounds?
		if (CheckMaxRounds())
			return;

		if (CheckWinLimit())
			return;
	}

	if (!IsCareer() || (m_fCareerMatchMenuTime <= 0.0 || m_fCareerMatchMenuTime >= gpGlobals->time))
	{
		if (m_iStoredSpectValue != allow_spectators.value)
		{
			m_iStoredSpectValue = allow_spectators.value;

			MESSAGE_BEGIN(MSG_ALL, gmsgAllowSpec);
				WRITE_BYTE((int)allow_spectators.value);
			MESSAGE_END();
		}

		// Check for the end of the round.
		if (IsFreezePeriod())
		{
			CheckFreezePeriodExpired();
		}
		else
		{
			CheckRoundTimeExpired();
		}

		if (m_fTeamCount != 0.0f && m_fTeamCount <= gpGlobals->time)
		{
			if (!IsCareer() || !m_fCareerRoundMenuTime)
			{
				RestartRound();
			}
			else if (TheCareerTasks != NULL)
			{
				bool isBotSpeaking = false;

				if (m_fTeamCount + 10.0 > gpGlobals->time)
				{
					isBotSpeaking = IsBotSpeaking();
				}

				if (!isBotSpeaking)
				{
					if (m_fCareerMatchMenuTime == 0.0f && m_iCareerMatchWins)
					{
						bool canCTsWin = true;
						bool canTsWin = true;

						if (m_iNumCTWins < m_iCareerMatchWins || (m_iNumCTWins - m_iNumTerroristWins < m_iRoundWinDifference))
							canCTsWin = false;

						if (m_iNumTerroristWins < m_iCareerMatchWins || (m_iNumTerroristWins - m_iNumCTWins < m_iRoundWinDifference))
							canTsWin = false;

						if (!Q_strcmp(humans_join_team.string, "CT"))
						{
							if (!TheCareerTasks->AreAllTasksComplete())
							{
								canCTsWin = false;
							}
						}
						else if (!TheCareerTasks->AreAllTasksComplete())
						{
							canTsWin = false;
						}

						if (canCTsWin || canTsWin)
						{
							m_fCareerRoundMenuTime = 0;
							m_fCareerMatchMenuTime = gpGlobals->time + 3.0f;

							return;
						}
					}

					m_bFreezePeriod = TRUE;

					for (int i = 1; i <= gpGlobals->maxClients; ++i)
					{
						CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

						if (pPlayer != NULL && !pPlayer->IsBot())
						{
							MESSAGE_BEGIN(MSG_ONE, gmsgCZCareerHUD, NULL, pPlayer->pev);
								WRITE_STRING("ROUND");
								WRITE_LONG(m_iNumCTWins);
								WRITE_LONG(m_iNumTerroristWins);
								WRITE_BYTE(m_iCareerMatchWins);
								WRITE_BYTE(m_iRoundWinDifference);
								WRITE_BYTE(m_iRoundWinStatus);
							MESSAGE_END();

							pPlayer->m_iHideHUD |= HIDEHUD_ALL;
							m_fTeamCount = gpGlobals->time + 100000.0;

							UTIL_LogPrintf("Career Round %d %d %d %d\n", m_iRoundWinStatus, m_iNumCTWins, m_iNumTerroristWins, TheCareerTasks->AreAllTasksComplete());
							break;
						}
					}

					m_fCareerRoundMenuTime = 0;
				}
			}

			if (TheTutor != NULL)
			{
				TheTutor->PurgeMessages();
			}
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
	}
	else
	{
		if (m_fCareerMatchMenuTime + 10 <= gpGlobals->time || !IsBotSpeaking())
		{
			UTIL_CareerDPrintf("Ending career match...one team has won the specified number of rounds\n");

			MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
				WRITE_STRING("MATCH");
				WRITE_LONG(m_iNumCTWins);
				WRITE_LONG(m_iNumTerroristWins);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgCZCareerHUD);
				WRITE_STRING("MATCH");
				WRITE_LONG(m_iNumCTWins);
				WRITE_LONG(m_iNumTerroristWins);
				WRITE_BYTE(m_iCareerMatchWins);
				WRITE_BYTE(m_iRoundWinDifference);
				WRITE_BYTE(m_iRoundWinStatus);
			MESSAGE_END();

			UTIL_LogPrintf("Career Match %d %d %d %d\n", m_iRoundWinStatus, m_iNumCTWins, m_iNumTerroristWins, TheCareerTasks->AreAllTasksComplete());
			SERVER_COMMAND("setpause\n");
		}
	}
}

bool CHalfLifeMultiplay::CheckGameOver()
{
	// someone else quit the game already
	if (g_fGameOver)
	{
		int time = (int)CVAR_GET_FLOAT("mp_chattime");

		if (time < 1)
			CVAR_SET_STRING("mp_chattime", "1");

		else if (time > MAX_INTERMISSION_TIME)
			CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

		// bounds check
		m_flIntermissionEndTime = m_flIntermissionStartTime + mp_chattime.value;

		// check to see if we should change levels now
		if (m_flIntermissionEndTime < gpGlobals->time && !IsCareer())
		{
			if (!UTIL_HumansInGame()		// if only bots, just change immediately
				|| m_iEndIntermissionButtonHit		// check that someone has pressed a key, or the max intermission time is over
				|| ((m_flIntermissionStartTime + MAX_INTERMISSION_TIME) < gpGlobals->time))
			{
				// intermission is over
				ChangeLevel();
			}
		}

		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::CheckTimeLimit()
{
	float fTimeLimit = timelimit.value;

	if (fTimeLimit < 0)
	{
		CVAR_SET_FLOAT("mp_timelimit", 0);
		return false;
	}

	if (!IsCareer())
	{
		if (fTimeLimit != 0.0f)
		{
			g_flTimeLimit = g_flResetTime + fTimeLimit * 60.0f;
		}

		if (fTimeLimit > 0 && gpGlobals->time >= g_flTimeLimit)
		{
			ALERT(at_console, "Changing maps because time limit has been met\n");
			GoToIntermission();
			return true;
		}
	}

	return false;
}

bool CHalfLifeMultiplay::CheckMaxRounds()
{
	if (m_iMaxRounds != 0 && m_iTotalRoundsPlayed >= m_iMaxRounds)
	{
		ALERT(at_console, "Changing maps due to maximum rounds have been met\n");
		GoToIntermission();
		return true;
	}

	return false;
}

bool CHalfLifeMultiplay::CheckWinLimit()
{
	// has one team won the specified number of rounds?
	if (m_iMaxRoundsWon != 0 && (m_iNumCTWins >= m_iMaxRoundsWon || m_iNumTerroristWins >= m_iMaxRoundsWon))
	{
		if ((m_iNumCTWins - m_iNumTerroristWins >= m_iRoundWinDifference) || (m_iNumTerroristWins - m_iNumCTWins >= m_iRoundWinDifference))
		{
			ALERT(at_console, "Changing maps...one team has won the specified number of rounds\n");
			GoToIntermission();
			return true;
		}
	}

	return false;
}

void CHalfLifeMultiplay::CheckFreezePeriodExpired()
{
	if (TimeRemaining() > 0)
		return;

	// Log this information
	UTIL_LogPrintf("World triggered \"Round_Start\"\n");

	// Freeze period expired: kill the flag
	m_bFreezePeriod = FALSE;

	char CT_sentence[40];
	char T_sentence[40];

	switch (RANDOM_LONG(0, 3))
	{
	case 0:
		Q_strncpy(CT_sentence, "%!MRAD_MOVEOUT", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_MOVEOUT", sizeof(T_sentence));
		break;
	case 1:
		Q_strncpy(CT_sentence, "%!MRAD_LETSGO", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_LETSGO", sizeof(T_sentence));
		break;
	case 2:
		Q_strncpy(CT_sentence, "%!MRAD_LOCKNLOAD", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_LOCKNLOAD", sizeof(T_sentence));
		break;
	default:
		Q_strncpy(CT_sentence, "%!MRAD_GO", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_GO", sizeof(T_sentence));
		break;
	}

	// More specific radio commands for the new scenarios : Prison & Assasination
	if (m_bMapHasEscapeZone)
	{
		Q_strncpy(CT_sentence, "%!MRAD_ELIM", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_GETOUT", sizeof(T_sentence));
	}
	else if (m_iMapHasVIPSafetyZone == MAP_HAVE_VIP_SAFETYZONE_YES)
	{
		Q_strncpy(CT_sentence, "%!MRAD_VIP", sizeof(CT_sentence));
		Q_strncpy(T_sentence, "%!MRAD_LOCKNLOAD", sizeof(T_sentence));
	}

	// Reset the round time
	m_fRoundCount = gpGlobals->time;

	// in seconds
	m_iRoundTimeSecs = m_iRoundTime;

	bool bCTPlayed = false;
	bool bTPlayed = false;

	if (TheCareerTasks != NULL)
	{
		TheCareerTasks->HandleEvent(EVENT_ROUND_START);
	}

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *plr = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (!plr || plr->pev->flags == FL_DORMANT)
		{
			continue;
		}

		if (plr->m_iJoiningState == JOINED)
		{
			if (plr->m_iTeam == CT && !bCTPlayed)
			{
				plr->Radio(CT_sentence);
				bCTPlayed = true;
			}
			else if (plr->m_iTeam == TERRORIST && !bTPlayed)
			{
				plr->Radio(T_sentence);
				bTPlayed = true;
			}

			if (plr->m_iTeam != SPECTATOR)
			{
				plr->ResetMaxSpeed();
				plr->m_bCanShoot = true;
			}
		}

		plr->SyncRoundTimer();
	}

	if (TheBots != NULL)
	{
		TheBots->OnEvent(EVENT_ROUND_START);
	}

	if (TheCareerTasks != NULL)
	{
		TheCareerTasks->HandleEvent(EVENT_ROUND_START);
	}
}

void CHalfLifeMultiplay::CheckRoundTimeExpired()
{
	if (!HasRoundTimeExpired())
		return;

#if 0
	// Round time expired
	float flEndRoundTime;

	// Check to see if there's still a live C4 hanging around.. if so, wait until this C4 blows before ending the round
	CGrenade *C4 = (CGrenade *)UTIL_FindEntityByClassname(NULL, "grenade");

	if (C4 != NULL)
	{
		if (!C4->m_bJustBlew)
			flEndRoundTime = C4->m_flC4Blow;
		else
			flEndRoundTime = gpGlobals->time + 5.0;
	}
#endif

	// New code to get rid of round draws!!
	if (m_bMapHasBombTarget)
	{
		Broadcast("ctwin");

		m_iAccountCT += REWARD_TARGET_BOMB_SAVED;
		m_iNumCTWins++;

		EndRoundMessage("#Target_Saved", ROUND_TARGET_SAVED);
		TerminateRound(5, WINSTATUS_CTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
		}

		UpdateTeamScores();
		MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(TERRORIST);
	}
	else if (UTIL_FindEntityByClassname(NULL, "hostage_entity") != NULL)
	{
		Broadcast("terwin");
		m_iAccountTerrorist += REWARD_HOSTAGE_NOT_RESCUED;
		m_iNumTerroristWins++;

		EndRoundMessage("#Hostages_Not_Rescued", ROUND_HOSTAGE_NOT_RESCUED);
		TerminateRound(5, WINSTATUS_TERRORISTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
		}

		UpdateTeamScores();
		MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(CT);
	}
	else if (m_bMapHasEscapeZone)
	{
		Broadcast("ctwin");
		m_iNumCTWins++;

		EndRoundMessage("#Terrorists_Not_Escaped", ROUND_TERRORISTS_NOT_ESCAPED);
		TerminateRound(5, WINSTATUS_CTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_CTS);
		}

		UpdateTeamScores();
	}
	else if (m_iMapHasVIPSafetyZone == MAP_HAVE_VIP_SAFETYZONE_YES)
	{
		Broadcast("terwin");
		m_iAccountTerrorist += REWARD_VIP_NOT_ESCAPED;
		m_iNumTerroristWins++;

		EndRoundMessage("#VIP_Not_Escaped", ROUND_VIP_NOT_ESCAPED);
		TerminateRound(5, WINSTATUS_TERRORISTS);

		if (IsCareer())
		{
			QueueCareerRoundEndMenu(5, WINSTATUS_TERRORISTS);
		}

		UpdateTeamScores();
	}

	// This is done so that the portion of code has enough time to do it's thing.
	m_fRoundCount = gpGlobals->time + 60.0f;
}

void CHalfLifeMultiplay::CheckLevelInitialized()
{
	if (!m_bLevelInitialized)
	{
		// Count the number of spawn points for each team
		// This determines the maximum number of players allowed on each
		CBaseEntity *ent = NULL;

		m_iSpawnPointCount_Terrorist = 0;
		m_iSpawnPointCount_CT = 0;

		while ((ent = UTIL_FindEntityByClassname(ent, "info_player_deathmatch")) != NULL)
			++m_iSpawnPointCount_Terrorist;

		while ((ent = UTIL_FindEntityByClassname(ent, "info_player_start")) != NULL)
			++m_iSpawnPointCount_CT;

		m_bLevelInitialized = true;
	}
}

void CHalfLifeMultiplay::CheckRestartRound()
{
	// Restart the round if specified by the server
	int iRestartDelay = (int)restartround.value;

	if (!iRestartDelay)
	{
		iRestartDelay = sv_restart.value;
	}

	if (iRestartDelay > 0)
	{
		if (iRestartDelay > 60)
			iRestartDelay = 60;

		// log the restart
		UTIL_LogPrintf("World triggered \"Restart_Round_(%i_%s)\"\n", iRestartDelay, (iRestartDelay == 1) ? "second" : "seconds");
		UTIL_LogPrintf("Team \"CT\" scored \"%i\" with \"%i\" players\n", m_iNumCTWins, m_iNumCT);
		UTIL_LogPrintf("Team \"TERRORIST\" scored \"%i\" with \"%i\" players\n", m_iNumTerroristWins, m_iNumTerrorist);

		// let the players know
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Game_will_restart_in", UTIL_dtos1(iRestartDelay), (iRestartDelay == 1) ? "SECOND" : "SECONDS");
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "#Game_will_restart_in_console", UTIL_dtos1(iRestartDelay), (iRestartDelay == 1) ? "SECOND" : "SECONDS");

		m_fTeamCount = gpGlobals->time + iRestartDelay;
		m_bCompleteReset = true;

		CVAR_SET_FLOAT("sv_restartround", 0);
		CVAR_SET_FLOAT("sv_restart", 0);

		CareerRestart();
	}
}

bool CHalfLifeMultiplay::HasRoundTimeExpired()
{
	// We haven't completed other objectives, so go for this!.
	if (TimeRemaining() > 0 || m_iRoundWinStatus != WINNER_NONE)
	{
		return false;
	}

	// If the bomb is planted, don't let the round timer end the round.
	// keep going until the bomb explodes or is defused
	if (!IsBombPlanted())
	{
		if (cv_bot_nav_edit.value == 0.0f || IS_DEDICATED_SERVER() || UTIL_HumansInGame() != 1)
		{
			return true;
		}
	}

	return false;
}

bool CHalfLifeMultiplay::IsBombPlanted()
{
	if (m_bMapHasBombTarget)
	{
		CGrenade *bomb = NULL;

		while ((bomb = (CGrenade *)UTIL_FindEntityByClassname(bomb, "grenade")) != NULL)
		{
			if (bomb->m_bIsC4)
			{
				return true;
			}
		}
	}

	return false;
}

// living players on the given team need to be marked as not receiving any money
// next round.

void CHalfLifeMultiplay::MarkLivingPlayersOnTeamAsNotReceivingMoneyNextRound(int iTeam)
{
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (!player || FNullEnt(player->pev))
			continue;

		if (player->m_iTeam == iTeam)
		{
			if (player->pev->health > 0 && player->pev->deadflag == DEAD_NO)
			{
				player->m_bReceivesNoMoneyNextRound = true;
			}
		}
	}
}

void CHalfLifeMultiplay::CareerRestart()
{
	g_fGameOver = FALSE;

	if (m_fTeamCount == 0.0f)
	{
		m_fTeamCount = gpGlobals->time + 1.0f;
	}

	// for reset everything
	m_bCompleteReset = true;
	m_fCareerRoundMenuTime = 0;
	m_fCareerMatchMenuTime = 0;

	if (TheCareerTasks != NULL)
	{
		TheCareerTasks->Reset(false);
	}

	m_bSkipSpawn = false;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (!player || FNullEnt(player->pev))
			continue;

		if (!player->IsBot())
		{
			player->ForceClientDllUpdate();
		}
	}
}

BOOL CHalfLifeMultiplay::IsMultiplayer()
{
	return TRUE;
}

BOOL CHalfLifeMultiplay::IsDeathmatch()
{
	return TRUE;
}

BOOL CHalfLifeMultiplay::IsCoOp()
{
	return gpGlobals->coop;
}

BOOL CHalfLifeMultiplay::FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (!pWeapon->CanDeploy())
		return FALSE;

	if (!pPlayer->m_pActiveItem)
		return TRUE;

	if (!pPlayer->m_iAutoWepSwitch)
		return FALSE;

	if (!pPlayer->m_pActiveItem->CanHolster())
		return FALSE;

	if (pWeapon->iWeight() > pPlayer->m_pActiveItem->iWeight())
		return TRUE;

	return FALSE;
}

BOOL CHalfLifeMultiplay::GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon)
{
	CBasePlayerItem *pCheck;
	CBasePlayerItem *pBest; // this will be used in the event that we don't find a weapon in the same category.
	int iBestWeight;
	int i;

	if (!pCurrentWeapon->CanHolster())
	{
		// can't put this gun away right now, so can't switch.
		return FALSE;
	}

	iBestWeight = -1; // no weapon lower than -1 can be autoswitched to
	pBest = NULL;

	for (i = 0; i < MAX_ITEM_TYPES; ++i)
	{
		pCheck = pPlayer->m_rgpPlayerItems[i];

		while (pCheck != NULL)
		{
			// don't reselect the weapon we're trying to get rid of
			if (pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon)
			{
				//ALERT (at_console, "Considering %s\n", STRING(pCheck->pev->classname));
				// we keep updating the 'best' weapon just in case we can't find a weapon of the same weight
				// that the player was using. This will end up leaving the player with his heaviest-weighted
				// weapon.

				if (pCheck->CanDeploy())
				{
					// if this weapon is useable, flag it as the best
					iBestWeight = pCheck->iWeight();
					pBest = pCheck;
				}
			}

			pCheck = pCheck->m_pNext;
		}
	}

	// if we make it here, we've checked all the weapons and found no useable
	// weapon in the same catagory as the current weapon.

	// if pBest is null, we didn't find ANYTHING. Shouldn't be possible- should always
	// at least get the crowbar, but ya never know.
	if (pBest == NULL)
	{
		return FALSE;
	}

	pPlayer->SwitchWeapon(pBest);
	return TRUE;
}

BOOL CHalfLifeMultiplay::ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd)
{
	return m_VoiceGameMgr.ClientCommand(pPlayer, pcmd);
}

BOOL CHalfLifeMultiplay::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	return FALSE;
}

BOOL CHalfLifeMultiplay::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	m_VoiceGameMgr.ClientConnected(pEntity);
	return TRUE;
}

void CHalfLifeMultiplay::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
		WRITE_BYTE(1);
	MESSAGE_END();
}

void CHalfLifeMultiplay::InitHUD(CBasePlayer *pl)
{
	int i;

	// notify other clients of player joining the game
	UTIL_LogPrintf("\"%s<%i><%s><>\" entered the game\n", STRING(pl->pev->netname), GETPLAYERUSERID(pl->edict()), GETPLAYERAUTHID(pl->edict()));

	UpdateGameMode(pl);

	if (!g_flWeaponCheat)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgViewMode, NULL, pl->edict());
		MESSAGE_END();
	}

	// sending just one score makes the hud scoreboard active; otherwise
	// it is just disabled for single play
	MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
		WRITE_BYTE(ENTINDEX(pl->edict()));
		WRITE_SHORT(0);
		WRITE_SHORT(0);
		WRITE_SHORT(0);
		WRITE_SHORT(pl->m_iTeam);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgShadowIdx, NULL, pl->edict());
		WRITE_LONG(g_iShadowSprite);
	MESSAGE_END();

	if (IsCareer())
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgCZCareer, NULL, pl->edict());
			WRITE_STRING("START");
			WRITE_SHORT(m_iRoundTime);
		MESSAGE_END();
	}
	else
		SendMOTDToClient(pl->edict());

	// loop through all active players and send their score info to the new client
	for (i = 1; i <= gpGlobals->maxClients; ++i)
	{
		// FIXME: Probably don't need to cast this just to read m_iDeaths
		CBasePlayer *plr = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (plr != NULL)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
				WRITE_BYTE(i);	// client number
				WRITE_SHORT((int)plr->pev->frags);
				WRITE_SHORT(plr->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(plr->m_iTeam);
			MESSAGE_END();
		}
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
		WRITE_STRING("TERRORIST");
		WRITE_SHORT(m_iNumTerroristWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
		WRITE_STRING("CT");
		WRITE_SHORT(m_iNumCTWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgAllowSpec, NULL, pl->edict());
		WRITE_BYTE((int)allow_spectators.value);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgForceCam, NULL, pl->edict());
		WRITE_BYTE(forcecamera.value != 0);
		WRITE_BYTE(forcechasecam.value != 0);
		WRITE_BYTE(fadetoblack.value != 0);
	MESSAGE_END();

	if (g_fGameOver)
	{
		MESSAGE_BEGIN(MSG_ONE, SVC_INTERMISSION, NULL, pl->edict());
		MESSAGE_END();
	}

	for (i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *plr = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (plr != NULL)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pl->edict());
				WRITE_BYTE(plr->entindex());
				switch (plr->m_iTeam)
				{
				case CT:
					WRITE_STRING("CT");
					break;
				case TERRORIST:
					WRITE_STRING("TERRORIST");
					break;
				case SPECTATOR:
					WRITE_STRING("SPECTATOR");
					break;
				default:
					WRITE_STRING("UNASSIGNED");
					break;
				}
			MESSAGE_END();

			plr->SetScoreboardAttributes(pl);

			if (i != pl->entindex())
			{
				if (plr->pev->flags == FL_DORMANT)
					continue;

				if (plr->pev->deadflag == DEAD_NO)
				{
					MESSAGE_BEGIN(MSG_ONE, gmsgRadar, NULL, pl->edict());
						WRITE_BYTE(plr->entindex());
						WRITE_COORD(plr->pev->origin.x);
						WRITE_COORD(plr->pev->origin.y);
						WRITE_COORD(plr->pev->origin.z);
					MESSAGE_END();
				}
			}
		}
	}

	if (m_bBombDropped)
	{
		CBaseEntity *pWeaponC4 = UTIL_FindEntityByClassname(NULL, "weapon_c4");

		if (pWeaponC4 != NULL)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgBombDrop, NULL, pl->edict());
				WRITE_COORD(pWeaponC4->pev->origin.x);
				WRITE_COORD(pWeaponC4->pev->origin.y);
				WRITE_COORD(pWeaponC4->pev->origin.z);
				WRITE_BYTE(0);
			MESSAGE_END();
		}
	}
}

void CHalfLifeMultiplay::ClientDisconnected(edict_t *pClient)
{
	if (pClient != NULL)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(CBaseEntity::Instance(pClient));

		if (pPlayer != NULL)
		{
			pPlayer->has_disconnected = true;
			pPlayer->pev->deadflag = DEAD_DEAD;
			pPlayer->SetScoreboardAttributes();

			if (pPlayer->m_bHasC4)
			{
				pPlayer->DropPlayerItem("weapon_c4");
			}

			if (pPlayer->m_bHasDefuser)
			{
				pPlayer->DropPlayerItem("item_thighpack");
			}

			if (pPlayer->m_bIsVIP)
			{
				m_pVIP = NULL;
			}

			pPlayer->m_iCurrentKickVote = 0;

			if (pPlayer->m_iMapVote)
			{
				--m_iMapVotes[ pPlayer->m_iMapVote ];

				if (m_iMapVotes[ pPlayer->m_iMapVote ] < 0)
				{
					m_iMapVotes[ pPlayer->m_iMapVote ] = 0;
				}
			}

			MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(pClient));
				WRITE_SHORT(0);
				WRITE_SHORT(0);
				WRITE_SHORT(0);
				WRITE_SHORT(0);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
				WRITE_BYTE(ENTINDEX(pClient));
				WRITE_STRING("UNASSIGNED");
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgLocation);
				WRITE_BYTE(ENTINDEX(pClient));
				WRITE_STRING("");
			MESSAGE_END();

			const char *team = GetTeam(pPlayer->m_iTeam);

			FireTargets("game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0);
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" disconnected\n", STRING(pPlayer->pev->netname), GETPLAYERUSERID(pPlayer->edict()), GETPLAYERAUTHID(pPlayer->edict()), team);

			// destroy all of the players weapons and items
			pPlayer->RemoveAllItems(TRUE);

			if (pPlayer->m_pObserver != NULL)
			{
				pPlayer->m_pObserver->SUB_Remove();
			}

			CBasePlayer *client = NULL;

			while ((client = (CBasePlayer *)UTIL_FindEntityByClassname(client, "player")) != NULL)
			{
				if (FNullEnt(client->edict()))
					break;

				if (!client->pev || client == pPlayer)
					continue;

				if (client->m_hObserverTarget == pPlayer)
				{
					int iMode = client->pev->iuser1;

					client->pev->iuser1 = OBS_NONE;
					client->Observer_SetMode(iMode);
				}
			}
		}
	}

	CheckWinConditions();
}

float CHalfLifeMultiplay::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED * 1.25;
}

BOOL CHalfLifeMultiplay::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (!pAttacker || PlayerRelationship(pPlayer, pAttacker) != GR_TEAMMATE)
	{
		return TRUE;
	}

	if (CVAR_GET_FLOAT("mp_friendlyfire") != 0 || pAttacker == pPlayer)
	{
		return TRUE;
	}

	return FALSE;
}

void CHalfLifeMultiplay::PlayerThink(CBasePlayer *pPlayer)
{
	if (g_fGameOver)
	{
		// check for button presses
		if (!IsCareer() && (pPlayer->m_afButtonPressed & (IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP)))
		{
			m_iEndIntermissionButtonHit = TRUE;
		}

		// clear attack/use commands from player
		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}

	if (!pPlayer->m_bCanShoot && !IsFreezePeriod())
	{
		pPlayer->m_bCanShoot = true;
	}

	if (pPlayer->m_pActiveItem && pPlayer->m_pActiveItem->IsWeapon())
	{
		CBasePlayerWeapon *pWeapon = static_cast<CBasePlayerWeapon *>(pPlayer->m_pActiveItem->GetWeaponPtr());

		if (pWeapon->m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		{
			pPlayer->m_bCanShoot = false;
		}
	}

	if (pPlayer->m_iMenu != Menu_ChooseTeam && pPlayer->m_iJoiningState == SHOWTEAMSELECT)
	{
		int team = MENU_SLOT_TEAM_UNDEFINED;

		if (!Q_stricmp(humans_join_team.string, "T"))
		{
			team = MENU_SLOT_TERRORIST;
		}
		else if (!Q_stricmp(humans_join_team.string, "CT"))
		{
			team = MENU_SLOT_CT;
		}
		else
		{
			if (allow_spectators.value == 0.0f)
				ShowVGUIMenu(pPlayer, VGUI_Menu_Team, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_5), "#Team_Select");
			else
				ShowVGUIMenu(pPlayer, VGUI_Menu_Team, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_5 | MENU_KEY_6), "#Team_Select_Spect");
		}

		pPlayer->m_iMenu = Menu_ChooseTeam;
		pPlayer->m_iJoiningState = PICKINGTEAM;

		if (team != MENU_SLOT_TEAM_UNDEFINED && !pPlayer->IsBot())
		{
			HandleMenu_ChooseTeam(pPlayer, team);

			if (team != MENU_SLOT_TEAM_SPECT && IsCareer())
			{
				// slot 6 - chooses randomize the appearance to model player
				HandleMenu_ChooseAppearance(pPlayer, 6);
			}
		}
	}
}

// Purpose: Player has just spawned. Equip them.

void CHalfLifeMultiplay::PlayerSpawn(CBasePlayer *pPlayer)
{
	// This is tied to the joining state (m_iJoiningState).. add it when the joining state is there.
	if (pPlayer->m_bJustConnected)
	{
		return;
	}

	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

	bool addDefault = true;
	CBaseEntity *pWeaponEntity = NULL;

	while ((pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip")) != NULL)
	{
		pWeaponEntity->Touch(pPlayer);
		addDefault = false;
	}

	if (pPlayer->m_bNotKilled)
		addDefault = false;

	if (addDefault || pPlayer->m_bIsVIP)
	{
		pPlayer->GiveDefaultItems();
	}

	pPlayer->SetPlayerModel(false);
}

BOOL CHalfLifeMultiplay::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	// Player cannot respawn twice in a round
	if (pPlayer->m_iNumSpawns > 0)
	{
		return FALSE;
	}

	// Player cannot respawn until next round if more than 20 seconds in

	// Tabulate the number of players on each team.
	m_iNumCT = CountTeamPlayers(CT);
	m_iNumTerrorist = CountTeamPlayers(TERRORIST);


	if (m_iNumTerrorist > 0 && m_iNumCT > 0)
	{
		// If this player just connected and fadetoblack is on, then maybe
		// the server admin doesn't want him peeking around.
		if (gpGlobals->time > m_fRoundCount + 20.0f)
		{
			if (fadetoblack.value != 0.0f)
			{
				UTIL_ScreenFade(pPlayer, Vector(0, 0, 0), 3, 3, 255, (FFADE_OUT | FFADE_STAYOUT));
			}

			return FALSE;
		}
	}

	// Player cannot respawn while in the Choose Appearance menu
	if (pPlayer->m_iMenu == Menu_ChooseAppearance)
	{
		return FALSE;
	}

	return TRUE;
}

float CHalfLifeMultiplay::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gpGlobals->time;//now!
}

BOOL CHalfLifeMultiplay::AllowAutoTargetCrosshair()
{
	return FALSE;
}

// IPointsForKill - how many points awarded to anyone
// that kills this player?

int CHalfLifeMultiplay::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	return 1;
}

void CHalfLifeMultiplay::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	DeathNotice(pVictim, pKiller, pInflictor);

	pVictim->m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
	pVictim->m_iDeaths++;
	pVictim->m_bNotKilled = false;
	pVictim->m_bEscaped = false;
	pVictim->m_iTrain = (TRAIN_NEW | TRAIN_OFF);
	SET_VIEW(ENT(pVictim->pev), ENT(pVictim->pev));

	CBasePlayer *peKiller = NULL;
	CBaseEntity *ktmp = CBaseEntity::Instance(pKiller);

	if (ktmp && ktmp->Classify() == CLASS_PLAYER)
	{
		peKiller = static_cast<CBasePlayer *>(ktmp);
	}
	else if (ktmp && ktmp->Classify() == CLASS_VEHICLE)
	{
		CBasePlayer *pDriver = static_cast<CBasePlayer *>(((CFuncVehicle *)ktmp)->m_pDriver);

		if (pDriver != NULL)
		{
			pKiller = pDriver->pev;
			peKiller = static_cast<CBasePlayer *>(pDriver);
		}
	}

	FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);

	// Did the player kill himself?
	if (pVictim->pev == pKiller)
	{
		// Players lose a frag for killing themselves
		pKiller->frags -= 1;
	}
	else if (peKiller && peKiller->IsPlayer())
	{
		// if a player dies in a deathmatch game and the killer is a client, award the killer some points
		CBasePlayer *killer = GetClassPtr<CBasePlayer>(pKiller);
		bool killedByFFA = false;

		if (killer->m_iTeam == pVictim->m_iTeam && !killedByFFA)
		{
			// if a player dies by from teammate
			pKiller->frags -= IPointsForKill(peKiller, pVictim);

			killer->AddAccount(PAYBACK_FOR_KILLED_TEAMMATES);
			killer->m_iTeamKills++;
			killer->m_bJustKilledTeammate = true;

			ClientPrint(killer->pev, HUD_PRINTCENTER, "#Killed_Teammate");
			ClientPrint(killer->pev, HUD_PRINTCONSOLE, "#Game_teammate_kills", UTIL_dtos1(killer->m_iTeamKills));

			if (killer->m_iTeamKills == 3 && CVAR_GET_FLOAT("mp_autokick") != 0.0f)
			{
				ClientPrint(killer->pev, HUD_PRINTCONSOLE, "#Banned_For_Killing_Teamates");

				int iUserID = GETPLAYERUSERID(killer->edict());

				if (iUserID != -1)
				{
					SERVER_COMMAND(UTIL_VarArgs("kick # %d\n", iUserID));
				}
			}

			if (!(killer->m_flDisplayHistory & DHF_FRIEND_KILLED))
			{
				killer->m_flDisplayHistory |= DHF_FRIEND_KILLED;
				killer->HintMessage("#Hint_careful_around_teammates");
			}
		}
		else
		{
			// if a player dies in a deathmatch game and the killer is a client, award the killer some points
			pKiller->frags += IPointsForKill(peKiller, pVictim);

			if (pVictim->m_bIsVIP)
			{
				killer->HintMessage("#Hint_reward_for_killing_vip", TRUE);
				killer->AddAccount(REWARD_KILLED_VIP);

				MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
					WRITE_BYTE(9);
					WRITE_BYTE(DRC_CMD_EVENT);
					WRITE_SHORT(ENTINDEX(pVictim->edict()));
					WRITE_SHORT(ENTINDEX(ENT(pInflictor)));
					WRITE_LONG(DRC_FLAG_PRIO_MASK | DRC_FLAG_DRAMATIC | DRC_FLAG_FINAL);
				MESSAGE_END();

				UTIL_LogPrintf("\"%s<%i><%s><TERRORIST>\" triggered \"Assassinated_The_VIP\"\n", STRING(killer->pev->netname), GETPLAYERUSERID(killer->edict()), GETPLAYERAUTHID(killer->edict()));
			}
			else
				killer->AddAccount(REWARD_KILLED_ENEMY);

			if (!(killer->m_flDisplayHistory & DHF_ENEMY_KILLED))
			{
				killer->m_flDisplayHistory |= DHF_ENEMY_KILLED;
				killer->HintMessage("#Hint_win_round_by_killing_enemy");
			}
		}

		FireTargets("game_playerkill", peKiller, peKiller, USE_TOGGLE, 0);
	}
	else
	{
		// killed by the world
		pKiller->frags -= 1;
	}

	// update the scores
	// killed scores
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
		WRITE_BYTE(ENTINDEX(pVictim->edict()));
		WRITE_SHORT((int)pVictim->pev->frags);
		WRITE_SHORT(pVictim->m_iDeaths);
		WRITE_SHORT(0);
		WRITE_SHORT(pVictim->m_iTeam);
	MESSAGE_END();

	// killers score, if it's a player
	CBaseEntity *ep = CBaseEntity::Instance(pKiller);

	if (ep && ep->Classify() == CLASS_PLAYER)
	{
		CBasePlayer *PK = static_cast<CBasePlayer *>(ep);

		MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
			WRITE_BYTE(ENTINDEX(PK->edict()));
			WRITE_SHORT((int)PK->pev->frags);
			WRITE_SHORT(PK->m_iDeaths);
			WRITE_SHORT(0);
			WRITE_SHORT(PK->m_iTeam);
		MESSAGE_END();

		// let the killer paint another decal as soon as he'd like.
		PK->m_flNextDecalTime = gpGlobals->time;
	}
}

void CHalfLifeMultiplay::DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor)
{
	// Work out what killed the player, and send a message to all clients about it
	// CBaseEntity *Killer = CBaseEntity::Instance(pKiller);

	// by default, the player is killed by the world
	const char *killer_weapon_name = "world";
	int killer_index = 0;

	// Hack to fix name change
	const char *tau = "tau_cannon";
	const char *gluon = "gluon gun";

	// Is the killer a client?
	if (pKiller->flags & FL_CLIENT)
	{
		killer_index = ENTINDEX(ENT(pKiller));

		if (pevInflictor)
		{
			if (pevInflictor == pKiller)
			{
				// If the inflictor is the killer, then it must be their current weapon doing the damage
				CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(CBaseEntity::Instance(pKiller));

				if (pPlayer != NULL && pPlayer->m_pActiveItem != NULL)
				{
					killer_weapon_name = pPlayer->m_pActiveItem->pszName();
				}
			}
			else
			{
				// it's just that easy
				killer_weapon_name = STRING(pevInflictor->classname);
			}
		}
	}
	else
		killer_weapon_name = STRING(pevInflictor->classname);

	// strip the monster_* or weapon_* from the inflictor's classname
	if (!Q_strncmp(killer_weapon_name, "weapon_", 7))
		killer_weapon_name += 7;

	else if (!Q_strncmp(killer_weapon_name, "knife_", 6))
		killer_weapon_name += 6;

	else if (!Q_strncmp(killer_weapon_name, "monster_", 8))
		killer_weapon_name += 8;

	else if (!Q_strncmp(killer_weapon_name, "func_", 5))
		killer_weapon_name += 5;

	int iGotHeadshot = 0;

	if (pVictim->m_bHeadshotKilled)
		iGotHeadshot = 1;

	if (TheTutor == NULL)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgDeathMsg);
			WRITE_BYTE(killer_index);			// the killer
			WRITE_BYTE(ENTINDEX(pVictim->edict()));		// the victim
			WRITE_BYTE(iGotHeadshot);			// is killed headshot
			WRITE_STRING(killer_weapon_name);		// what they were killed by (should this be a string?)
		MESSAGE_END();
	}

	// replace the code names with the 'real' names
	if (!Q_strcmp(killer_weapon_name, "egon"))
		killer_weapon_name = gluon;

	else if (!Q_strcmp(killer_weapon_name, "gauss"))
		killer_weapon_name = tau;

	// Did he kill himself?
	if (pVictim->pev == pKiller)
	{
		// killed self
		const char *team = GetTeam(pVictim->m_iTeam);
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n", STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()),
			GETPLAYERAUTHID(pVictim->edict()), team, killer_weapon_name);
	}
	else if (pKiller->flags & FL_CLIENT)
	{
		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer *>(CBaseEntity::Instance(pKiller));

		const char *VictimTeam = GetTeam(pVictim->m_iTeam);
		const char *KillerTeam = (pPlayer != NULL) ? GetTeam(pPlayer->m_iTeam) : "";

		UTIL_LogPrintf("\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n", STRING(pKiller->netname), GETPLAYERUSERID(ENT(pKiller)), GETPLAYERAUTHID(ENT(pKiller)),
			KillerTeam, STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()), GETPLAYERAUTHID(pVictim->edict()), VictimTeam, killer_weapon_name);
	}
	else
	{
		// killed by the world
		const char *team = GetTeam(pVictim->m_iTeam);
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" committed suicide with \"%s\" (world)\n", STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()),
			GETPLAYERAUTHID(pVictim->edict()), team, killer_weapon_name);
	}

	CheckWinConditions();

	MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
		WRITE_BYTE(9);		// command length in bytes
		WRITE_BYTE(DRC_CMD_EVENT);	// player killed
		WRITE_SHORT(ENTINDEX(pVictim->edict()));	// index number of primary entity

	if (pevInflictor)
		WRITE_SHORT(ENTINDEX(ENT(pevInflictor)));	// index number of secondary entity
	else
		WRITE_SHORT(ENTINDEX(ENT(pKiller)));	// index number of secondary entity

	if (iGotHeadshot)
		WRITE_LONG(9 | DRC_FLAG_DRAMATIC | DRC_FLAG_SLOWMOTION);
	else
		WRITE_LONG(7 | DRC_FLAG_DRAMATIC);	// eventflags (priority and flags)

	MESSAGE_END();
}

// PlayerGotWeapon - player has grabbed a weapon that was
// sitting in the world

void CHalfLifeMultiplay::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	;
}

// FlWeaponRespawnTime - what is the time in the future
// at which this weapon may spawn?

float CHalfLifeMultiplay::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

// FlWeaponRespawnTime - Returns 0 if the weapon can respawn
// now, otherwise it returns the time at which it can try
// to spawn again.

float CHalfLifeMultiplay::FlWeaponTryRespawn(CBasePlayerItem *pWeapon)
{
	if (pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD))
	{
		if (NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE))
			return 0;

		// we're past the entity tolerance level, so delay the respawn
		return FlWeaponRespawnTime(pWeapon);
	}

	return 0;
}

Vector CHalfLifeMultiplay::VecWeaponRespawnSpot(CBasePlayerItem *pWeapon)
{
	return pWeapon->pev->origin;
}

int CHalfLifeMultiplay::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	if (pWeapon->pev->spawnflags & SF_NORESPAWN)
	{
		return GR_WEAPON_RESPAWN_NO;
	}

	return GR_WEAPON_RESPAWN_YES;
}

BOOL CHalfLifeMultiplay::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem)
{
	return CGameRules::CanHavePlayerItem(pPlayer, pItem);
}

BOOL CHalfLifeMultiplay::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	return TRUE;
}

void CHalfLifeMultiplay::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
	;
}

int CHalfLifeMultiplay::ItemShouldRespawn(CItem *pItem)
{
	if (pItem->pev->spawnflags & SF_NORESPAWN)
	{
		return GR_ITEM_RESPAWN_NO;
	}

	return GR_ITEM_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlItemRespawnTime(CItem *pItem)
{
	return gpGlobals->time + ITEM_RESPAWN_TIME;
}

Vector CHalfLifeMultiplay::VecItemRespawnSpot(CItem *pItem)
{
	return pItem->pev->origin;
}

void CHalfLifeMultiplay::PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount)
{
	;
}

BOOL CHalfLifeMultiplay::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	return TRUE;
}

int CHalfLifeMultiplay::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	if (pAmmo->pev->spawnflags & SF_NORESPAWN)
	{
		return GR_AMMO_RESPAWN_NO;
	}

	return GR_AMMO_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo)
{
	return gpGlobals->time + 20.0f;
}

Vector CHalfLifeMultiplay::VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo)
{
	return pAmmo->pev->origin;
}

float CHalfLifeMultiplay::FlHealthChargerRechargeTime()
{
	return 60;
}

float CHalfLifeMultiplay::FlHEVChargerRechargeTime()
{
	return 30;
}

int CHalfLifeMultiplay::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

int CHalfLifeMultiplay::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

edict_t *CHalfLifeMultiplay::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	// gat valid spawn point
	edict_t *pentSpawnSpot = CGameRules::GetPlayerSpawnSpot(pPlayer);

	if (IsMultiplayer())
	{
		if (pentSpawnSpot->v.target)
		{
			FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);
		}
	}

	return pentSpawnSpot;
}

int CHalfLifeMultiplay::PlayerRelationship(CBasePlayer *pPlayer, CBaseEntity *pTarget)
{
	if (!pPlayer || !pTarget)
	{
		return GR_NOTTEAMMATE;
	}

	if (!pTarget->IsPlayer())
	{
		return GR_NOTTEAMMATE;
	}

	CBasePlayer *player = GetClassPtr<CBasePlayer>(pPlayer->pev);
	CBasePlayer *target = GetClassPtr<CBasePlayer>(pTarget->pev);

	if (player->m_iTeam != target->m_iTeam)
	{
		return GR_NOTTEAMMATE;
	}

	return GR_TEAMMATE;
}

BOOL CHalfLifeMultiplay::FAllowFlashlight()
{
	static cvar_t *mp_flashlight = NULL;

	if (!mp_flashlight)
		mp_flashlight = CVAR_GET_POINTER("mp_flashlight");

	if (mp_flashlight)
		return mp_flashlight->value != 0;

	return FALSE;
}

BOOL CHalfLifeMultiplay::FAllowMonsters()
{
	return CVAR_GET_FLOAT("mp_allowmonsters") != 0;
}

void CHalfLifeMultiplay::GoToIntermission()
{
	if (g_fGameOver)
		return;

	UTIL_LogPrintf("Team \"CT\" scored \"%i\" with \"%i\" players\n", m_iNumCTWins, m_iNumCT);
	UTIL_LogPrintf("Team \"TERRORIST\" scored \"%i\" with \"%i\" players\n", m_iNumTerroristWins, m_iNumTerrorist);

	if (IsCareer())
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
			WRITE_STRING("MATCH");
			WRITE_LONG(m_iNumCTWins);
			WRITE_LONG(m_iNumTerroristWins);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_ALL, gmsgCZCareerHUD);
			WRITE_STRING("MATCH");
			WRITE_LONG(m_iNumCTWins);
			WRITE_LONG(m_iNumTerroristWins);
			WRITE_BYTE(m_iCareerMatchWins);
			WRITE_BYTE(m_iRoundWinDifference);
			WRITE_BYTE(m_iRoundWinStatus);
		MESSAGE_END();

		if (TheCareerTasks != NULL)
		{
			UTIL_LogPrintf("Career Match %d %d %d %d\n", m_iRoundWinStatus, m_iNumCTWins, m_iNumTerroristWins, TheCareerTasks->AreAllTasksComplete());
		}
	}

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();

	if (IsCareer())
	{
		SERVER_COMMAND("setpause\n");
	}

	int time = (int)CVAR_GET_FLOAT("mp_chattime");

	if (time < 1)
		CVAR_SET_STRING("mp_chattime", "1");

	else if (time > MAX_INTERMISSION_TIME)
		CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

	m_flIntermissionEndTime = gpGlobals->time + (int)mp_chattime.value;
	m_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = TRUE;
	m_iEndIntermissionButtonHit = FALSE;
	m_iSpawnPointCount_Terrorist = 0;
	m_iSpawnPointCount_CT = 0;
	m_bLevelInitialized = false;
}

// Clean up memory used by mapcycle when switching it

void DestroyMapCycle(mapcycle_t *cycle)
{
	mapcycle_item_t *p, *n, *start;
	p = cycle->items;

	if (p != NULL)
	{
		start = p;
		p = p->next;
		while (p != start)
		{
			n = p->next;
			delete p;
			p = n;
		}

		delete cycle->items;
	}

	cycle->items = NULL;
	cycle->next_item = NULL;
}

char *MP_COM_GetToken()
{
	return mp_com_token;
}

char *MP_COM_Parse(char *data)
{
	int c;
	int len;

	len = 0;
	mp_com_token[0] = '\0';

	if (!data)
	{
		return NULL;
	}

skipwhite:
	// skip whitespace
	while (*data <= ' ')
	{
		if (!data[0])
			return NULL;

		++data;
	}

	c = *data;

	// skip // comments till the next line
	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			++data;

		goto skipwhite;	// start over new line
	}

	// handle quoted strings specially: copy till the end or another quote
	if (c == '\"')
	{
		++data;	// skip starting quote

		while (true)
		{
			// get char and advance
			c = *data++;

			if (c == '\"' || !c)
			{
				mp_com_token[ len ] = '\0';
				return data;
			}

			mp_com_token[ len++ ] = c;
		}
	}

	// parse single characters
	if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ',')
	{
		mp_com_token[ len++ ] = c;
		mp_com_token[ len ] = '\0';

		return data + 1;
	}

	// parse a regular word
	do
	{
		mp_com_token[ len++ ] = c;
		++data;
		c = *data;

		if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ',')
			break;
	}
	while (c > 32);

	mp_com_token[ len ] = '\0';
	return data;
}

int MP_COM_TokenWaiting(char *buffer)
{
	char *p;

	p = buffer;
	while (*p && *p != '\n')
	{
		if (!isspace(*p) || isalnum(*p))
			return 1;

		++p;
	}

	return 0;
}

int ReloadMapCycleFile(char *filename, mapcycle_t *cycle)
{
	char szBuffer[ MAX_RULE_BUFFER ];
	char szMap[ 32 ];
	int length;
	char *pFileList;
	char *aFileList = pFileList = (char *)LOAD_FILE_FOR_ME(filename, &length);
	int hasbuffer;
	mapcycle_item_s *item, *newlist = NULL, *next;

	if (pFileList && length)
	{
		// the first map name in the file becomes the default
		while (true)
		{
			hasbuffer = 0;
			Q_memset(szBuffer, 0, sizeof(szBuffer));

			pFileList = MP_COM_Parse(pFileList);

			if (Q_strlen(mp_com_token) <= 0)
				break;

			Q_strcpy(szMap, mp_com_token);

			// Any more tokens on this line?
			if (MP_COM_TokenWaiting(pFileList))
			{
				pFileList = MP_COM_Parse(pFileList);
				if (Q_strlen(mp_com_token) > 0)
				{
					hasbuffer = 1;
					Q_strcpy(szBuffer, mp_com_token);
				}
			}

			// Check map
			if (IS_MAP_VALID(szMap))
			{
				// Create entry
				char *s;

				item = new mapcycle_item_s;

				Q_strcpy(item->mapname, szMap);

				item->minplayers = 0;
				item->maxplayers = 0;

				Q_memset(item->rulebuffer, 0, sizeof(item->rulebuffer));

				if (hasbuffer)
				{
					s = GET_KEY_VALUE(szBuffer, "minplayers");

					if (s && s[0] != '\0')
					{
						item->minplayers = Q_atoi(s);
						item->minplayers = Q_max(item->minplayers, 0);
						item->minplayers = Q_min(item->minplayers, gpGlobals->maxClients);
					}

					s = GET_KEY_VALUE(szBuffer, "maxplayers");
					if (s && s[0] != '\0')
					{
						item->maxplayers = Q_atoi(s);
						item->maxplayers = Q_max(item->maxplayers, 0);
						item->maxplayers = Q_min(item->maxplayers, gpGlobals->maxClients);
					}

					// Remove keys
					REMOVE_KEY_VALUE(szBuffer, "minplayers");
					REMOVE_KEY_VALUE(szBuffer, "maxplayers");

					Q_strcpy(item->rulebuffer, szBuffer);
				}

				item->next = cycle->items;
				cycle->items = item;
			}
			else
				ALERT(at_console, "Skipping %s from mapcycle, not a valid map\n", szMap);
		}

		FREE_FILE(aFileList);
	}

	// Fixup circular list pointer
	item = cycle->items;

	// Reverse it to get original order
	while (item)
	{
		next = item->next;
		item->next = newlist;
		newlist = item;
		item = next;
	}

	cycle->items = newlist;
	item = cycle->items;

	// Didn't parse anything
	if (!item)
	{
		return 0;
	}

	while (item->next != NULL)
	{
		item = item->next;
	}

	item->next = cycle->items;
	cycle->next_item = item->next;

	return 1;
}

// Determine the current # of active players on the server for map cycling logic

int CountPlayers()
{
	int num = 0;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if (pEnt != NULL)
		{
			num = num + 1;
		}
	}

	return num;
}

// Parse commands/key value pairs to issue right after map xxx command is issued on server level transition

void ExtractCommandString(char *s, char *szCommand)
{
	// Now make rules happen
	char pkey[512];
	char value[512]; // use two buffers so compares

	// work without stomping on each other
	char *o;

	if (*s == '\\')
		++s;

	while (true)
	{
		o = pkey;
		while (*s != '\\')
		{
			if (!*s)
			{
				return;
			}

			*o++ = *s++;
		}

		*o = '\0';
		++s;

		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
			{
				return;
			}

			*o++ = *s++;
		}

		*o = '\0';

		Q_strcat(szCommand, pkey);
		if (Q_strlen(value) > 0)
		{
			Q_strcat(szCommand, " ");
			Q_strcat(szCommand, value);
		}
		Q_strcat(szCommand, "\n");

		if (!*s)
		{
			return;
		}

		++s;
	}
}

void CHalfLifeMultiplay::ResetAllMapVotes()
{
	CBaseEntity *pTempEntity = NULL;

	while ((pTempEntity = UTIL_FindEntityByClassname(pTempEntity, "player")) != NULL)
	{
		if (FNullEnt(pTempEntity->edict()))
			break;

		CBasePlayer *pTempPlayer = GetClassPtr<CBasePlayer>(pTempEntity->pev);

		if (pTempPlayer->m_iTeam != UNASSIGNED)
		{
			pTempPlayer->m_iMapVote = 0;
		}
	}

	for (int j = 0; j < MAX_VOTE_MAPS; ++j)
		m_iMapVotes[j] = 0;
}

int GetMapCount()
{
	static mapcycle_t mapcycle2;
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");

	DestroyMapCycle(&mapcycle2);
	ReloadMapCycleFile(mapcfile, &mapcycle2);

	int iCount = 0, iDone = 0;
	for (mapcycle_item_s *item = mapcycle2.next_item; item != NULL && iDone != 1; item = item->next)
	{
		if (item == mapcycle2.next_item)
			iDone = 1;

		++iCount;
	}

	return iCount;
}

void CHalfLifeMultiplay::DisplayMaps(CBasePlayer *player, int iVote)
{
	static mapcycle_t mapcycle2;
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");
	char *pszNewMap = NULL;

	int iCount = 0, done = 0;

	DestroyMapCycle(&mapcycle2);
	ReloadMapCycleFile(mapcfile, &mapcycle2);

	mapcycle_item_s *item = mapcycle2.next_item;
	while (!done && item != NULL)
	{
		if (item->next == mapcycle2.next_item)
			done = 1;

		++iCount;

		if (player != NULL)
		{
			if (m_iMapVotes[iCount] == 1)
			{
				ClientPrint(player->pev, HUD_PRINTCONSOLE, "#Vote", UTIL_dtos1(iCount), item->mapname, UTIL_dtos2(1));
			}
			else
				ClientPrint(player->pev, HUD_PRINTCONSOLE, "#Votes", UTIL_dtos1(iCount), item->mapname, UTIL_dtos2(m_iMapVotes[iCount]));
		}

		if (iCount == iVote)
		{
			pszNewMap = item->mapname;
		}

		item = item->next;
	}

	if (!pszNewMap || !iVote)
	{
		return;
	}

	if (Q_strcmp(pszNewMap, STRING(gpGlobals->mapname)))
	{
		CHANGE_LEVEL(pszNewMap, NULL);
		return;
	}

	if (timelimit.value)
	{
		timelimit.value += 30;
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Map_Vote_Extend");
	}

	ResetAllMapVotes();
}

void CHalfLifeMultiplay::ProcessMapVote(CBasePlayer *player, int iVote)
{
	CBaseEntity *pTempEntity = NULL;

	int iValidVotes = 0, iNumPlayers = 0;

	while ((pTempEntity = UTIL_FindEntityByClassname(pTempEntity, "player")) != NULL)
	{
		if (FNullEnt(pTempEntity->edict()))
			break;

		CBasePlayer *pTempPlayer = GetClassPtr<CBasePlayer>(pTempEntity->pev);

		if (pTempPlayer->m_iTeam != UNASSIGNED)
		{
			++iNumPlayers;

			if( ( pTempPlayer->m_iMapVote = iVote ) )
				++iValidVotes;
		}
	}

	m_iMapVotes[iVote] = iValidVotes;

	float ratio = mapvoteratio.value;
	if (mapvoteratio.value > 1)
	{
		ratio = 1;
		CVAR_SET_STRING("mp_mapvoteratio", "1.0");
	}
	else if (mapvoteratio.value < 0.35)
	{
		ratio = 0.35;
		CVAR_SET_STRING("mp_mapvoteratio", "0.35");
	}

	int iRequiredVotes = 2;
	if (iNumPlayers > 2)
	{
		iRequiredVotes = (int)(iNumPlayers * ratio + 0.5);
	}

	if (iValidVotes < iRequiredVotes)
	{
		DisplayMaps(player, 0);
		ClientPrint(player->pev, HUD_PRINTCONSOLE, "#Game_required_votes", UTIL_dtos1(iRequiredVotes));
	}
	else
		DisplayMaps(NULL, iVote);
}

// Server is changing to a new level, check mapcycle.txt for map name and setup info

void CHalfLifeMultiplay::ChangeLevel()
{
	static char szPreviousMapCycleFile[256];
	static mapcycle_t mapcycle;

	char szNextMap[32];
	char szFirstMapInList[32];
	char szCommands[1500];
	char szRules[1500];
	int minplayers = 0, maxplayers = 0;

	// the absolute default level is hldm1
	Q_strcpy(szFirstMapInList, "hldm1");

	int curplayers;
	BOOL do_cycle = TRUE;

	// find the map to change to
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");
	assert(mapcfile != NULL);

	szCommands[0] = '\0';
	szRules[0] = '\0';

	curplayers = CountPlayers();

	// Has the map cycle filename changed?
	if (Q_stricmp(mapcfile, szPreviousMapCycleFile))
	{
		Q_strcpy(szPreviousMapCycleFile, mapcfile);

		DestroyMapCycle(&mapcycle);

		if (!ReloadMapCycleFile(mapcfile, &mapcycle) || !mapcycle.items)
		{
			ALERT(at_console, "Unable to load map cycle file %s\n", mapcfile);
			do_cycle = FALSE;
		}
	}

	if (do_cycle && mapcycle.items)
	{
		BOOL keeplooking = FALSE;
		BOOL found = FALSE;
		mapcycle_item_s *item;

		// Assume current map
		Q_strcpy(szNextMap, STRING(gpGlobals->mapname));
		Q_strcpy(szFirstMapInList, STRING(gpGlobals->mapname));

		// Traverse list
		for (item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next)
		{
			keeplooking = FALSE;

			assert(item != NULL);

			if (item->minplayers != 0)
			{
				if (curplayers >= item->minplayers)
				{
					found = TRUE;
					minplayers = item->minplayers;
				}
				else
				{
					keeplooking = TRUE;
				}
			}

			if (item->maxplayers != 0)
			{
				if (curplayers <= item->maxplayers)
				{
					found = TRUE;
					maxplayers = item->maxplayers;
				}
				else
				{
					keeplooking = TRUE;
				}
			}

			if (keeplooking)
			{
				continue;
			}

			found = TRUE;
			break;
		}

		if (!found)
		{
			item = mapcycle.next_item;
		}

		// Increment next item pointer
		mapcycle.next_item = item->next;

		// Perform logic on current item
		Q_strcpy(szNextMap, item->mapname);
		ExtractCommandString(item->rulebuffer, szCommands);
		Q_strcpy(szRules, item->rulebuffer);
	}

	if (!IS_MAP_VALID(szNextMap))
	{
		Q_strcpy(szNextMap, szFirstMapInList);
	}

	g_fGameOver = TRUE;

	ALERT(at_console, "CHANGE LEVEL: %s\n", szNextMap);
	if (minplayers || maxplayers)
	{
		ALERT(at_console, "PLAYER COUNT:  min %i max %i current %i\n", minplayers, maxplayers, curplayers);
	}

	if (Q_strlen(szRules) > 0)
	{
		ALERT(at_console, "RULES:  %s\n", szRules);
	}

	CHANGE_LEVEL(szNextMap, NULL);
	if (Q_strlen(szCommands) > 0)
	{
		SERVER_COMMAND(szCommands);
	}
}

void CHalfLifeMultiplay::SendMOTDToClient(edict_t *client)
{
	// read from the MOTD.txt file
	int length, char_count = 0;
	char *pFileList;
	char *aFileList = pFileList = (char *)LOAD_FILE_FOR_ME((char *)CVAR_GET_STRING("motdfile"), &length);

	// send the server name
	MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, client);
		WRITE_STRING(CVAR_GET_STRING("hostname"));
	MESSAGE_END();

	// Send the message of the day
	// read it chunk-by-chunk, and send it in parts
	while (pFileList && *pFileList && char_count < MAX_MOTD_LENGTH)
	{
		char chunk[MAX_MOTD_CHUNK + 1];

		if (Q_strlen(pFileList) < sizeof(chunk))
		{
			Q_strcpy(chunk, pFileList);
		}
		else
		{
			Q_strncpy(chunk, pFileList, sizeof(chunk) - 1);
			// Q_strncpy doesn't always append the null terminator
			chunk[sizeof(chunk) - 1] = '\0';
		}

		char_count += Q_strlen(chunk);

		if (char_count < MAX_MOTD_LENGTH)
			pFileList = aFileList + char_count;
		else
			*pFileList = '\0';

		MESSAGE_BEGIN(MSG_ONE, gmsgMOTD, NULL, client);
			WRITE_BYTE((*pFileList != '\0') ? FALSE : TRUE); // FALSE means there is still more message to come
			WRITE_STRING(chunk);
		MESSAGE_END();
	}

	FREE_FILE(aFileList);
}

void CHalfLifeMultiplay::ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer)
{
	pPlayer->SetPlayerModel(pPlayer->m_bHasC4);
	pPlayer->SetPrefsFromUserinfo(infobuffer);
}
