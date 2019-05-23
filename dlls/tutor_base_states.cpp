
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

CBaseTutorStateSystem::CBaseTutorStateSystem()
{
	;
}

CBaseTutorStateSystem::~CBaseTutorStateSystem()
{
	;
}

int CBaseTutorStateSystem::GetCurrentStateType() const
{
	if (m_currentState != NULL)
	{
		return m_currentState->GetType();
	}

	return 0;
}

CBaseTutorState::CBaseTutorState()
{
	;
}

CBaseTutorState::~CBaseTutorState()
{
	;
}

int CBaseTutorState::GetType() const
{
	return m_type;
}

}
