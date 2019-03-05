
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

// Begin moving to a nearby hidey-hole.
// NOTE: Do not forget this state may include a very long "move-to" time to get to our hidey spot!

void HideState::OnEnter(CCSBot *me)
{
	m_isAtSpot = false;

	// if duration is "infinite", set it to a reasonably long time to prevent infinite camping
	if (m_duration < 0.0f)
	{
		m_duration = RANDOM_FLOAT(30.0f, 60.0f);
	}

	// decide whether to "ambush" or not - never set to false so as not to override external setting
	if (RANDOM_FLOAT(0.0f, 100.0f) < 50.0f)
	{
		m_isHoldingPosition = true;
	}

	// if we are holding position, decide for how long
	if (m_isHoldingPosition)
	{
		m_holdPositionTime = RANDOM_FLOAT(3.0f, 10.0f);
	}
	else
	{
		m_holdPositionTime = 0.0f;
	}

	m_heardEnemy = false;
	m_firstHeardEnemyTime = 0.0f;
	m_retry = 0;

	if (me->IsFollowing())
	{
		m_leaderAnchorPos = me->GetFollowLeader()->pev->origin;
	}
}

// Move to a nearby hidey-hole.
// NOTE: Do not forget this state may include a very long "move-to" time to get to our hidey spot!

void HideState::OnUpdate(CCSBot *me)
{
	CCSBotManager *ctrl = TheCSBots();

	// wait until finished reloading to leave hide state
	if (!me->IsActiveWeaponReloading())
	{
		if (gpGlobals->time - me->GetStateTimestamp() > m_duration)
		{
			if (me->GetTask() == CCSBot::GUARD_LOOSE_BOMB)
			{
				// if we're guarding the loose bomb, continue to guard it but pick a new spot
				me->Hide(ctrl->GetLooseBombArea());
				return;
			}
			else if (me->GetTask() == CCSBot::GUARD_BOMB_ZONE)
			{
				// if we're guarding a bombsite, continue to guard it but pick a new spot
				const CCSBotManager::Zone *zone = ctrl->GetClosestZone(&me->pev->origin);
				if (zone != NULL)
				{
					CNavArea *area = ctrl->GetRandomAreaInZone(zone);
					if (area != NULL)
					{
						me->Hide(area);
						return;
					}
				}
			}
			else if (me->GetTask() == CCSBot::GUARD_HOSTAGE_RESCUE_ZONE)
			{
				// if we're guarding a rescue zone, continue to guard this or another rescue zone
				if (me->GuardRandomZone())
				{
					me->SetTask(CCSBot::GUARD_HOSTAGE_RESCUE_ZONE);
					me->PrintIfWatched("Continuing to guard hostage rescue zones\n");
					me->SetDisposition(CCSBot::OPPORTUNITY_FIRE);
					me->GetChatter()->GuardingHostageEscapeZone(IS_PLAN);
					return;
				}
			}

			me->Idle();
			return;
		}

		// if we are momentarily hiding while following someone, check to see if he has moved on
		if (me->IsFollowing())
		{
			CBasePlayer *leader = me->GetFollowLeader();

			// BOTPORT: Determine walk/run velocity thresholds
			float runThreshold = 200.0f;
			if (leader->pev->velocity.IsLengthGreaterThan(runThreshold))
			{
				// leader is running, stay with him
				me->Follow(leader);
				return;
			}

			// if leader has moved, stay with him
			const float followRange = 250.0f;
			if ((m_leaderAnchorPos - leader->pev->origin).IsLengthGreaterThan(followRange))
			{
				me->Follow(leader);
				return;
			}
		}

		// if we see a nearby buddy in combat, join him
		// TODO: Perhaps tie in to TakeDamage(), so it works for human players, too

		// Scenario logic
		switch (ctrl->GetScenario())
		{
			case CCSBotManager::SCENARIO_DEFUSE_BOMB:
			{
				if (me->m_iTeam == CT)
				{
					// if we are just holding position (due to a radio order) and the bomb has just planted, go defuse it
					if (me->GetTask() == CCSBot::HOLD_POSITION &&
						ctrl->IsBombPlanted() &&
						ctrl->GetBombPlantTimestamp() > me->GetStateTimestamp())
					{
						me->Idle();
						return;
					}

					// if we are guarding the defuser and he dies/gives up, stop hiding (to choose another defuser)
					if (me->GetTask() == CCSBot::GUARD_BOMB_DEFUSER && ctrl->GetBombDefuser() == NULL)
					{
						me->Idle();
						return;
					}

					// if we are guarding the loose bomb and it is picked up, stop hiding
					if (me->GetTask() == CCSBot::GUARD_LOOSE_BOMB && ctrl->GetLooseBomb() == NULL)
					{
						me->GetChatter()->TheyPickedUpTheBomb();
						me->Idle();
						return;
					}

					// if we are guarding a bombsite and the bomb is dropped and we hear about it, stop guarding
					if (me->GetTask() == CCSBot::GUARD_BOMB_ZONE && me->GetGameState()->IsLooseBombLocationKnown())
					{
						me->Idle();
						return;
					}

					// if we are guarding (bombsite, initial encounter, etc) and the bomb is planted, go defuse it
					if (me->IsDoingScenario() && me->GetTask() == CCSBot::GUARD_BOMB_ZONE && ctrl->IsBombPlanted())
					{
						me->Idle();
						return;
					}

				}
				// TERRORIST
				else
				{
					// if we are near the ticking bomb and someone starts defusing it, attack!
					if (ctrl->GetBombDefuser())
					{
						Vector toDefuser = ctrl->GetBombDefuser()->pev->origin;
						const float hearDefuseRange = 2000.0f;
						if ((toDefuser - me->pev->origin).IsLengthLessThan(hearDefuseRange))
						{
							// if we are nearby, attack, otherwise move to the bomb (which will cause us to attack when we see defuser)
							if (me->CanSeePlantedBomb())
							{
								me->Attack(ctrl->GetBombDefuser());
							}
							else
							{
								me->MoveTo(&toDefuser, FASTEST_ROUTE);
								me->InhibitLookAround(10.0f);
							}

							return;
						}
					}
				}
				break;
			}
			case CCSBotManager::SCENARIO_RESCUE_HOSTAGES:
			{
				// if we're guarding the hostages and they all die or are taken, do something else
				if (me->GetTask() == CCSBot::GUARD_HOSTAGES)
				{
					if (me->GetGameState()->AreAllHostagesBeingRescued() || me->GetGameState()->AreAllHostagesGone())
					{
						me->Idle();
						return;
					}
				}
				else if (me->GetTask() == CCSBot::GUARD_HOSTAGE_RESCUE_ZONE)
				{
					// if we stumble across a hostage, guard it
					CHostage *hostage = me->GetGameState()->GetNearestVisibleFreeHostage();
					if (hostage != NULL)
					{
						// we see a free hostage, guard it
						CNavArea *area = TheNavAreaGrid.GetNearestNavArea(&hostage->pev->origin);
						if (area != NULL)
						{
							me->SetTask(CCSBot::GUARD_HOSTAGES);
							me->Hide(area);
							me->PrintIfWatched("I'm guarding hostages I found\n");
							// don't chatter here - he'll tell us when he's in his hiding spot
							return;
						}
					}
				}
			}
			default:
				break;
		}

		bool isSettledInSniper = (me->IsSniper() && m_isAtSpot) ? true : false;

		// only investigate noises if we are initiating attacks, and we aren't a "settled in" sniper
		// dont investigate noises if we are reloading
		if (!me->IsActiveWeaponReloading() &&
			!isSettledInSniper &&
			me->GetDisposition() == CCSBot::ENGAGE_AND_INVESTIGATE)
		{
			// if we are holding position, and have heard the enemy nearby, investigate after our hold time is up
			if (m_isHoldingPosition && m_heardEnemy && (gpGlobals->time - m_firstHeardEnemyTime > m_holdPositionTime))
			{
				// TODO: We might need to remember specific location of last enemy noise here
				me->InvestigateNoise();
				return;
			}

			// investigate nearby enemy noises
			if (me->ShouldInvestigateNoise())
			{
				// if we are holding position, check if enough time has elapsed since we first heard the enemy
				if (m_isAtSpot && m_isHoldingPosition)
				{
					if (!m_heardEnemy)
					{
						// first time we heard the enemy
						m_heardEnemy = true;
						m_firstHeardEnemyTime = gpGlobals->time;
						me->PrintIfWatched("Heard enemy, holding position for %f2.1 seconds...\n", m_holdPositionTime);
					}
				}
				else
				{
					// not holding position - investigate enemy noise
					me->InvestigateNoise();
					return;
				}
			}
		}
	}

	// look around
	me->UpdateLookAround();

	// if we are at our hiding spot, crouch and wait
	if (m_isAtSpot)
	{
		me->Crouch();

		// if we have a shield, hide behind it
		if (me->HasShield() && !me->IsProtectedByShield())
			me->SecondaryAttack();

		// while sitting at our hiding spot, if we are being attacked but can't see our attacker, move somewhere else
		const float hurtRecentlyTime = 1.0f;
		if (!me->IsEnemyVisible() && me->GetTimeSinceAttacked() < hurtRecentlyTime)
		{
			me->Idle();
			return;
		}

		// encourage the human player
		if (!me->IsDoingScenario())
		{
			if (me->m_iTeam == CT)
			{
				if (me->GetTask() == CCSBot::GUARD_BOMB_ZONE && me->IsAtHidingSpot() && ctrl->IsBombPlanted())
				{
					if (me->GetNearbyEnemyCount() == 0)
					{
						const float someTime = 30.0f;
						const float littleTime = 11.0;

						if (ctrl->GetBombTimeLeft() > someTime)
							me->GetChatter()->Encourage("BombsiteSecure", RANDOM_FLOAT(10.0f, 15.0f));
						else if (ctrl->GetBombTimeLeft() > littleTime)
							me->GetChatter()->Encourage("WaitingForHumanToDefuseBomb", RANDOM_FLOAT(5.0f, 8.0f));
						else
							me->GetChatter()->Encourage("WaitingForHumanToDefuseBombPanic", RANDOM_FLOAT(3.0f, 4.0f));
					}
				}

				if (me->GetTask() == CCSBot::GUARD_HOSTAGES && me->IsAtHidingSpot())
				{
					if (me->GetNearbyEnemyCount() == 0)
					{
						CHostage *hostage = me->GetGameState()->GetNearestVisibleFreeHostage();
						if (hostage != NULL)
						{
							me->GetChatter()->Encourage("WaitingForHumanToRescueHostages", RANDOM_FLOAT(10.0f, 15.0f));
						}
					}
				}
			}
		}
	}
	else
	{
		// if a Player is using this hiding spot, give up
		float range;
		CBasePlayer *camper = UTIL_GetClosestPlayer(&m_hidingSpot, &range);

		const float closeRange = 75.0f;
		if (camper != NULL && camper != me && range < closeRange && me->IsVisible(camper, CHECK_FOV))
		{
			// player is in our hiding spot
			me->PrintIfWatched("Someone's in my hiding spot - picking another...\n");

			const int maxRetries = 3;
			if (m_retry++ >= maxRetries)
			{
				me->PrintIfWatched("Can't find a free hiding spot, giving up.\n");
				me->Idle();
				return;
			}

			// pick another hiding spot near where we were planning on hiding
			me->Hide(TheNavAreaGrid.GetNavArea(&m_hidingSpot));
			return;
		}

		Vector toSpot;
		toSpot.x = m_hidingSpot.x - me->pev->origin.x;
		toSpot.y = m_hidingSpot.y - me->pev->origin.y;
		toSpot.z = m_hidingSpot.z - me->GetFeetZ(); // use feet location
		float dist = toSpot.Length();

		const float crouchDist = 200.0f;
		if (dist < crouchDist)
			me->Crouch();

		const float atDist = 20.0f;
		if (dist < atDist)
		{
			m_isAtSpot = true;

			// make sure our approach points are valid, since we'll be watching them
			me->ComputeApproachPoints();

			// ready our weapon and prepare to attack
			me->EquipBestWeapon(MUST_EQUIP);
			me->SetDisposition(CCSBot::OPPORTUNITY_FIRE);

			// if we are a sniper, update our task
			if (me->GetTask() == CCSBot::MOVE_TO_SNIPER_SPOT)
			{
				me->SetTask(CCSBot::SNIPING);
			}

			// determine which way to look
			TraceResult result;
			float outAngle = 0.0f;
			float outAngleRange = 0.0f;
			for (float angle = 0.0f; angle < 360.0f; angle += 45.0f)
			{
				UTIL_TraceLine(me->GetEyePosition(), me->GetEyePosition() + 1000.0f * Vector(BotCOS(angle), BotSIN(angle), 0.0f), ignore_monsters, ignore_glass, ENT(me->pev), &result);

				if (result.flFraction > outAngleRange)
				{
					outAngle = angle;
					outAngleRange = result.flFraction;
				}
			}

			me->SetLookAheadAngle(outAngle);
		}

		// move to hiding spot
		if (me->UpdatePathMovement() != CCSBot::PROGRESSING && !m_isAtSpot)
		{
			// we couldn't get to our hiding spot - pick another
			me->PrintIfWatched("Can't get to my hiding spot - finding another...\n");

			// search from hiding spot, since we know it was valid
			const Vector *pos = FindNearbyHidingSpot(me, &m_hidingSpot, m_searchFromArea, m_range, me->IsSniper());
			if (pos == NULL)
			{
				// no available hiding spots
				me->PrintIfWatched("No available hiding spots - hiding where I'm at.\n");

				// hide where we are
				m_hidingSpot.x = me->pev->origin.x;
				m_hidingSpot.y = me->pev->origin.y;
				m_hidingSpot.z = me->GetFeetZ();
			}
			else
			{
				m_hidingSpot = *pos;
			}

			// build a path to our new hiding spot
			if (me->ComputePath(TheNavAreaGrid.GetNavArea(&m_hidingSpot), &m_hidingSpot, FASTEST_ROUTE) == false)
			{
				me->PrintIfWatched("Can't pathfind to hiding spot\n");
				me->Idle();
				return;
			}
		}
	}
}

void HideState::OnExit(CCSBot *me)
{
	m_isHoldingPosition = false;

	me->StandUp();
	me->ResetStuckMonitor();
	me->ClearLookAt();
	me->ClearApproachPoints();

	// if we have a shield, put it away
	if (me->HasShield() && me->IsProtectedByShield())
		me->SecondaryAttack();
}
