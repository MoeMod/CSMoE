
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

void HostageFollowState::OnEnter(CHostageImprov *improv)
{
	improv->Chatter(HOSTAGE_CHATTER_START_FOLLOW);
	improv->Agree();

	m_isWaiting = false;
	m_isWaitingForFriend = false;

	improv->MoveTo(improv->GetFeet());

	m_lastLeaderPos = Vector(999999, 999999, 999999);

	m_makeWayTimer.Invalidate();
	SetFollowRange(3000.0f, 1000.0f, RANDOM_FLOAT(125, 175));

	if (improv->IsTerroristNearby())
	{
		improv->DelayedChatter(3s, HOSTAGE_CHATTER_WARN_NEARBY);
	}
}

void HostageFollowState::OnUpdate(CHostageImprov *improv)
{
	// if we lost our leader, give up
	if (m_leader == nullptr)
	{
		improv->Idle();
		return;
	}

	if (m_leader->pev->deadflag != DEAD_NO)
	{
		improv->Frighten(CHostageImprov::TERRIFIED);
		improv->Idle();
		return;
	}

	float range = (m_leader->pev->origin - improv->GetCentroid()).Length();

	if( range > m_giveUpRange || ( improv->GetPath()->GetSegmentCount() > 0 && improv->GetPath()->GetLength() > m_maxPathLength) )
	{
		improv->Idle();
		return;
	}

	const float walkRange = m_stopRange + 50.0f;
	const float continueRange = m_stopRange + 20.0f;
	const float runThreshold = 140.0f;

	bool isLeaderRunning;
	float leaderSpeed = m_leader->pev->velocity.Make2D().Length();

	if (leaderSpeed > runThreshold)
	{
		isLeaderRunning = true;
		m_isWaiting = false;
	}
	else
	{
		// track when began to run
		isLeaderRunning = false;

		if (!m_isWaiting)
		{
			if (range < m_stopRange)
			{
				m_isWaiting = true;
				m_impatientTimer.Start(RandomDuration<float>(5.0s, 20.0s));
			}
		}
		else if (range > continueRange)
			m_isWaiting = false;
	}

	if (!m_isWaiting)
	{
		bool makeWay = improv->IsFriendInTheWay();

		if (makeWay && !m_isWaitingForFriend)
		{
			m_isWaitingForFriend = true;
			m_waitForFriendTimer.Start(15.0s);

			improv->Stop();
			return;
		}

		if (makeWay && !m_waitForFriendTimer.IsElapsed())
		{
			improv->Stop();
			return;
		}

		if (improv->GetPath()->GetSegmentCount() <= 0 && m_repathTimer.IsElapsed())
		{
			improv->MoveTo(m_leader->pev->origin);

			m_lastLeaderPos = m_leader->pev->origin;
			m_repathTimer.Start(1.0s);
		}

		if (isLeaderRunning || range > walkRange)
			improv->Run();
		else
			improv->Walk();

		//if (!FClassnameIs(improv->GetEntity()->pev, "monster_entity"))
		//{
			const float movedRange = 35.0f;
			if ((m_lastLeaderPos - m_leader->pev->origin).IsLengthGreaterThan(movedRange))
			{
				improv->MoveTo(m_leader->pev->origin);
				m_lastLeaderPos = m_leader->pev->origin;
			}
		//}

		return;
	}

	if (m_impatientTimer.IsElapsed() && !TheCSBots()->IsRoundOver())
	{
		m_impatientTimer.Start(RandomDuration<float>(20.0s, 30.0s));

		if (improv->CanSeeRescueZone())
			improv->Chatter(HOSTAGE_CHATTER_SEE_RESCUE_ZONE, false);
		else
			improv->Chatter(HOSTAGE_CHATTER_IMPATIENT_FOR_RESCUE, false);
	}

	// remove hostage's stuff
	if (!FClassnameIs(improv->GetEntity()->pev, "monster_entity"))
	{
		const float closeRange = 200.0f;
		if ((m_leader->pev->origin - improv->GetCentroid()).IsLengthLessThan(closeRange))
		{
			bool makeWay = false;
			const float cosTolerance = 0.99f;

			if (improv->IsPlayerLookingAtMe(static_cast<CBasePlayer *>(static_cast<CBaseEntity *>(m_leader)), cosTolerance))
			{
				if (!m_makeWayTimer.HasStarted())
				{
					m_makeWayTimer.Start(RandomDuration<float>(0.4s, 0.75s));
				}
				else if (m_makeWayTimer.IsElapsed())
				{
					m_impatientTimer.Invalidate();

					Vector to = (m_leader->pev->origin - improv->GetCentroid());
					to.NormalizeInPlace();

					Vector cross(-to.y, -to.x, 0);
					if (cross.x * gpGlobals->v_forward.x - cross.y * gpGlobals->v_forward.y < 0)
						cross.y = to.x;
					else
						cross.x = to.y;

					float ground;
					float const sideStepSize = 15.0f;
					Vector sideStepPos = improv->GetFeet() + cross * sideStepSize;

					if (GetGroundHeight(&sideStepPos, &ground))
					{
						if (abs((int)(ground - improv->GetFeet().z)) < 18.0f)
						{
							const float push = 20.0f;
							Vector lat = cross * push;

							improv->ApplyForce(lat);
							improv->MoveTo(sideStepPos);
							return;
						}
					}
				}
			}
			else
			{
				m_makeWayTimer.Invalidate();
			}
		}
	}

	improv->Stop();

	CBasePlayer *terrorist = improv->GetClosestVisiblePlayer(TERRORIST);

	if (terrorist != NULL)
	{
		improv->LookAt(terrorist->EyePosition());
	}
	else
	{
		improv->LookAt(m_leader->EyePosition());
	}
}

void HostageFollowState::OnExit(CHostageImprov *improv)
{
	improv->Stop();
}

void HostageFollowState::UpdateStationaryAnimation(CHostageImprov *improv)
{
	if (improv->IsScared())
		improv->UpdateIdleActivity(ACT_FOLLOW_IDLE_SCARED, ACT_RESET);
	else
		improv->UpdateIdleActivity(ACT_FOLLOW_IDLE, ACT_FOLLOW_IDLE_FIDGET);
}

}
