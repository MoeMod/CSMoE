
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

void HostageIdleState::OnEnter(CHostageImprov *improv)
{
	m_moveState = MoveDone;
	m_fleeTimer.Invalidate();
	m_mustFlee = false;
}

void HostageIdleState::OnUpdate(CHostageImprov *improv)
{
	if (!UTIL_ActivePlayersInGame())
		return;

	if (m_mustFlee || (improv->IsScared() && !improv->IsTerroristNearby() && m_moveState != Moving))
	{
		if (!m_mustFlee && improv->GetScareIntensity() == CHostageImprov::TERRIFIED)
			m_mustFlee = true;

		if ((improv->GetScareIntensity() == CHostageImprov::TERRIFIED || m_mustFlee) || (m_fleeTimer.IsElapsed() && improv->GetScareIntensity() > CHostageImprov::NERVOUS))
		{
			m_fleeTimer.Start(RandomDuration<float>(10s, 20s));

			const float fleeChance = 33.3f;
			constexpr auto terroristRecentTime = 5.0s;

			if (!m_mustFlee && improv->GetTimeSinceLastSawPlayer(TERRORIST) > terroristRecentTime && RANDOM_FLOAT(0, 100) < fleeChance)
				m_mustFlee = true;

			if (m_mustFlee)
			{
				m_mustFlee = false;

				const Vector *spot = FindNearbyRetreatSpot(NULL, &improv->GetFeet(), improv->GetLastKnownArea(), 500.0, TERRORIST, false);

				if (spot != NULL)
				{
					improv->MoveTo(*spot);
					improv->Run();

					m_moveState = Moving;

					if (improv->GetScareIntensity() == CHostageImprov::TERRIFIED)
					{
						improv->Frighten(CHostageImprov::SCARED);
					}

					return;
				}
			}
		}
	}

	if (m_moveState && improv->IsAtMoveGoal())
	{
		m_moveState = NotMoving;

		improv->Stop();
		improv->FaceOutwards();
		
		const float crouchChance = 33.3f;
		if (improv->IsScared() && !improv->IsAtHome() && RANDOM_FLOAT(0, 100) <= crouchChance)
		{
			improv->Crouch();
		}

		return;
	}

	if (m_moveState == Moving)
	{
		improv->Run();
		return;
	}

	if (!improv->IsAtMoveGoal(75.0f))
	{
		improv->Walk();
		m_moveState = Moving;
		return;
	}

	// remove hostage's stuff
	if (FClassnameIs(improv->GetEntity()->pev, "monster_entity"))
	{
		return;
	}

	CBasePlayer *rescuer = improv->GetClosestVisiblePlayer(CT);
	CBasePlayer *captor = improv->GetClosestVisiblePlayer(TERRORIST);

	if (rescuer != NULL)
	{
		improv->LookAt(rescuer->EyePosition());
		improv->Stop();

		if (captor != NULL)
		{
			const float attentionRange = 700.0f;
			float rangeT = (improv->GetCentroid() - captor->pev->origin).Length();
			
			if (rangeT < attentionRange)
			{
				const float cosTolerance = 0.95f;
				if (improv->IsAnyPlayerLookingAtMe(TERRORIST, cosTolerance))
				{
					improv->LookAt(captor->EyePosition());
				}
				else
				{
					TraceResult result;
					UTIL_TraceLine(rescuer->pev->origin, captor->pev->origin, ignore_monsters, ignore_glass, captor->edict(), &result);

					if (result.flFraction != 1.0f && m_disagreeTimer.IsElapsed())
					{
						improv->Disagree();
						m_disagreeTimer.Start(RandomDuration<float>(2s, 4s));
					}
				}

				return;
			}
		}
		else if (!TheCSBots()->IsRoundOver() && m_askTimer.IsElapsed())
		{
			const float closeRange = 200.0f;
			if ((rescuer->pev->origin - improv->GetCentroid()).IsLengthLessThan(closeRange))
			{
				if (improv->IsPlayerLookingAtMe(rescuer, 0.99))
				{
					HostageChatterType say;
					if (improv->IsTerroristNearby())
						say = HOSTAGE_CHATTER_WARN_NEARBY;
					else
						say = HOSTAGE_CHATTER_PLEASE_RESCUE_ME;

					improv->Chatter(say, false);
					m_askTimer.Start(RandomDuration<float>(3s, 10s));
				}
			}
		}

		if (m_waveTimer.IsElapsed())
		{
			CHostage *hostage = improv->GetEntity();

			const float waveRange = 250.0f;
			if ((rescuer->pev->origin - hostage->pev->origin).IsLengthGreaterThan(waveRange))
			{
				improv->Stop();
				improv->Wave();

				improv->LookAt(rescuer->EyePosition());
				improv->Chatter(HOSTAGE_CHATTER_CALL_TO_RESCUER, false);

				m_moveState = NotMoving;
				m_waveTimer.Start(RandomDuration<float>(10s, 20s));
			}
		}
	}
	else if (captor != NULL)
	{
		improv->LookAt(captor->EyePosition());
		improv->Stop();

		const float closeRange = 200.0f;
		if ((captor->pev->origin - improv->GetCentroid()).IsLengthLessThan(closeRange) && improv->IsPlayerLookingAtMe(captor, 0.99))
		{
			if (!m_intimidatedTimer.HasStarted())
			{
				m_intimidatedTimer.Start();
			}

			if (!improv->IsScared())
			{
				improv->Frighten(CHostageImprov::NERVOUS);
			}

			constexpr auto minThreatenTime = 1.0s;
			if ((!m_intimidatedTimer.HasStarted() || m_intimidatedTimer.IsGreaterThen(minThreatenTime)) && m_pleadTimer.IsElapsed())
			{
				improv->Chatter(HOSTAGE_CHATTER_INTIMIDATED, true);
				m_pleadTimer.Start(RandomDuration<float>(10s, 20s));
			}

			if (!improv->IsAtHome())
			{
				improv->Chatter(HOSTAGE_CHATTER_RETREAT, true);
				improv->Retreat();
			}
		}
		else
		{
			m_intimidatedTimer.Invalidate();
		}
	}
	else
	{
		improv->ClearLookAt();

		const float pushbackRange = 60.0f;
		if ((pushbackRange - improv->GetAggression() * 5.0) * 1s < TheCSBots()->GetElapsedRoundTime() && m_escapeTimer.IsElapsed())
		{
			constexpr auto stayHomeDuration = 5.0s;
			m_escapeTimer.Start(stayHomeDuration);

			auto sightTimeT = improv->GetTimeSinceLastSawPlayer(TERRORIST);
			auto sightTimeCT = improv->GetTimeSinceLastSawPlayer(CT);

			const auto waitTime = 15.0s - improv->GetAggression() * 3.0s;

			if (sightTimeT > waitTime && sightTimeCT > waitTime)
			{
				if (improv->IsTerroristNearby())
				{
					if (cv_hostage_debug.value > 0.0f)
					{
						CONSOLE_ECHO("Hostage: I want to escape, but a T is nearby\n");
					}

					m_escapeTimer.Start(waitTime);
				}
				else
				{
					improv->Escape();

					if (cv_hostage_debug.value > 0.0f)
					{
						CONSOLE_ECHO("Hostage: I'm escaping!\n");
					}
				}
			}
		}
	}

}

void HostageIdleState::OnExit(CHostageImprov *improv)
{
	improv->StandUp();
	improv->ClearFaceTo();
}

void HostageIdleState::UpdateStationaryAnimation(CHostageImprov *improv)
{
	if (improv->IsScared())
	{
		if (improv->GetScareIntensity() == CHostageImprov::TERRIFIED)
			improv->Afraid();
		else
			improv->UpdateIdleActivity(ACT_IDLE_SCARED, ACT_RESET);
	}
	else if (improv->IsAtHome())
	{
		improv->UpdateIdleActivity(ACT_IDLE, ACT_IDLE_FIDGET);
	}
	else
		improv->UpdateIdleActivity(ACT_IDLE_SNEAKY, ACT_IDLE_SNEAKY_FIDGET);
}

}
