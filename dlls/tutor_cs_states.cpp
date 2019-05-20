#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "globals.h"
#include "game.h"
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

namespace sv {

/*
* Globals initialization
*/
const char *const g_TutorStateStrings[20] =
{
	"#Cstrike_TutorState_Undefined",
	"#Cstrike_TutorState_Looking_For_Hostage",
	"#Cstrike_TutorState_Escorting_Hostage",
	"#Cstrike_TutorState_Following_Hostage_Escort",
	"#Cstrike_TutorState_Moving_To_Bombsite",
	"#Cstrike_TutorState_Looking_For_Bomb_Carrier",
	"#Cstrike_TutorState_Guarding_Loose_Bomb",
	"#Cstrike_TutorState_Defusing_Bomb",
	"#Cstrike_TutorState_Guarding_Hostage",
	"#Cstrike_TutorState_Moving_To_Intercept_Enemy",
	"#Cstrike_TutorState_Looking_For_Hostage_Escort",
	"#Cstrike_TutorState_Attacking_Hostage_Escort",
	"#Cstrike_TutorState_Escorting_Bomb_Carrier",
	"#Cstrike_TutorState_Moving_To_Bomb_Site",
	"#Cstrike_TutorState_Planting_Bomb",
	"#Cstrike_TutorState_Guarding_Bomb",
	"#Cstrike_TutorState_Looking_For_Loose_Bomb",
	"#Cstrike_TutorState_Running_Away_From_Ticking_Bomb",
	"#Cstrike_TutorState_Buy_Time",
	"#Cstrike_TutorState_Waiting_For_Start"
};

CCSTutorStateSystem::CCSTutorStateSystem()
{
	m_currentState = new CCSTutorUndefinedState;
}

CCSTutorStateSystem::~CCSTutorStateSystem()
{
	if (m_currentState != NULL)
	{
		delete m_currentState;
		m_currentState = NULL;
	}
}

bool CCSTutorStateSystem::UpdateState(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	if (m_currentState == NULL)
	{
		m_currentState = new CCSTutorUndefinedState;
	}

	if (m_currentState != NULL)
	{
		TutorStateType nextStateType = static_cast<TutorStateType>(m_currentState->CheckForStateTransition(event, entity, other));

		if (nextStateType != TUTORSTATE_UNDEFINED)
		{
			if (m_currentState != NULL)
			{
				delete m_currentState;
			}

			m_currentState = NULL;
			m_currentState = ConstructNewState(nextStateType);

			return true;
		}
	}

	return false;
}

const char * CCSTutorStateSystem::GetCurrentStateString()
{
	if (m_currentState != NULL)
	{
		return m_currentState->GetStateString();
	}

	return NULL;
}

CBaseTutorState *CCSTutorStateSystem::ConstructNewState(int stateType)
{
	CBaseTutorState *ret = NULL;

	if (stateType != TUTORSTATE_UNDEFINED)
	{
		if (stateType == TUTORSTATE_BUYTIME)
		{
			ret = new CCSTutorBuyMenuState;
		}
		else if (stateType == TUTORSTATE_WAITING_FOR_START)
		{
			ret = new CCSTutorWaitingForStartState;
		}
	}
	else
	{
		ret = new CCSTutorUndefinedState;
	}

	return ret;
}

CCSTutorUndefinedState::CCSTutorUndefinedState()
{
	m_type = 0;
}

CCSTutorUndefinedState::~CCSTutorUndefinedState()
{
	;
}

int CCSTutorUndefinedState::CheckForStateTransition(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	if (event == EVENT_PLAYER_SPAWNED)
	{
		return HandlePlayerSpawned(entity, other);
	}

	return 0;
}

int CCSTutorUndefinedState::HandlePlayerSpawned(CBaseEntity *entity, CBaseEntity *other)
{
	CBasePlayer *localPlayer = UTIL_GetLocalPlayer();

	if (localPlayer != NULL)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);

		if (player != NULL && player->IsPlayer() && player == localPlayer)
		{
			// flags
			return TUTORSTATE_WAITING_FOR_START;
		}
	}

	return 0;
}

const char * CCSTutorUndefinedState::GetStateString()
{
	return NULL;
}

CCSTutorWaitingForStartState::CCSTutorWaitingForStartState()
{
	m_type = TUTORSTATE_WAITING_FOR_START;
}

CCSTutorWaitingForStartState::~CCSTutorWaitingForStartState()
{
	;
}

int CCSTutorWaitingForStartState::CheckForStateTransition(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	switch (event)
	{
	case EVENT_PLAYER_SPAWNED:
		return HandlePlayerSpawned(entity, other);
	case EVENT_BUY_TIME_START:
		return HandleBuyTimeStart(entity, other);
	default:
		break;
	}

	return 0;
}

const char * CCSTutorWaitingForStartState::GetStateString()
{
	if (m_type < TUTORSTATE_UNDEFINED || m_type > TUTORSTATE_WAITING_FOR_START)
	{
		return nullptr;
	}
	return g_TutorStateStrings[m_type];
}

int CCSTutorWaitingForStartState::HandlePlayerSpawned(CBaseEntity *entity, CBaseEntity *other)
{
	CBasePlayer *localPlayer = UTIL_GetLocalPlayer();

	if (localPlayer != NULL)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);

		if (player != NULL && player->IsPlayer() && player == localPlayer)
		{
			// flags
			return TUTORSTATE_WAITING_FOR_START;
		}
	}

	return 0;
}

int CCSTutorWaitingForStartState::HandleBuyTimeStart(CBaseEntity *entity, CBaseEntity *other)
{
	return TUTORSTATE_BUYTIME;
}

CCSTutorBuyMenuState::CCSTutorBuyMenuState()
{
	m_type = TUTORSTATE_BUYTIME;
}

CCSTutorBuyMenuState::~CCSTutorBuyMenuState()
{
	;
}

int CCSTutorBuyMenuState::CheckForStateTransition(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	if (event == EVENT_ROUND_START)
	{
		return HandleRoundStart(entity, other);
	}

	return 0;
}

const char * CCSTutorBuyMenuState::GetStateString()
{
	if (m_type < TUTORSTATE_UNDEFINED || m_type > TUTORSTATE_WAITING_FOR_START)
	{
		return nullptr;
	}
	return g_TutorStateStrings[m_type];
}

int CCSTutorBuyMenuState::HandleRoundStart(CBaseEntity *entity, CBaseEntity *other)
{
	return TUTORSTATE_WAITING_FOR_START;
}

}
