#include "maintypes.h"
#include "common.h"

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

// Move to the bomb on the floor and pick it up

void FetchBombState::OnEnter(CCSBot *me)
{
	me->DestroyPath();
}

// Move to the bomb on the floor and pick it up

void FetchBombState::OnUpdate(CCSBot *me)
{
	if (me->IsCarryingBomb())
	{
		me->PrintIfWatched( "I picked up the bomb\n" );
		me->Idle();
		return;
	}

	CBaseEntity *bomb = TheCSBots()->GetLooseBomb();
	if (bomb != NULL)
	{
		if (!me->HasPath())
		{
			// build a path to the bomb
			if (me->ComputePath(TheNavAreaGrid.GetNavArea(&bomb->pev->origin), &bomb->pev->origin, SAFEST_ROUTE) == false)
			{
				me->PrintIfWatched("Fetch bomb pathfind failed\n");

				// go Hunt instead of Idle to prevent continuous re-pathing to inaccessible bomb
				me->Hunt();
				//return;
			}
		}
	}
	else
	{
		// someone picked up the bomb
		me->PrintIfWatched("Bomb not loose\n");
		me->Idle();
		return;
	}

	// look around
	me->UpdateLookAround();

	if (me->UpdatePathMovement() != CCSBot::PROGRESSING)
		me->Idle();
}
