#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "bmodels.h"

#include "mod_none.h"

#include <algorithm>

void CMod_None::CheckMapConditions()
{
	IBaseMod::CheckMapConditions();
	m_mapBombZones.clear();
	if (m_bMapHasBombZone)
	{
		CBaseEntity *pEntity = nullptr;
		
		while (pEntity = UTIL_FindEntityByClassname(pEntity, "func_bomb_target"))
		{
			m_mapBombZones.emplace_back(pEntity, VecBModelOrigin(pEntity->pev));
		}
		// pEntity = nullptr;
		while (pEntity = UTIL_FindEntityByClassname(pEntity, "info_bomb_target"))
		{
			m_mapBombZones.emplace_back(pEntity, pEntity->pev->origin);
		}

		using EVpair_t = decltype(m_mapBombZones)::value_type;
		std::sort(m_mapBombZones.begin(), m_mapBombZones.end(), [](const EVpair_t &a, const EVpair_t &b) {return a.first->eoffset() < b.first->eoffset(); });
	}
}

void CMod_None::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_NONE);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	WRITE_BYTE(m_mapBombZones.size());
	for (const auto& EV : m_mapBombZones)
	{
		const Vector &pos(EV.second);
		WRITE_COORD(pos[0]);
		WRITE_COORD(pos[1]);
		WRITE_COORD(pos[2]);
	}

	MESSAGE_END();
}

bool CMod_None::CanPlayerBuy(CBasePlayer *player, bool display)
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

	int buyTime = (int)(CVAR_GET_FLOAT("mp_buytime") * 60.0f);

	if (buyTime < MIN_BUY_TIME)
	{
		buyTime = MIN_BUY_TIME;
		CVAR_SET_FLOAT("mp_buytime", (MIN_BUY_TIME / 60.0f));
	}

	if (gpGlobals->time - m_fRoundCount > buyTime)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#Cant_buy", UTIL_dtos1(buyTime));
		}

		return false;
	}

	if (player->m_bIsVIP)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#VIP_cant_buy");
		}

		return false;
	}

	if (m_bCTCantBuy && player->m_iTeam == CT)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#CT_cant_buy");
		}

		return false;
	}

	if (m_bTCantBuy && player->m_iTeam == TERRORIST)
	{
		if (display)
		{
			ClientPrint(player->pev, HUD_PRINTCENTER, "#Terrorist_cant_buy");
		}

		return false;
	}

	return true;
}