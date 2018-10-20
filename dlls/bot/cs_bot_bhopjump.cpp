
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
	// zombie only
	/*if (!m_bIsZombie)
		return false;
	// too slow or too heavy...
	if (pev->maxspeed < 285.0f || pev->gravity > 0.83f)
		return false;*/
	if (!IsRunning())
		return false;

	m_isBhopJumping = true;
	return false;
}

void CCSBot::BhopJump_UpdateJump()
{
	if (!m_isBhopJumping)
		return;

	if (!IsRunning() || pev->velocity.Length2D() < pev->maxspeed * 0.8f)
	{
		m_isBhopJumping = false;
		return; // too slow to bhop...
	}

	if (pev->velocity.z > 0.0f)
		return; // only dropping check

	float ground;
	if (!GetSimpleGroundHeightWithFloor(&pev->origin, &ground))
		return;

	const float flDistStand = 36;
	const float flDistDuck = 18;

	if (pev->origin.z - ground > flDistStand + flDistDuck) // ignores 
		return;

	if (pev->origin.z - ground > flDistStand)
	{
		Crouch();
		return;
	}
	// ready to jump
	if (pev->origin.z - ground > flDistDuck)
	{
		StandUp();
		m_buttonFlags |= IN_JUMP;
		m_isJumpCrouching = true;
		m_isJumpCrouched = false;
	}
	
}

void CCSBot::BhopJump_UpdateSync()
{
	if (!m_isBhopJumping)
		return;

	if (m_forwardSpeed < 0)
	{
		m_isBhopJumping = false;
		return; // unsupposed direction to bhop...
	}

	if (pev->flags & FL_ONGROUND)
		return;

	if (gpGlobals->time > m_flBhopSyncNext)
	{
		m_BhopLastSyncDir = m_BhopLastSyncDir == LEFT ? RIGHT : LEFT;
		m_flBhopSyncNext = gpGlobals->time + RANDOM_FLOAT(0.15f, 0.3f);
	}

	const float flSpeed = RANDOM_FLOAT(30.0f, 45.0f);
	m_lookYaw += m_BhopLastSyncDir == LEFT ? flSpeed : -flSpeed;

	if (IsViewMoving())
	{
		if (m_lookYawVel > 0.0f)
			StrafeLeft();
		else if (m_lookYawVel < 0.0f)
			StrafeRight();
	}
	
}