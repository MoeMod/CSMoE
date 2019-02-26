
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

bool CCSBot::BhopJump_Start()
{
	// dont jump when bot_stop 1
	if (cv_bot_stop.value != 0.0f)
		return false;
	// zombie only
	if (!m_bIsZombie)
		return false;
	// too slow or too heavy...
	if (pev->maxspeed < 285.0f || pev->gravity > 0.83f)
		return false;
	// should be fully run-up
	if (!IsRunning() || pev->velocity.Length2D() < pev->maxspeed * 0.9f)
		return false;

	// starts bhopping
	m_isBhopJumping = true;

	// attacking when bhopping will make space-walking effect... especially for speed zombie
	if(IsUsingKnife())
		m_isRapidFiring = true;

	return false;
}

void CCSBot::BhopJump_UpdateJump()
{
	if (!m_isBhopJumping)
		return;

	// too slow to bhop...
	if (!IsRunning() || pev->velocity.Length2D() < pev->maxspeed * 0.5f)
	{
		m_isBhopJumping = false;
		m_isRapidFiring = false;
		return;
	}

	// only dropping check
	if (pev->velocity.z > 0.0f)
		return;

	float ground;
	if (!GetSimpleGroundHeightWithFloor(&pev->origin, &ground))
		return;

	const float flDistStand = 36;
	const float flDistDuck = 18;

	if (pev->origin.z - ground > flDistStand + flDistDuck) // ignores 
		return;

	if (pev->origin.z - ground > flDistStand && !(pev->flags & FL_ONGROUND))
	{
		Crouch();
		m_buttonFlags |= IN_DUCK;
		return;
	}

	// in case of jumpping ahead
	if ((m_aimSpot - pev->origin).Length2D() < 60.0f)
		return;

	// ready to jump
	if (pev->origin.z - ground > flDistDuck || pev->flags & FL_ONGROUND)
	{
		
		StandUp();
		m_buttonFlags &= ~IN_DUCK;
		m_buttonFlags |= IN_JUMP;
		m_isJumpCrouching = true;
		m_isJumpCrouched = false;
	}
	
}

void CCSBot::BhopJump_UpdateSync()
{
	if (!m_isBhopJumping)
		return;

	// unsupposed direction to bhop...
	if (m_forwardSpeed < 0)
	{
		m_isBhopJumping = false;
		m_isRapidFiring = false;
		return; 
	}

	if (pev->flags & FL_ONGROUND)
		return;

	if (gpGlobals->time > m_flBhopSyncNext)
	{
		vec_t idealYaw = UTIL_VecToAngles(m_aimSpot - pev->origin).y;

		if (idealYaw - m_lookYaw > 30.f) // should StrafeLeft
		{
			m_BhopLastSyncDir = LEFT;
			m_flBhopSyncNext = gpGlobals->time + 0.2f;
		}
		else if (m_lookYaw - idealYaw > 30.f) // should StrafeRight
		{
			m_BhopLastSyncDir = RIGHT;
			m_flBhopSyncNext = gpGlobals->time + 0.2f;
		}
		else // default (random direction)
		{
			m_BhopLastSyncDir = m_BhopLastSyncDir == LEFT ? RIGHT : LEFT;
			m_flBhopSyncNext = gpGlobals->time + RANDOM_FLOAT(0.15f, 0.3f);
		}
		
	}

	const float flSpeed = RANDOM_FLOAT(30.0f, 45.0f);
	m_lookYaw += m_BhopLastSyncDir == LEFT ? flSpeed : -flSpeed;

	if (IsViewMoving())
	{
		bool bLeftDirection = m_lookYawVel > 0.0f;

		static cvar_t *sv_maxspeed = CVAR_GET_POINTER("sv_maxspeed");
		if (pev->velocity.Length() > sv_maxspeed->value * 1.08)
		{
			// change direction to decrease speed in case of stuck...
			bLeftDirection = !bLeftDirection;
			Crouch();
			m_buttonFlags |= IN_DUCK;
		}

		// stupider bot has less sync-rate
		if (RANDOM_FLOAT(0.0f, 1.0f + GetProfile()->GetSkill()) < 0.3f)
			bLeftDirection = !bLeftDirection;

		if (bLeftDirection)
			StrafeLeft();
		else
			StrafeRight();
	}
	
}

bool CCSBot::Knockback(CBasePlayer *attacker, const KnockbackData &data)
{
	bool value = CBasePlayer::Knockback(attacker, data);
	if(!value)
		return false;

	if (cv_bot_stop.value != 0.0f)
		return value;

	if (RANDOM_FLOAT(0.0f, 1.0f + GetProfile()->GetSkill()) < 0.75f)
		return value;

	if (!IsCrouching())
	{
		Crouch();
		m_buttonFlags |= IN_DUCK;
	}
	else
	{
		m_isBhopJumping = true;
		m_isAimingAtEnemy = false;

		StandUp();
		m_buttonFlags &= ~IN_DUCK;

		m_buttonFlags |= IN_JUMP;

		m_BhopLastSyncDir = m_BhopLastSyncDir == LEFT ? RIGHT : LEFT;
		m_flBhopSyncNext = gpGlobals->time + 0.15f;
		//m_lookYawVel *= 3.0f;
	}

	return value;
}