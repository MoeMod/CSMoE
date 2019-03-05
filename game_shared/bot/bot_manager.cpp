
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "client.h"

#include "pm_shared.h"

#include "utllinkedlist.h"

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

#include "tutor_base_states.h"
#include "tutor_base_tutor.h"

/*
* Globals initialization
*/
const char *GameEventName[NUM_GAME_EVENTS + 1] =
{
	"EVENT_INVALID",
	"EVENT_WEAPON_FIRED",
	"EVENT_WEAPON_FIRED_ON_EMPTY",
	"EVENT_WEAPON_RELOADED",
	"EVENT_HE_GRENADE_EXPLODED",
	"EVENT_FLASHBANG_GRENADE_EXPLODED",
	"EVENT_SMOKE_GRENADE_EXPLODED",
	"EVENT_GRENADE_BOUNCED",
	"EVENT_BEING_SHOT_AT",
	"EVENT_PLAYER_BLINDED_BY_FLASHBANG",
	"EVENT_PLAYER_FOOTSTEP",
	"EVENT_PLAYER_JUMPED",
	"EVENT_PLAYER_DIED",
	"EVENT_PLAYER_LANDED_FROM_HEIGHT",
	"EVENT_PLAYER_TOOK_DAMAGE",
	"EVENT_HOSTAGE_DAMAGED",
	"EVENT_HOSTAGE_KILLED",
	"EVENT_DOOR",
	"EVENT_BREAK_GLASS",
	"EVENT_BREAK_WOOD",
	"EVENT_BREAK_METAL",
	"EVENT_BREAK_FLESH",
	"EVENT_BREAK_CONCRETE",
	"EVENT_BOMB_PLANTED",
	"EVENT_BOMB_DROPPED",
	"EVENT_BOMB_PICKED_UP",
	"EVENT_BOMB_BEEP",
	"EVENT_BOMB_DEFUSING",
	"EVENT_BOMB_DEFUSE_ABORTED",
	"EVENT_BOMB_DEFUSED",
	"EVENT_BOMB_EXPLODED",
	"EVENT_HOSTAGE_USED",
	"EVENT_HOSTAGE_RESCUED",
	"EVENT_ALL_HOSTAGES_RESCUED",
	"EVENT_VIP_ESCAPED",
	"EVENT_VIP_ASSASSINATED",
	"EVENT_TERRORISTS_WIN",
	"EVENT_CTS_WIN",
	"EVENT_ROUND_DRAW",
	"EVENT_ROUND_WIN",
	"EVENT_ROUND_LOSS",
	"EVENT_ROUND_START",
	"EVENT_PLAYER_SPAWNED",
	"EVENT_CLIENT_CORPSE_SPAWNED",
	"EVENT_BUY_TIME_START",
	"EVENT_PLAYER_LEFT_BUY_ZONE",
	"EVENT_DEATH_CAMERA_START",
	"EVENT_KILL_ALL",
	"EVENT_ROUND_TIME",
	"EVENT_DIE",
	"EVENT_KILL",
	"EVENT_HEADSHOT",
	"EVENT_KILL_FLASHBANGED",
	"EVENT_TUTOR_BUY_MENU_OPENNED",
	"EVENT_TUTOR_AUTOBUY",
	"EVENT_PLAYER_BOUGHT_SOMETHING",
	"EVENT_TUTOR_NOT_BUYING_ANYTHING",
	"EVENT_TUTOR_NEED_TO_BUY_PRIMARY_WEAPON",
	"EVENT_TUTOR_NEED_TO_BUY_PRIMARY_AMMO",
	"EVENT_TUTOR_NEED_TO_BUY_SECONDARY_AMMO",
	"EVENT_TUTOR_NEED_TO_BUY_ARMOR",
	"EVENT_TUTOR_NEED_TO_BUY_DEFUSE_KIT",
	"EVENT_TUTOR_NEED_TO_BUY_GRENADE",
	"EVENT_CAREER_TASK_DONE",
	"EVENT_START_RADIO_1",
	"EVENT_RADIO_COVER_ME",
	"EVENT_RADIO_YOU_TAKE_THE_POINT",
	"EVENT_RADIO_HOLD_THIS_POSITION",
	"EVENT_RADIO_REGROUP_TEAM",
	"EVENT_RADIO_FOLLOW_ME",
	"EVENT_RADIO_TAKING_FIRE",
	"EVENT_START_RADIO_2",
	"EVENT_RADIO_GO_GO_GO",
	"EVENT_RADIO_TEAM_FALL_BACK",
	"EVENT_RADIO_STICK_TOGETHER_TEAM",
	"EVENT_RADIO_GET_IN_POSITION_AND_WAIT",
	"EVENT_RADIO_STORM_THE_FRONT",
	"EVENT_RADIO_REPORT_IN_TEAM",
	"EVENT_START_RADIO_3",
	"EVENT_RADIO_AFFIRMATIVE",
	"EVENT_RADIO_ENEMY_SPOTTED",
	"EVENT_RADIO_NEED_BACKUP",
	"EVENT_RADIO_SECTOR_CLEAR",
	"EVENT_RADIO_IN_POSITION",
	"EVENT_RADIO_REPORTING_IN",
	"EVENT_RADIO_GET_OUT_OF_THERE",
	"EVENT_RADIO_NEGATIVE",
	"EVENT_RADIO_ENEMY_DOWN",
	"EVENT_END_RADIO",
	"EVENT_NEW_MATCH",
	"EVENT_PLAYER_CHANGED_TEAM",
	"EVENT_BULLET_IMPACT",
	"EVENT_GAME_COMMENCE",
	"EVENT_WEAPON_ZOOMED",
	"EVENT_HOSTAGE_CALLED_FOR_HELP",
	NULL,
};

// STL uses exceptions, but we are not compiling with them - ignore warning
#ifdef _MSC_VER
#pragma warning(disable : 4530)
#endif

const float smokeRadius = 115.0f;		// for smoke grenades

// Convert name to GameEventType
// TODO: Find more appropriate place for this function

GameEventType NameToGameEvent(const char *name)
{
	for (int i = 0; GameEventName[i] != NULL; ++i)
	{
		if (!Q_stricmp(GameEventName[i], name))
			return static_cast<GameEventType>(i);
	}

	return EVENT_INVALID;
}

CBotManager::CBotManager()
{
	InitBotTrig();
}

// Invoked when the round is restarting

void CBotManager::RestartRound()
{
	DestroyAllGrenades();
}

// Invoked at the start of each frame

void CBotManager::StartFrame()
{
	// debug smoke grenade visualization
	if (cv_bot_debug.value == 5)
	{
		Vector edge, lastEdge;

		int it = m_activeGrenadeList.Head ();

		while (it != m_activeGrenadeList.InvalidIndex ())
		{
			ActiveGrenade *ag = m_activeGrenadeList[it];

			int current = it;
			it = m_activeGrenadeList.Next (it);

			// lazy validation
			if (!ag->IsValid ())
			{
				m_activeGrenadeList.Remove (current);
				delete ag;

				continue;
			}
			const Vector *pos = ag->GetDetonationPosition();

			UTIL_DrawBeamPoints(*pos, *pos + Vector(0, 0, 50), 1, 255, 100, 0);

			lastEdge = Vector(smokeRadius + pos->x, pos->y, pos->z);
			float angle;
			for (angle = 0.0f; angle <= 180.0f; angle += 22.5f)
			{
				edge.x = smokeRadius * BotCOS(angle) + pos->x;
				edge.y = pos->y;
				edge.z = smokeRadius * BotSIN(angle) + pos->z;

				UTIL_DrawBeamPoints(edge, lastEdge, 1, 255, 50, 0);

				lastEdge = edge;
			}

			lastEdge = Vector(pos->x, smokeRadius + pos->y, pos->z);
			for (angle = 0.0f; angle <= 180.0f; angle += 22.5f)
			{
				edge.x = pos->x;
				edge.y = smokeRadius * BotCOS(angle) + pos->y;
				edge.z = smokeRadius * BotSIN(angle) + pos->z;

				UTIL_DrawBeamPoints(edge, lastEdge, 1, 255, 50, 0);

				lastEdge = edge;
			}
		}
	}

	// Process each active bot
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (!pPlayer)
			continue;

		if (pPlayer->IsBot() && IsEntityValid(pPlayer))
		{
			CBot *pBot = static_cast<CBot *>(pPlayer);
			pBot->BotThink();
		}
	}
}

// Return the filename for this map's "nav map" file

const char *CBotManager::GetNavMapFilename() const
{
	static char filename[256];
	Q_sprintf(filename, "maps\\%s.nav", STRING(gpGlobals->mapname));
	return filename;
}

// Invoked when given player does given event (some events have NULL player).
// Events are propogated to all bots.
// TODO: This has become the game-wide event dispatcher. We should restructure this.

void CBotManager::OnEvent(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	// propogate event to all bots
	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *player = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(i));

		if (player == NULL)
			continue;

		if (FNullEnt(player->pev))
			continue;

		if (FStrEq(STRING(player->pev->netname), ""))
			continue;

		if (!player->IsBot())
			continue;

		// do not send self-generated event
		if (entity == player)
			continue;

		CBot *bot = static_cast<CBot *>(player);
		bot->OnEvent(event, entity, other);
	}

	if (TheTutor != NULL)
	{
		TheTutor->OnEvent(event, entity, other);
	}

	if (g_pHostages != NULL)
	{
		g_pHostages->OnEvent(event, entity, other);
	}
}

// Add an active grenade to the bot's awareness

void CBotManager::AddGrenade(int type, CGrenade *grenade)
{
	m_activeGrenadeList.AddToTail(new ActiveGrenade (type, grenade));
}

// The grenade entity in the world is going away

void CBotManager::RemoveGrenade(CGrenade *grenade)
{
	FOR_EACH_LL (m_activeGrenadeList, it)
	{
		ActiveGrenade *ag = m_activeGrenadeList[it];

		if (ag->IsEntity (grenade))
		{
			ag->OnEntityGone ();
			return;
		}
	}
}

// Destroy any invalid active grenades

NOXREF void CBotManager::ValidateActiveGrenades()
{
	int it = m_activeGrenadeList.Head ();

	while (it != m_activeGrenadeList.InvalidIndex ())
	{
		ActiveGrenade *ag = m_activeGrenadeList[it];

		int current = it;
		it = m_activeGrenadeList.Next (it);

		// lazy validation
		if (!ag->IsValid ())
		{
			m_activeGrenadeList.Remove (current);
			delete ag;
			continue;
		}
	}
}

void CBotManager::DestroyAllGrenades()
{
	m_activeGrenadeList.PurgeAndDeleteElements ();
}

// Return true if position is inside a smoke cloud

bool CBotManager::IsInsideSmokeCloud(const Vector *pos)
{
	int it = m_activeGrenadeList.Head ();

	while (it != m_activeGrenadeList.InvalidIndex ())
	{
		ActiveGrenade *ag = m_activeGrenadeList[it];

		int current = it;
		it = m_activeGrenadeList.Next (it);

		// lazy validation
		if (!ag->IsValid())
		{
			m_activeGrenadeList.Remove (current);
			delete ag;
			continue;
		}

		if (ag->GetID() == WEAPON_SMOKEGRENADE)
		{
			const Vector *smokeOrigin = ag->GetDetonationPosition();

			if ((*smokeOrigin - *pos).IsLengthLessThan(smokeRadius))
				return true;
		}
	}

	return false;
}

// Return true if line intersects smoke volume
// Determine the length of the line of sight covered by each smoke cloud,
// and sum them (overlap is additive for obstruction).
// If the overlap exceeds the threshold, the bot can't see through.

bool CBotManager::IsLineBlockedBySmoke(const Vector *from, const Vector *to)
{
	const float smokeRadiusSq = smokeRadius * smokeRadius;

	// distance along line of sight covered by smoke
	float totalSmokedLength = 0.0f;

	// compute unit vector and length of line of sight segment
	Vector sightDir = *to - *from;
	float sightLength = sightDir.NormalizeInPlace();

	int it = m_activeGrenadeList.Head ();

	while (it != m_activeGrenadeList.InvalidIndex ())
	{
		ActiveGrenade *ag = m_activeGrenadeList[it];

		int current = it;
		it = m_activeGrenadeList.Next (it);

		// lazy validation
		if (!ag->IsValid ())
		{
			m_activeGrenadeList.Remove (current);
			delete ag;
			continue;
		}

		if (ag->GetID() == WEAPON_SMOKEGRENADE)
		{
			const Vector *smokeOrigin = ag->GetDetonationPosition();

			Vector toGrenade = *smokeOrigin - *from;

			float alongDist = DotProduct(toGrenade, sightDir);

			// compute closest point to grenade along line of sight ray
			Vector close;

			// constrain closest point to line segment
			if (alongDist < 0.0f)
				close = *from;

			else if (alongDist >= sightLength)
				close = *to;
			else
				close = *from + sightDir * alongDist;

			// if closest point is within smoke radius, the line overlaps the smoke cloud
			Vector toClose = close - *smokeOrigin;
			float lengthSq = toClose.LengthSquared();

			if (lengthSq < smokeRadiusSq)
			{
				// some portion of the ray intersects the cloud
				float fromSq = toGrenade.LengthSquared();
				float toSq = (*smokeOrigin - *to).LengthSquared();

				if (fromSq < smokeRadiusSq)
				{
					if (toSq < smokeRadiusSq)
					{
						// both 'from' and 'to' lie within the cloud
						// entire length is smoked
						totalSmokedLength += (*to - *from).Length();
					}
					else
					{
						// 'from' is inside the cloud, 'to' is outside
						// compute half of total smoked length as if ray crosses entire cloud chord
						float halfSmokedLength = sqrt(smokeRadiusSq - lengthSq);

						if (alongDist > 0.0f)
						{
							// ray goes thru 'close'
							totalSmokedLength += halfSmokedLength + (close - *from).Length();
						}
						else
						{
							// ray starts after 'close'
							totalSmokedLength += halfSmokedLength - (close - *from).Length();
						}
					}
				}
				else if (toSq < smokeRadiusSq)
				{
					// 'from' is outside the cloud, 'to' is inside
					// compute half of total smoked length as if ray crosses entire cloud chord
					float halfSmokedLength = sqrt(smokeRadiusSq - lengthSq);

					Vector v = *to - *smokeOrigin;
					if (DotProduct(v, sightDir) > 0.0f)
					{
						// ray goes thru 'close'
						totalSmokedLength += halfSmokedLength + (close - *to).Length();
					}
					else
					{
						// ray ends before 'close'
						totalSmokedLength += halfSmokedLength - (close - *to).Length();
					}
				}
				else
				{
					// 'from' and 'to' lie outside of the cloud - the line of sight completely crosses it
					// determine the length of the chord that crosses the cloud

					float smokedLength = 2.0f * sqrt(smokeRadiusSq - lengthSq);
					totalSmokedLength += smokedLength;
				}
			}
		}
	}

	// define how much smoke a bot can see thru
	const float maxSmokedLength = 0.7f * smokeRadius;

	// return true if the total length of smoke-covered line-of-sight is too much
	return (totalSmokedLength > maxSmokedLength);
}
