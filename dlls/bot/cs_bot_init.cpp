
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

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

/*
* Globals initialization
*/
cvar_t cv_bot_traceview = { "bot_traceview", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_stop = { "bot_stop", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_show_nav = { "bot_show_nav", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_show_danger = { "bot_show_danger", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_nav_edit = { "bot_nav_edit", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_nav_zdraw = { "bot_nav_zdraw", "4", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_walk = { "bot_walk", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_difficulty = { "bot_difficulty", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_debug = { "bot_debug", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_quicksave = { "bot_quicksave", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_quota = { "bot_quota", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_quota_match = { "bot_quota_match", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_prefix = { "bot_prefix", "", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_rogues = { "bot_allow_rogues", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_pistols = { "bot_allow_pistols", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_shotguns = { "bot_allow_shotguns", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_sub_machine_guns = { "bot_allow_sub_machine_guns", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_rifles = { "bot_allow_rifles", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_machine_guns = { "bot_allow_machine_guns", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_grenades = { "bot_allow_grenades", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_snipers = { "bot_allow_snipers", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_allow_shield = { "bot_allow_shield", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_join_team = { "bot_join_team", "any", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_join_after_player = { "bot_join_after_player", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_auto_vacate = { "bot_auto_vacate", "1", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_zombie = { "bot_zombie", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_defer_to_human = { "bot_defer_to_human", "0", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_chatter = { "bot_chatter", "normal", FCVAR_SERVER, 0.0f, NULL };
cvar_t cv_bot_profile_db = { "bot_profile_db", "BotProfile.db", FCVAR_SERVER, 0.0f, NULL };

void InstallBotControl()
{
	if (TheBots != NULL)
	{
		delete TheBots;
		TheBots = NULL;
	}

	TheBots = new CCSBotManager;
}

// Engine callback for custom server commands

void Bot_ServerCommand()
{
	if (TheBots != NULL)
	{
		const char *pcmd = CMD_ARGV(0);
		TheBots->ServerCommand(pcmd);
	}
}

void Bot_RegisterCvars()
{
   if (g_bEnableCSBot)
   {
      CVAR_REGISTER (&cv_bot_traceview);
      CVAR_REGISTER (&cv_bot_stop);
      CVAR_REGISTER (&cv_bot_show_nav);
      CVAR_REGISTER (&cv_bot_show_danger);
      CVAR_REGISTER (&cv_bot_nav_edit);
      CVAR_REGISTER (&cv_bot_nav_zdraw);
      CVAR_REGISTER (&cv_bot_walk);
      CVAR_REGISTER (&cv_bot_difficulty);
      CVAR_REGISTER (&cv_bot_debug);
      CVAR_REGISTER (&cv_bot_quicksave);
      CVAR_REGISTER (&cv_bot_quota);
      CVAR_REGISTER (&cv_bot_quota_match);
      CVAR_REGISTER (&cv_bot_prefix);
      CVAR_REGISTER (&cv_bot_allow_rogues);
      CVAR_REGISTER (&cv_bot_allow_pistols);
      CVAR_REGISTER (&cv_bot_allow_shotguns);
      CVAR_REGISTER (&cv_bot_allow_sub_machine_guns);
      CVAR_REGISTER (&cv_bot_allow_rifles);
      CVAR_REGISTER (&cv_bot_allow_machine_guns);
      CVAR_REGISTER (&cv_bot_allow_grenades);
      CVAR_REGISTER (&cv_bot_allow_snipers);
      CVAR_REGISTER (&cv_bot_allow_shield);
      CVAR_REGISTER (&cv_bot_join_team);
      CVAR_REGISTER (&cv_bot_join_after_player);
      CVAR_REGISTER (&cv_bot_auto_vacate);
      CVAR_REGISTER (&cv_bot_zombie);
      CVAR_REGISTER (&cv_bot_defer_to_human);
      CVAR_REGISTER (&cv_bot_chatter);
      CVAR_REGISTER (&cv_bot_profile_db);
   }
}

// Constructor

CCSBot::CCSBot() : m_gameState(this), m_chatter(this)
{
	;
}

// Prepare bot for action

bool CCSBot::Initialize(const BotProfile *profile)
{
	// extend
	CBot::Initialize(profile);

	// CS bot initialization
	m_diedLastRound = false;
	m_morale = POSITIVE;		// starting a new round makes everyone a little happy

	m_combatRange = RANDOM_FLOAT(325, 425);

	m_navNodeList = NULL;
	m_currentNode = NULL;

	// set initial safe time guess for this map
	m_safeTime = 15.0f + 5.0f * GetProfile()->GetAggression();

	m_name[0] = '\0';

	ResetValues();
	StartNormalProcess();

	return true;
}

// Reset internal data to initial state

void CCSBot::ResetValues()
{
	m_chatter.Reset();
	m_gameState.Reset();

	m_avoid = NULL;
	m_avoidTimestamp = 0.0f;

	m_hurryTimer.Invalidate();

	m_isStuck = false;
	m_stuckTimestamp = 0.0f;
	m_wiggleTimestamp = 0.0f;
	m_stuckJumpTimestamp = 0.0f;

	m_pathLength = 0;
	m_pathIndex = 0;
	m_areaEnteredTimestamp = 0.0f;
	m_currentArea = NULL;
	m_lastKnownArea = NULL;

	m_avoidFriendTimer.Invalidate();
	m_isFriendInTheWay = false;
	m_isWaitingBehindFriend = false;

	m_disposition = ENGAGE_AND_INVESTIGATE;

	m_enemy = NULL;

	m_isWaitingToTossGrenade = false;
	m_wasSafe = true;

	m_nearbyEnemyCount = 0;
	m_enemyPlace = 0;
	m_nearbyFriendCount = 0;
	m_closestVisibleFriend = NULL;
	m_closestVisibleHumanFriend = NULL;

	for (size_t w = 0; w < ARRAYSIZE(m_watchInfo); ++w)
	{
		m_watchInfo[w].timestamp = 0.0f;
		m_watchInfo[w].isEnemy = false;
	}

	m_isEnemyVisible = false;
	m_visibleEnemyParts = NONE;
	m_lastSawEnemyTimestamp = 0.0f;
	m_firstSawEnemyTimestamp = 0.0f;
	m_currentEnemyAcquireTimestamp = 0.0f;
	m_isLastEnemyDead = true;
	m_attacker = NULL;
	m_attackedTimestamp = 0.0f;
	m_enemyDeathTimestamp = 0.0f;
	m_lastVictimID = 0;
	m_isAimingAtEnemy = false;
	m_fireWeaponTimestamp = 0.0f;
	m_equipTimer.Invalidate();

	m_isFollowing = false;
	m_leader = NULL;
	m_followTimestamp = 0.0f;
	m_allowAutoFollowTime = 0.0f;

	m_enemyQueueIndex = 0;
	m_enemyQueueCount = 0;
	m_enemyQueueAttendIndex = 0;
	m_bomber = NULL;

	m_lookAroundStateTimestamp = 0.0f;
	m_inhibitLookAroundTimestamp = 0.0f;

	m_lookPitch = 0.0f;
	m_lookPitchVel = 0.0f;
	m_lookYaw = 0.0f;
	m_lookYawVel = 0.0f;

	m_aimOffsetTimestamp = 0.0f;
	m_aimSpreadTimestamp = 0.0f;
	m_lookAtSpotState = NOT_LOOKING_AT_SPOT;

	m_spotEncounter = NULL;
	m_spotCheckTimestamp = 0.0f;
	m_peripheralTimestamp = 0.0f;

	m_avgVelIndex = 0;
	m_avgVelCount = 0;

	m_lastOrigin = (pev != NULL) ? pev->origin : Vector(0, 0, 0);

	m_lastRadioCommand = EVENT_INVALID;
	m_lastRadioRecievedTimestamp = 0.0f;
	m_lastRadioSentTimestamp = 0.0f;
	m_radioSubject = NULL;
	m_voiceFeedbackEndTimestamp = 0.0f;

	m_hostageEscortCount = 0;
	m_hostageEscortCountTimestamp = 0.0f;

	m_noisePosition = Vector(0, 0, 0);
	m_noiseTimestamp = 0.0f;
	m_noiseCheckTimestamp = 0.0f;
	m_isNoiseTravelRangeChecked = false;

	m_stateTimestamp = 0.0f;
	m_task = SEEK_AND_DESTROY;
	m_taskEntity = NULL;

	m_approachPointCount = 0;
	m_approachPointViewPosition = Vector(0, 0, 0);

	m_checkedHidingSpotCount = 0;
	m_isJumpCrouching = false;

	StandUp();
	Run();
	m_mustRunTimer.Invalidate();
	m_repathTimer.Invalidate();
	m_pathLadder = NULL;

	m_huntState.ClearHuntArea();

	// adjust morale - if we died, our morale decreased,
	// but if we live, no adjustement (round win/loss also adjusts morale
	if (m_diedLastRound)
		DecreaseMorale();

	m_diedLastRound = false;

	// IsRogue() randomly changes this
	m_isRogue = false;

	m_surpriseDelay = 0.0f;
	m_surpriseTimestamp = 0.0f;

	// even though these are EHANDLEs, they need to be NULL-ed
	m_goalEntity = NULL;
	m_avoid = NULL;
	m_enemy = NULL;

	for (int i = 0; i < MAX_ENEMY_QUEUE; ++i)
	{
		m_enemyQueue[i].player = NULL;
		m_enemyQueue[i].isReloading = false;
		m_enemyQueue[i].isProtectedByShield = false;
	}

	// start in idle state
	StopAttacking();
	Idle();
}

// Called when bot is placed in map, and when bots are reset after a round ends.
// NOTE: For some reason, this can be called twice when a bot is added.

void CCSBot::SpawnBot()
{
	CCSBotManager *ctrl = TheCSBots();

	ctrl->ValidateMapData();
	ResetValues();

	Q_strcpy(m_name, STRING(pev->netname));

	SetState(&m_buyState);
	SetTouch(&CCSBot::BotTouch);

	if (!TheNavAreaList.Count () && !ctrl->IsLearningMap())
	{
		ctrl->SetLearningMapFlag();
		StartLearnProcess();
	}
}

void CCSBot::RoundRespawn()
{
	// do the normal player spawn process
	CBasePlayer::RoundRespawn();
	EndVoiceFeedback();
}

void CCSBot::Disconnect()
{
	EndVoiceFeedback();

	if (m_processMode != PROCESS_NORMAL)
	{
		hideProgressMeter();
	}
}
