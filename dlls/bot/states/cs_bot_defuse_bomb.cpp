

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

namespace sv {

// Begin defusing the bomb

void DefuseBombState::OnEnter(CCSBot *me)
{
	me->Crouch();
	me->SetDisposition(CCSBot::SELF_DEFENSE);
	me->GetChatter()->Say("DefusingBomb");
}

// Defuse the bomb

void DefuseBombState::OnUpdate(CCSBot *me)
{
	const Vector *bombPos = me->GetGameState()->GetBombPosition();
	CCSBotManager *ctrl = TheCSBots();

	if (bombPos == NULL)
	{
		me->PrintIfWatched("In Defuse state, but don't know where the bomb is!\n");
		me->Idle();
		return;
	}

	// look at the bomb
	me->SetLookAt("Defuse bomb", bombPos, PRIORITY_HIGH);

	// defuse...
	me->UseEnvironment();

	if (gpGlobals->time - me->GetStateTimestamp() > 1.0f)
	{
		// if we missed starting the defuse, give up
		if (ctrl->GetBombDefuser() == NULL)
		{
			me->PrintIfWatched("Failed to start defuse, giving up\n");
			me->Idle();
			return;
		}
		else if (ctrl->GetBombDefuser() != me)
		{
			// if someone else got the defuse, give up
			me->PrintIfWatched("Someone else started defusing, giving up\n");
			me->Idle();
			return;
		}
	}

	// if bomb has been defused, give up
	if (!ctrl->IsBombPlanted())
	{
		me->Idle();
		return;
	}
}

void DefuseBombState::OnExit(CCSBot *me)
{
	me->StandUp();
	me->ResetStuckMonitor();
	me->SetTask(CCSBot::SEEK_AND_DESTROY);
	me->SetDisposition(CCSBot::ENGAGE_AND_INVESTIGATE);
	me->ClearLookAt();
}

}
