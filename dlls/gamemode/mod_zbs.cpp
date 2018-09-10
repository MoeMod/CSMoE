#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "bmodels.h"

#include "mod_zbs.h"

#include <algorithm>

void CMod_ZombieScenario::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZBS);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

bool CMod_ZombieScenario::CanPlayerBuy(CBasePlayer *player, bool display)
{
	// is the player alive?
	if (player->pev->deadflag != DEAD_NO)
	{
		return false;
	}

	// is the player in a buy zone?
	if (!(player->m_signals.GetState() & SIGNAL_BUY))
	{
		return false;
	}

	return true;
}