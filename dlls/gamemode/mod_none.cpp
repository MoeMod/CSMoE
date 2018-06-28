#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "bte_mod.h"

#include "bmodels.h"

#include "mod_none.h"

CMod_None::CMod_None()
{
	if (m_bMapHasBombZone)
	{
		CBaseEntity *pEntity = nullptr;
		while (pEntity = UTIL_FindEntityByClassname(pEntity, "func_bomb_target"))
		{
			m_mapBombZones.push_back(VecBModelOrigin(pEntity->pev));
		}
		// pEntity = nullptr;
		while (pEntity = UTIL_FindEntityByClassname(pEntity, "info_bomb_target"))
		{
			m_mapBombZones.push_back(pEntity->pev->origin);
		}
	}
}

void CMod_None::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_NONE);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(13); // MaxRound (mp_roundlimit)
	WRITE_BYTE(13); // MaxTime (mp_roundlimit)

	if (m_bMapHasBombZone) // BombTarget Position (for followicon & radar)
	{
		WRITE_BYTE(m_mapBombZones.size());
		for (const Vector & pos: m_mapBombZones)
		{
			WRITE_COORD(pos[0]);
			WRITE_COORD(pos[1]);
			WRITE_COORD(pos[2]);
		}
	}

	MESSAGE_END();
}