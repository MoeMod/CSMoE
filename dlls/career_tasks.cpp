#include "maintypes.h"

#include "common.h"

#include "pm_math.h"
#include "pm_shared.h"

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"

#include "game.h"
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
CCareerTaskManager *TheCareerTasks = NULL;

const TaskInfo taskInfo[] =
{
	{ "defuse",			EVENT_BOMB_DEFUSED,		&CCareerTask::NewTask },
	{ "plant",			EVENT_BOMB_PLANTED,		&CCareerTask::NewTask },
	{ "rescue",			EVENT_HOSTAGE_RESCUED,		&CCareerTask::NewTask },
	{ "killall",			EVENT_KILL_ALL,			&CCareerTask::NewTask },
	{ "kill",			EVENT_KILL,			&CCareerTask::NewTask },
	{ "killwith",			EVENT_KILL,			&CCareerTask::NewTask },
	{ "killblind",			EVENT_KILL_FLASHBANGED,		&CCareerTask::NewTask },
	{ "killvip",			EVENT_KILL,			&CCareerTask::NewTask },
	{ "headshot",			EVENT_HEADSHOT,			&CCareerTask::NewTask },
	{ "headshotwith",		EVENT_HEADSHOT,			&CCareerTask::NewTask },
	{ "winfast",			EVENT_ROUND_WIN,		&CCareerTask::NewTask },
	{ "rescue",			EVENT_HOSTAGE_RESCUED,		&CCareerTask::NewTask },
	{ "rescueall",			EVENT_ALL_HOSTAGES_RESCUED,	&CCareerTask::NewTask },
	{ "injure",			EVENT_PLAYER_TOOK_DAMAGE,	&CCareerTask::NewTask },
	{ "injurewith",			EVENT_PLAYER_TOOK_DAMAGE,	&CCareerTask::NewTask },
	{ "killdefuser",		EVENT_KILL,			&CCareerTask::NewTask },
	{ "stoprescue",			EVENT_KILL,			&CCareerTask::NewTask },
	{ "defendhostages",		EVENT_ROUND_WIN,		&CCareerTask::NewTask },
	{ "hostagessurvive",		EVENT_ROUND_WIN,		&CCareerTask::NewTask },
	{ "preventdefuse",		EVENT_ROUND_WIN,		&CPreventDefuseTask::NewTask },
	{ NULL,				EVENT_INVALID,			&CCareerTask::NewTask }
};

CCareerTask *CPreventDefuseTask::NewTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete)
{
	CPreventDefuseTask *pNewTask = new CPreventDefuseTask(taskName, event, weaponName, n, mustLive, crossRounds, id, isComplete);

	pNewTask->m_bombPlantedThisRound = false;
	pNewTask->m_defuseStartedThisRound = false;

	return (CCareerTask *)pNewTask;
}

CPreventDefuseTask::CPreventDefuseTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete)
{
	CCareerTask(taskName, event, weaponName, n, mustLive, crossRounds, id, isComplete);

	m_bombPlantedThisRound = false;
	m_defuseStartedThisRound = false;
}

void CPreventDefuseTask::Reset()
{
	m_bombPlantedThisRound = false;
	m_defuseStartedThisRound = false;

	CCareerTask::Reset();
}

void CPreventDefuseTask::OnEvent(GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim)
{
	if (IsComplete())
		return;

	if (event == EVENT_BOMB_PLANTED)
	{
		m_bombPlantedThisRound = true;
	}
	else if (event == EVENT_BOMB_DEFUSING)
	{
		m_defuseStartedThisRound = true;
	}

	CCareerTask::OnEvent(event, pAttacker, pVictim);

	if (event >= EVENT_ROUND_DRAW && event <= EVENT_ROUND_LOSS)
	{
		m_bombPlantedThisRound = false;
		m_defuseStartedThisRound = false;
	}
}

CCareerTask *CCareerTask::NewTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete)
{
	CCareerTask *pTask = new CCareerTask(taskName, event, weaponName, n, mustLive, crossRounds, id, isComplete);
	return pTask;
}

CCareerTask::CCareerTask(const char *taskName, GameEventType event, const char *weaponName, int n, bool mustLive, bool crossRounds, int id, bool isComplete)
{
	m_isComplete = isComplete;
	m_event = event;
	m_eventsNeeded = n;

	m_name = taskName;
	m_eventsSeen = 0;
	m_mustLive = mustLive;
	m_crossRounds = crossRounds;
	m_diedThisRound = false;
	m_id = id;
	m_weaponId = AliasToWeaponID(weaponName);
	m_weaponClassId = AliasToWeaponClass(weaponName);

	m_rescuer = (Q_stricmp(taskName, "stoprescue") == 0);
	m_defuser = (Q_stricmp(taskName, "killdefuser") == 0);
	m_vip = (Q_stricmp(taskName, "killvip") == 0);

	if (event == EVENT_ALL_HOSTAGES_RESCUED)
	{
		m_mustLive = true;
		m_crossRounds = false;
	}

	if (m_isComplete)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
			WRITE_STRING("TASKDONE");
			WRITE_BYTE(m_id);
		MESSAGE_END();
	}
}

void CCareerTask::Reset()
{
	m_eventsSeen = 0;
	m_isComplete = false;

	MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
		WRITE_STRING("TASKUNDONE");
		WRITE_BYTE(m_id);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
		WRITE_STRING("TASKPART");
		WRITE_BYTE(m_id);
		WRITE_SHORT(m_eventsSeen);
	MESSAGE_END();
}

void CCareerTask::SendPartialNotification()
{
	MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
		WRITE_STRING("TASKPART");
		WRITE_BYTE(m_id);
		WRITE_SHORT(m_eventsSeen);
	MESSAGE_END();

	UTIL_LogPrintf("Career Task Partial %d %d\n", m_id, m_eventsSeen);
}

void CCareerTask::OnWeaponKill(int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim)
{
	if( m_isComplete || ( m_event != EVENT_KILL && ( m_event != EVENT_HEADSHOT || !headshot ) ) )
	{
		return;
	}

	if (!pVictim || (m_defuser && !pVictim->m_bIsDefusing) || (m_vip && !pVictim->m_bIsVIP))
	{
		return;
	}

	if (m_rescuer)
	{
		int hostages_ = 0;
		CBaseEntity *hostageEntity = NULL;

		while ((hostageEntity = UTIL_FindEntityByClassname(hostageEntity, "hostage_entity")) != NULL)
		{
			if (hostageEntity->pev->takedamage != DAMAGE_YES)
				continue;

			CHostage *hostage = static_cast<CHostage *>(hostageEntity);

			if (!hostage->IsFollowingSomeone())
				continue;

			if (hostage->IsValid() && hostage->m_target == pAttacker)
				++hostages_;
		}

		if (!hostages_)
			return;
	}

	if (m_weaponId == WEAPON_SHIELDGUN)
	{
		if (!killerHasShield)
			return;
	}
	else if (m_weaponId)
	{
		if (m_weaponId != weaponId)
			return;
	}
	else if (m_weaponClassId)
	{
		if (m_weaponClassId != weaponClassId)
			return;
	}

	++m_eventsSeen;
	SendPartialNotification();
}

void CCareerTask::OnWeaponInjury(int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker)
{
	if (m_isComplete || m_event != EVENT_PLAYER_TOOK_DAMAGE)
	{
		return;
	}

	if (m_weaponId == WEAPON_SHIELDGUN)
	{
		if (!attackerHasShield)
			return;
	}
	else if (m_weaponId)
	{
		if (m_weaponId != weaponId)
			return;
	}
	else if (m_weaponClassId)
	{
		if (m_weaponClassId != weaponClassId)
			return;
	}

	++m_eventsSeen;
	SendPartialNotification();
}

void CCareerTask::OnEvent(GameEventType event, CBasePlayer *pVictim, CBasePlayer *pAttacker)
{
	if (m_isComplete)
		return;

	if (event == m_event)
	{
		if ((m_defuser && !pAttacker->m_bIsDefusing) || (m_vip && !pAttacker->m_bIsVIP))
			return;

		if (m_rescuer)
		{
			int hostages_ = 0;
			CBaseEntity *hostageEntity = NULL;

			while ((hostageEntity = UTIL_FindEntityByClassname(hostageEntity, "hostage_entity")) != NULL)
			{
				if (hostageEntity->pev->takedamage != DAMAGE_YES)
					continue;

				CHostage *hostage = static_cast<CHostage *>(hostageEntity);

				if (!hostage->IsFollowingSomeone())
					continue;

				if (hostage->IsValid() && hostage->m_target == pAttacker)
					++hostages_;
			}

			if (!hostages_)
			{
				return;
			}
		}

		if( m_event != EVENT_KILL || ( !m_weaponId && !m_weaponClassId
			&& m_event != EVENT_HEADSHOT ) || ( !m_weaponId && !m_weaponClassId
			&& m_event != EVENT_PLAYER_TOOK_DAMAGE ) || ( !m_weaponId && !m_weaponClassId ) )
		{
			if (m_event == EVENT_ROUND_WIN)
			{
				if (!Q_strcmp(m_name, "defendhostages"))
				{
					int hostages_ = 0;
					CBaseEntity *hostageEntity = NULL;

					while ((hostageEntity = UTIL_FindEntityByClassname(hostageEntity, "hostage_entity")) != NULL)
					{
						if (hostageEntity->pev->takedamage != 1.0f && hostageEntity->pev->deadflag != DEAD_DEAD)
							++hostages_;
					}

					if (!hostages_)
					{
						++m_eventsSeen;
						SendPartialNotification();
					}
				}
				else if (!Q_strcmp(m_name, "hostagessurvive"))
				{
					int hostages_ = 0;
					CBaseEntity *hostageEntity = NULL;

					while ((hostageEntity = UTIL_FindEntityByClassname(hostageEntity, "hostage_entity")) != NULL)
					{
						CHostage *hostage = (CHostage *)hostageEntity;

						if (hostage && hostage->IsDead())
							++hostages_;
					}

					if (!hostages_)
					{
						++m_eventsSeen;
						SendPartialNotification();
					}
				}
				else if (!Q_strcmp(m_name, "winfast"))
				{
					if (m_eventsNeeded >= TheCareerTasks->GetRoundElapsedTime())
					{
						m_eventsSeen = m_eventsNeeded;
						SendPartialNotification();
					}
				}
				else if (IsTaskCompletableThisRound())
				{
					++m_eventsSeen;
					SendPartialNotification();
				}
			}
			else
			{
				++m_eventsSeen;
				SendPartialNotification();
			}
		}
	}

	if (event == m_event && !m_mustLive && m_eventsSeen >= m_eventsNeeded && IsTaskCompletableThisRound())
	{
		CBasePlayer *player = UTIL_GetLocalPlayer();
		EMIT_SOUND(ENT(player->pev), CHAN_VOICE, "events/task_complete.wav", VOL_NORM, ATTN_NORM);

		m_isComplete = true;
		MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
			WRITE_STRING("TASKDONE");
			WRITE_BYTE(m_id);
		MESSAGE_END();

		if (TheTutor != NULL)
		{
			TheTutor->OnEvent(EVENT_CAREER_TASK_DONE);
		}

		UTIL_LogPrintf("Career Task Done %d\n", m_id);

		if (m_event == EVENT_ROUND_WIN && !Q_strcmp(m_name, "winfast"))
		{
			TheCareerTasks->SetFinishedTaskTime((int)TheCareerTasks->GetRoundElapsedTime());
			UTIL_GetLocalPlayer()->SyncRoundTimer();
		}
	}
	else if (event >= EVENT_ROUND_DRAW)
	{
		if (event > EVENT_ROUND_LOSS)
		{
			if (event == EVENT_DIE && (m_mustLive || m_crossRounds))
			{
				m_eventsSeen = 0;
				SendPartialNotification();
				m_diedThisRound = true;
			}
		}
		else if (m_mustLive)
		{
			if (m_eventsSeen >= m_eventsNeeded && !m_diedThisRound && IsTaskCompletableThisRound())
			{
				CBasePlayer *player = UTIL_GetLocalPlayer();
				EMIT_SOUND(ENT(player->pev), CHAN_VOICE, "events/task_complete.wav", VOL_NORM, ATTN_NORM);

				m_isComplete = true;
				MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
					WRITE_STRING("TASKDONE");
					WRITE_BYTE(m_id);
				MESSAGE_END();

				UTIL_LogPrintf("Career Task Done %d\n", m_id);

				if (m_event == EVENT_ROUND_WIN && !Q_strcmp(m_name, "winfast"))
				{
					TheCareerTasks->SetFinishedTaskTime((int)TheCareerTasks->GetRoundElapsedTime());
					UTIL_GetLocalPlayer()->SyncRoundTimer();
				}

				if (TheTutor != NULL)
				{
					TheTutor->OnEvent(EVENT_CAREER_TASK_DONE);
				}
			}

			m_diedThisRound = false;

			if (m_mustLive)
			{
				m_eventsSeen = 0;
				SendPartialNotification();
			}
		}
	}
}

void CCareerTaskManager::Create()
{
	if (TheCareerTasks != NULL)
	{
		TheCareerTasks->Reset();
		return;
	}

	TheCareerTasks = new CCareerTaskManager;
}

CCareerTaskManager::CCareerTaskManager()
{
	m_taskTime = 0;
	Reset();
}

void CCareerTaskManager::Reset(bool deleteTasks)
{
	if (deleteTasks)
	{
		m_tasks.PurgeAndDeleteElements ();
		m_nextId = 0;
	}
	else
	{
		FOR_EACH_LL (m_tasks, it)
		{
			CCareerTask *pTask = m_tasks[it];
			pTask->Reset();
		}
	}

	m_finishedTaskTime  = 0;
	m_finishedTaskRound = 0;
	m_shouldLatchRoundEndMessage = false;

	m_roundStartTime = gpGlobals->time + CVAR_GET_FLOAT("mp_freezetime");
}

void CCareerTaskManager::SetFinishedTaskTime(int val)
{
	CHalfLifeMultiplay *mp = g_pGameRules;

	m_finishedTaskTime = val;
	m_finishedTaskRound = mp->m_iTotalRoundsPlayed;
}

void CCareerTaskManager::AddTask(const char *taskName, const char *weaponName, int eventCount, bool mustLive, bool crossRounds, bool isComplete)
{
	++m_nextId;

	for (int i = 0; i < ARRAYSIZE(taskInfo); ++i)
	{
		const TaskInfo *pTaskInfo = &taskInfo[ i ];

		if (pTaskInfo->taskName != NULL)
		{
			if (!Q_stricmp(pTaskInfo->taskName, taskName))
			{
				CCareerTask *newTask = pTaskInfo->factory
				(
					pTaskInfo->taskName,
					pTaskInfo->event,
					weaponName,
					eventCount,
					mustLive,
					crossRounds,
					m_nextId,
					isComplete
				);

				m_tasks.AddToTail (newTask);

				if (pTaskInfo->event == EVENT_ROUND_WIN && !Q_strcmp(taskName, "winfast"))
				{
					m_taskTime = eventCount;

					if (isComplete)
					{
						m_finishedTaskTime = eventCount;
					}
				}

				return;
			}
		}
	}

	MESSAGE_BEGIN(MSG_ALL, gmsgCZCareer);
		WRITE_STRING("TASKDONE");
		WRITE_BYTE(m_nextId);
	MESSAGE_END();
}

void CCareerTaskManager::HandleEvent(GameEventType event, CBasePlayer *pAttacker, CBasePlayer *pVictim)
{
	if (event == EVENT_ROUND_START)
	{
		m_roundStartTime = gpGlobals->time;
		return;
	}

	if ((event <= EVENT_ROUND_LOSS && event >= EVENT_ROUND_DRAW) && m_shouldLatchRoundEndMessage)
	{
		m_roundEndMessage = event;
		return;
	}

	FOR_EACH_LL (m_tasks, it)
	{
		m_tasks[it]->OnEvent(event, pAttacker, pVictim);
	}
}

void CCareerTaskManager::HandleWeaponKill(int weaponId, int weaponClassId, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim)
{
	FOR_EACH_LL (m_tasks, it)
	{
		m_tasks[it]->OnWeaponKill(weaponId, weaponClassId, headshot, killerHasShield, pAttacker, pVictim);
	}
}

void CCareerTaskManager::HandleEnemyKill(bool wasBlind, const char *weaponName, bool headshot, bool killerHasShield, CBasePlayer *pAttacker, CBasePlayer *pVictim)
{
	HandleWeaponKill(AliasToWeaponID(weaponName), AliasToWeaponClass(weaponName), headshot, killerHasShield, pAttacker, pVictim);

	HandleEvent(EVENT_KILL, pAttacker, pVictim);

	if (headshot)
	{
		HandleEvent(EVENT_HEADSHOT, pAttacker, pVictim);
	}
	if (wasBlind)
	{
		HandleEvent(EVENT_KILL_FLASHBANGED, pAttacker, pVictim);
	}
}

void CCareerTaskManager::HandleWeaponInjury(int weaponId, int weaponClassId, bool attackerHasShield, CBasePlayer *pAttacker)
{
	FOR_EACH_LL (m_tasks, it)
	{
		m_tasks[it]->OnWeaponInjury(weaponId, weaponClassId, attackerHasShield, pAttacker);
	}
}

void CCareerTaskManager::HandleEnemyInjury(const char *weaponName, bool attackerHasShield, CBasePlayer *pAttacker)
{
	HandleWeaponInjury(AliasToWeaponID(weaponName), AliasToWeaponClass(weaponName), attackerHasShield, pAttacker);
	HandleEvent(EVENT_PLAYER_TOOK_DAMAGE);
}

void CCareerTaskManager::HandleDeath(int team, CBasePlayer *pAttacker)
{
	int enemyTeam = (Q_strcmp(humans_join_team.string, "CT") != 0) ? CT : TERRORIST;
	int numEnemies = 0;

	if (enemyTeam != team)
		return;

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (pPlayer && pPlayer->m_iTeam == enemyTeam && pPlayer->IsAlive())
			++numEnemies;
	}

	if (!numEnemies)
	{
		HandleEvent(EVENT_KILL_ALL);
	}
}

bool CCareerTaskManager::AreAllTasksComplete()
{
	FOR_EACH_LL (m_tasks, it)
	{
		if (!m_tasks[it]->IsComplete())
			return false;
	}

	return true;
}

int CCareerTaskManager::GetNumRemainingTasks()
{
	int ret = 0;
	FOR_EACH_LL (m_tasks, it)
	{
		if (!m_tasks[it]->IsComplete ())
			ret++;
	}

	return ret;
}

float CCareerTaskManager::GetRoundElapsedTime()
{
	return (gpGlobals->time - m_roundStartTime);
}

void CCareerTaskManager::LatchRoundEndMessage()
{
	m_shouldLatchRoundEndMessage = true;
}

void CCareerTaskManager::UnlatchRoundEndMessage()
{
	m_shouldLatchRoundEndMessage = false;
	HandleEvent(m_roundEndMessage);
}
