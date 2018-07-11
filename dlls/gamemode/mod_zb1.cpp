#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "bmodels.h"

#include "mod_zb1.h"

#include <algorithm>

CMod_Zombi::CMod_Zombi() // precache
{
	CVAR_SET_STRING("sv_skyname", "hk");

}

void CMod_Zombi::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

bool CMod_Zombi::CanPlayerBuy(CBasePlayer *player, bool display)
{
	// is the player alive?
	if (player->pev->deadflag != DEAD_NO)
		return false;

	if (player->m_iTeam == TEAM_TERRORIST)
		return false;

	return true;
}

void CMod_Zombi::Think()
{
	IBaseMod::Think();

	int iCountDown = m_fRoundCount - gpGlobals->time;
	if (iCountDown > 0 && iCountDown < 20)
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_ZombiSelectCount", UTIL_dtos1(20 - iCountDown));

		static const char *szCountDownSound[11] = {
			"", "vox/one.wav", "vox/two.wav", "vox/three.wav", "vox/four.wav", "vox/five.wav", "vox/six.wav",
			"vox/seven.wav", "vox/eight.wav", "vox/nine.wav", "vox/ten.wav"
		};

		if(iCountDown >= 10)
		CLIENT_COMMAND(NULL, "spk \"sound/%s\"", szCountDownSound[20 - iCountDown]);
	}
	else if (iCountDown == 20)
	{
		// select zombie
	}
}

void CMod_Zombi::RestartRound()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);

		if (!entity)
			continue;

		CBasePlayer *player = static_cast<CBasePlayer *>(entity);

		if (player->pev->deadflag != DEAD_DEAD && player->pev->deadflag != DEAD_RESPAWNABLE)
			continue;

		if (player->m_iTeam == TEAM_UNASSIGNED || player->m_iTeam == TEAM_SPECTATOR)
			continue;

		player->m_iTeam = CT;
	}

	CVAR_SET_FLOAT("mp_autoteambalance", 0.0f);
	IBaseMod::RestartRound();
	m_bTCantBuy = false;
}

void CMod_Zombi::PlayerSpawn(CBasePlayer *pPlayer)
{
	IBaseMod::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Give Armor
	pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	pPlayer->pev->armorvalue = 100;

}