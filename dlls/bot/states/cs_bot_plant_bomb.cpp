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

// Plant the bomb.

void PlantBombState::OnEnter(CCSBot *me)
{
	me->Crouch();
	me->SetDisposition(CCSBot::SELF_DEFENSE);

	float yaw = me->pev->v_angle.y;
	Vector2D dir(BotCOS(yaw), BotSIN(yaw));

	Vector down(me->pev->origin.x + 10.0f * dir.x, me->pev->origin.y + 10.0f * dir.y, me->GetFeetZ());
	me->SetLookAt("Plant bomb on floor", &down, PRIORITY_HIGH);
}

// Plant the bomb.

void PlantBombState::OnUpdate(CCSBot *me)
{
	CBasePlayerWeapon *gun = me->GetActiveWeapon();
	bool holdingC4 = false;
	if (gun != NULL)
	{
		if (FStrEq(STRING(gun->pev->classname), "weapon_c4"))
			holdingC4 = true;
	}

	// if we aren't holding the C4, grab it, otherwise plant it
	if (holdingC4)
		me->PrimaryAttack();
	else
		me->SelectItem("weapon_c4");

	// if we no longer have the C4, we've successfully planted
	if (!me->IsCarryingBomb())
	{
		// move to a hiding spot and watch the bomb
		me->SetTask(CCSBot::GUARD_TICKING_BOMB);
		me->Hide();
	}

	// if we time out, it's because we slipped into a non-plantable area
	const float timeout = 5.0f;
	if (gpGlobals->time - me->GetStateTimestamp() > timeout)
		me->Idle();
}

void PlantBombState::OnExit(CCSBot *me)
{
	// equip our rifle (in case we were interrupted while holding C4)
	me->EquipBestWeapon();
	me->StandUp();
	me->ResetStuckMonitor();
	me->SetDisposition(CCSBot::ENGAGE_AND_INVESTIGATE);
	me->ClearLookAt();
}
