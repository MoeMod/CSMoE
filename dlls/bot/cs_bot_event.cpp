
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

#include "bot_include.h"

namespace sv {

void CCSBot::OnEvent(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	GetGameState()->OnEvent(event, entity, other);
	GetChatter()->OnEvent(event, entity, other);

	// Morale adjustments happen even for dead players
	switch (event)
	{
	case EVENT_TERRORISTS_WIN:
		if (m_iTeam == CT)
		{
			DecreaseMorale();
		}
		else
		{
			IncreaseMorale();
		}
		break;
	case EVENT_CTS_WIN:
		if (m_iTeam == CT)
		{
			IncreaseMorale();
		}
		else
		{
			DecreaseMorale();
		}
		break;
	default:
		break;
	}

	if (!IsAlive())
		return;

	CBasePlayer *player = static_cast<CBasePlayer *>(entity);

	// If we just saw a nearby friend die, and we haven't yet acquired an enemy
	// automatically acquire our dead friend's killer
	if (!IsAttacking() && (GetDisposition() == ENGAGE_AND_INVESTIGATE || GetDisposition() == OPPORTUNITY_FIRE))
	{
		if (event == EVENT_PLAYER_DIED)
		{
			if (player->m_iTeam == m_iTeam)
			{
				CBasePlayer *killer = static_cast<CBasePlayer *>(other);

				// check that attacker is an enemy (for friendly fire, etc)
				if (killer && killer->IsPlayer())
				{
					// check if we saw our friend die - dont check FOV - assume we're aware of our surroundings in combat
					// snipers stay put
					if (!IsSniper() && IsVisible(&player->pev->origin))
					{
						// people are dying - we should hurry
						Hurry(RandomDuration(10.0s, 15.0s));

						// if we're hiding with only our knife, be a little more cautious
						const float knifeAmbushChance = 50.0f;
						if (!IsHiding() || !IsUsingKnife() || RANDOM_FLOAT(0, 100) < knifeAmbushChance)
						{
							PrintIfWatched("Attacking our friend's killer!\n");
							Attack(killer);
							return;
						}
					}
				}
			}
		}
	}

	switch (event)
	{
		case EVENT_PLAYER_DIED:
		{
			CBasePlayer *victim = player;
			CBasePlayer *killer = (other && other->IsPlayer()) ? static_cast<CBasePlayer *>(other) : NULL;

			// if the human player died in the single player game, tell the team
			if (g_pGameRules->IsCareer() && !victim->IsBot() && victim->m_iTeam == m_iTeam)
			{
				GetChatter()->Say("CommanderDown", 20.0s);
			}

			// keep track of the last player we killed
			if (killer == this)
			{
				m_lastVictimID = victim->entindex();
			}

			// react to teammate death
			if (victim->m_iTeam == m_iTeam)
			{
				// chastise friendly fire from humans
				if (killer != NULL && !killer->IsBot() && killer->m_iTeam == m_iTeam && killer != this)
				{
					GetChatter()->KilledFriend();
				}

				if (IsHunting())
				{
					PrintIfWatched("Rethinking hunt due to teammate death\n");
					Idle();
					return;
				}

				if (IsAttacking())
				{
					if (GetTimeSinceLastSawEnemy() > 0.4s)
					{
						PrintIfWatched("Rethinking my attack due to teammate death\n");

						// allow us to sneak past windows, doors, etc
						IgnoreEnemies(1.0s);

						// move to last known position of enemy - this could cause us to flank if 
						// the danger has changed due to our teammate's recent death
						SetTask(MOVE_TO_LAST_KNOWN_ENEMY_POSITION, GetEnemy());
						MoveTo(&GetLastKnownEnemyPosition());
						return;
					}
				}
			}
			// an enemy was killed
			else
			{
				if (killer != NULL && killer->m_iTeam == m_iTeam)
				{
					// only chatter about enemy kills if we see them occur, and they were the last one we see
					if (GetNearbyEnemyCount() <= 1)
					{
						// report if number of enemies left is few and we killed the last one we saw locally
						GetChatter()->EnemiesRemaining();

						if (IsVisible(&victim->pev->origin, CHECK_FOV))
						{						
							// congratulate teammates on their kills
							if (killer != NULL && killer != this)
							{
								auto delay = RandomDuration(2.0s, 3.0s);
								if (killer->IsBot())
								{
									if (RANDOM_FLOAT(0.0f, 100.0f) < 40.0f)
										GetChatter()->Say("NiceShot", 3.0s, delay);
								}
								else
								{
									// humans get the honorific
									if (g_pGameRules->IsCareer())
										GetChatter()->Say("NiceShotCommander", 3.0s, delay);
									else
										GetChatter()->Say("NiceShotSir", 3.0s, delay);
								}
							}
						}
					}
				}
			}
			return;
		}
		case EVENT_TERRORISTS_WIN:
			if (m_iTeam == TERRORIST)
				GetChatter()->CelebrateWin();
			return;
		case EVENT_CTS_WIN:
			if (m_iTeam == CT)
				GetChatter()->CelebrateWin();
			return;
		case EVENT_BOMB_DEFUSED:
			if (m_iTeam == CT && TheCSBots()->GetBombTimeLeft() < 2.0s)
				GetChatter()->Say("BarelyDefused");
			return;
		case EVENT_BOMB_PICKED_UP:
		{
			if (m_iTeam == CT && player != NULL)
			{
				// check if we're close enough to hear it
				const float bombPickupHearRangeSq = 1000.0f * 1000.0f;
				if ((pev->origin - player->pev->origin).LengthSquared() < bombPickupHearRangeSq)
				{
					GetChatter()->TheyPickedUpTheBomb();
				}
			}
			return;
		}
		case EVENT_BOMB_BEEP:
		{
			// if we don't know where the bomb is, but heard it beep, we've discovered it
			if (GetGameState()->IsPlantedBombLocationKnown() == false)
			{
				// check if we're close enough to hear it
				const float bombBeepHearRangeSq = 1000.0f * 1000.0f;
				if ((pev->origin - entity->pev->origin).LengthSquared() < bombBeepHearRangeSq)
				{
					// radio the news to our team
					if (m_iTeam == CT && GetGameState()->GetPlantedBombsite() == CSGameState::UNKNOWN)
					{
						const CCSBotManager::Zone *zone = TheCSBots()->GetZone(&entity->pev->origin);
						if (zone != NULL)
							GetChatter()->FoundPlantedBomb(zone->m_index);
					}

					// remember where the bomb is
					GetGameState()->UpdatePlantedBomb(&entity->pev->origin);
				}
			}
			return;
		}
		case EVENT_BOMB_PLANTED:
		{
			// if we're a CT, forget what we're doing and go after the bomb
			if (m_iTeam == CT)
			{
				Idle();
			}

			// if we are following someone, stop following
			if (IsFollowing())
			{
				StopFollowing();
				Idle();
			}

			OnEvent(EVENT_BOMB_BEEP, other);
			return;
		}
		case EVENT_BOMB_DEFUSE_ABORTED:
			PrintIfWatched("BOMB DEFUSE ABORTED\n");
			return;
		case EVENT_WEAPON_FIRED:
		case EVENT_WEAPON_FIRED_ON_EMPTY:
		case EVENT_WEAPON_RELOADED:
		{
			if (m_enemy == entity && IsUsingKnife())
			{
				ForceRun(5.0s);
				if (m_bIsZombie && this->m_ZombieClass)
				{
					if (!stricmp(this->m_ZombieClass, "china") || !stricmp(this->m_ZombieClass, "deimos") || !stricmp(this->m_ZombieClass, "heal") || !stricmp(this->m_ZombieClass, "deimos2") || !stricmp(this->m_ZombieClass, "pc") || !stricmp(this->m_ZombieClass, "heavy") || !stricmp(this->m_ZombieClass, "stamper") || !stricmp(this->m_ZombieClass, "zbs_deimos2") || !stricmp(this->m_ZombieClass, "nemesis") || !stricmp(this->m_ZombieClass, "tank") || !stricmp(this->m_ZombieClass, "teleport") || !stricmp(this->m_ZombieClass, "revival"))
					{
						if(pev->health <= 3000)
							ClientCommand("BTE_ZombieSkill2");
					}

					if(!stricmp(this->m_ZombieClass, "flying") || !stricmp(this->m_ZombieClass, "china") || !stricmp(this->m_ZombieClass, "teleport") || !stricmp(this->m_ZombieClass, "deimos") || !stricmp(this->m_ZombieClass, "heal") || !stricmp(this->m_ZombieClass, "deimos2") || !stricmp(this->m_ZombieClass, "pc") || !stricmp(this->m_ZombieClass, "heavy") || !stricmp(this->m_ZombieClass, "deathknight") || !stricmp(this->m_ZombieClass, "nemesis") || !stricmp(this->m_ZombieClass, "tank"))
						ClientCommand("BTE_ZombieSkill1");

					float skillUseRange = 1400.0f;
					if (!stricmp(this->m_ZombieClass, "booster"))
					{
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "resident"))
					{
						skillUseRange = 140;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill2");
					}
					else if (!stricmp(this->m_ZombieClass, "boomer"))
					{
						skillUseRange = 500;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange) && this->pev->health <= 2000)
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "stamper"))
					{
						skillUseRange = 140;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "witch") || !stricmp(this->m_ZombieClass, "aksha"))
					{
						skillUseRange = 1500;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "zbs_deimos2"))
					{
						skillUseRange = 3000;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "spider"))
					{
						skillUseRange = 2000;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill2");
						else
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "deathknight"))
					{
						skillUseRange = 3000;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill2");
					}
					else if (!stricmp(this->m_ZombieClass, "siren"))
					{
						skillUseRange = 400;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill1");
					}
					else if (!stricmp(this->m_ZombieClass, "meatwall"))
					{
						skillUseRange = 1000;
						if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
							ClientCommand("BTE_ZombieSkill2");
					}
						
				}
                else if (m_bIsZombie && event == EVENT_WEAPON_RELOADED)
				{
					const float skillUseRange = 8192.0f;
					if ((pev->origin - entity->pev->origin).IsLengthLessThan(skillUseRange))
					{
						ClientCommand("BTE_ZombieSkill1");
					}
				}
			}
			break;
		}
		case EVENT_SUPPLYBOX_ARRIVE:
		{
			// fetch the supplybox
			if (!m_bIsZombie && RANDOM_FLOAT(0.0f, 100.0f) < 10.0f && !IsDefending() )
			{
				RushToSupplyBox();
			}
			break;
		}
		case EVENT_PLAYER_TOOK_DAMAGE:
		{
			if (!other)
				break;
			if (!player->m_bIsZombie)
				break;
			if (!m_bIsZombie)
				break;
			if (IsAttacking())
				break;
			if (GetDisposition() != ENGAGE_AND_INVESTIGATE && GetDisposition() != OPPORTUNITY_FIRE)
				break;
			if (GetTask() == MOVE_TO_ATTACKER_POSITION)
				break;
			if (GetTimeSinceLastSawEnemy() < 2.0s)
				break;

			SetTask(MOVE_TO_ATTACKER_POSITION, other);
			MoveTo(&other->pev->origin);
			break;
		}
		default:
			break;
	}

	// Process radio events from our team
	if (player != NULL && player->m_iTeam == m_iTeam && event > EVENT_START_RADIO_1 && event < EVENT_END_RADIO)
	{
		// TODO: Distinguish between radio commands and responses
		if (event != EVENT_RADIO_AFFIRMATIVE && event != EVENT_RADIO_NEGATIVE && event != EVENT_RADIO_REPORTING_IN)
		{
			m_lastRadioCommand = event;
			m_lastRadioRecievedTimestamp = gpGlobals->time;
			m_radioSubject = player;
			m_radioPosition = player->pev->origin;
		}
	}

	// player_follows needs a player
	if (player == NULL)
		return;

	if (!IsRogue() && event == EVENT_HOSTAGE_CALLED_FOR_HELP && m_iTeam == CT && IsHunting())
	{
		if ((entity->pev->origin - pev->origin).IsLengthGreaterThan(1000.0f))
			return;

		Vector v = entity->Center();

		if (IsVisible(&v))
		{
			m_task = COLLECT_HOSTAGES;
			m_taskEntity = NULL;

			Run();
			m_goalEntity = entity;

			MoveTo(&entity->pev->origin, (RouteType)(m_hostageEscortCount == 0));
			PrintIfWatched("I'm fetching a hostage that called out to me\n");

			return;
		}
	}

	// don't pay attention to noise that friends make
	if (!IsEnemy(player))
		return;

	float range;
	PriorityType priority;
	bool isHostile;

	if (IsGameEventAudible(event, entity, other, &range, &priority, &isHostile) == false)
		return;

	if (event == EVENT_HOSTAGE_USED)
	{
		if (m_iTeam == CT)
			return;

		if ((entity->pev->origin - pev->origin).IsLengthGreaterThan(range))
			return;

		GetChatter()->HostagesBeingTaken();

		if (!GetGameState()->GetNearestVisibleFreeHostage() && m_task != GUARD_HOSTAGE_RESCUE_ZONE && GuardRandomZone())
		{
			m_task = GUARD_HOSTAGE_RESCUE_ZONE;
			m_taskEntity = NULL;

			SetDisposition(OPPORTUNITY_FIRE);
			PrintIfWatched("Trying to beat them to an escape zone!\n");
		}
	}

	// check if noise is close enough for us to hear
	const Vector *newNoisePosition = &player->pev->origin;
	float newNoiseDist = (pev->origin - *newNoisePosition).Length();
	if (newNoiseDist < range)
	{
		// we heard the sound
		if ((IsLocalPlayerWatchingMe() && cv_bot_debug.value == 3.0f) || cv_bot_debug.value == 4.0f)
		{
			PrintIfWatched("Heard noise (%s from %s, pri %s, time %3.1f)\n",
				(event == EVENT_WEAPON_FIRED) ? "Weapon fire " : "",
				STRING(player->pev->netname),
				(priority == PRIORITY_HIGH) ? "HIGH" : ((priority == PRIORITY_MEDIUM) ? "MEDIUM" : "LOW"),
				gpGlobals->time);
		}

		if (event == EVENT_PLAYER_FOOTSTEP && IsUsingSniperRifle() && newNoiseDist < 300.0)
			EquipPistol();

		// should we pay attention to it
		// if noise timestamp is zero, there is no prior noise
		if (m_noiseTimestamp > invalid_time_point)
		{
			// only overwrite recent sound if we are louder (closer), or more important - if old noise was long ago, its faded
			constexpr auto shortTermMemoryTime = 3.0s;
			if (gpGlobals->time - m_noiseTimestamp < shortTermMemoryTime)
			{
				// prior noise is more important - ignore new one
				if (priority < m_noisePriority)
					return;

				float oldNoiseDist = (pev->origin - m_noisePosition).Length();
				if (newNoiseDist >= oldNoiseDist)
					return;
			}
		}


		// find the area in which the noise occured
		// TODO: Better handle when noise occurs off the nav mesh
		// TODO: Make sure noise area is not through a wall or ceiling from source of noise
		// TODO: Change GetNavTravelTime to better deal with NULL destination areas
		CNavArea *noiseArea = TheNavAreaGrid.GetNavArea(newNoisePosition);
		if (noiseArea == NULL)
			noiseArea = TheNavAreaGrid.GetNearestNavArea(newNoisePosition);

		if (noiseArea == NULL)
		{
			PrintIfWatched("  *** Noise occurred off the nav mesh - ignoring!\n");
			return;
		}

		m_noiseArea = noiseArea;

		// remember noise priority
		m_noisePriority = priority;

		// randomize noise position in the area a bit - hearing isn't very accurate
		// the closer the noise is, the more accurate our placement
		// TODO: Make sure not to pick a position on the opposite side of ourselves.
		const float maxErrorRadius = 400.0f;
		const float maxHearingRange = 2000.0f;
		float errorRadius = maxErrorRadius * newNoiseDist / maxHearingRange;

		m_noisePosition.x = newNoisePosition->x + RANDOM_FLOAT(-errorRadius, errorRadius);
		m_noisePosition.y = newNoisePosition->y + RANDOM_FLOAT(-errorRadius, errorRadius);

		// make sure noise position remains in the same area
		m_noiseArea->GetClosestPointOnArea(&m_noisePosition, &m_noisePosition);

		m_isNoiseTravelRangeChecked = false;
		// note when we heard the noise
		m_noiseTimestamp = gpGlobals->time;
	}
}

}
