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

// Move towards currently heard noise

void InvestigateNoiseState::AttendCurrentNoise(CCSBot *me)
{
	if (!me->IsNoiseHeard() && me->GetNoisePosition())
		return;

	// remember where the noise we heard was
	m_checkNoisePosition = *me->GetNoisePosition();

	// tell our teammates (unless the noise is obvious, like gunfire)
	if (me->IsWellPastSafe() && me->HasNotSeenEnemyForLongTime() && me->GetNoisePriority() != PRIORITY_HIGH)
		me->GetChatter()->HeardNoise(me->GetNoisePosition());

	// figure out how to get to the noise
	me->PrintIfWatched("Attending to noise...\n");
	me->ComputePath(me->GetNoiseArea(), &m_checkNoisePosition, SAFEST_ROUTE);

	// consume the noise
	me->ForgetNoise();
}

void InvestigateNoiseState::OnEnter(CCSBot *me)
{
	AttendCurrentNoise(me);
}

// Use TravelDistance instead of distance...

void InvestigateNoiseState::OnUpdate(CCSBot *me)
{
	float newNoiseDist;
	if (me->ShouldInvestigateNoise(&newNoiseDist))
	{
		Vector toOldNoise = m_checkNoisePosition - me->pev->origin;
		const float muchCloserDist = 100.0f;
		if (toOldNoise.IsLengthGreaterThan(newNoiseDist + muchCloserDist))
		{
			// new sound is closer
			AttendCurrentNoise(me);
		}
	}

	// if the pathfind fails, give up
	if (!me->HasPath())
	{
		me->Idle();
		return;
	}

	// look around
	me->UpdateLookAround();

	// get distance remaining on our path until we reach the source of the noise
	float noiseDist = (m_checkNoisePosition - me->pev->origin).Length();

	if (me->IsUsingKnife())
	{
		if (me->IsHurrying())
			me->Run();
		else
			me->Walk();
	}
	else
	{
		const float closeToNoiseRange = 1500.0f;
		if (noiseDist < closeToNoiseRange)
		{
			// if we dont have many friends left, or we are alone, and we are near noise source, sneak quietly
			if (me->GetFriendsRemaining() <= 2 && !me->IsHurrying())
			{
				me->Walk();
			}
			else
			{
				me->Run();
			}
		}
		else
		{
			me->Run();
		}
	}

	// if we can see the noise position and we're close enough to it and looking at it,
	// we don't need to actually move there (it's checked enough)
	const float closeRange = 200.0f;
	if (noiseDist < closeRange)
	{
		if (me->IsLookingAtPosition(&m_checkNoisePosition) && me->IsVisible(&m_checkNoisePosition))
		{
			// can see noise position
			me->PrintIfWatched("Noise location is clear.\n");
			//me->ForgetNoise();
			me->Idle();
			return;
		}
	}

	// move towards noise
	if (me->UpdatePathMovement() != CCSBot::PROGRESSING)
	{
		me->Idle();
	}
}

void InvestigateNoiseState::OnExit(CCSBot *me)
{
	// reset to run mode in case we were sneaking about
	me->Run();
}
