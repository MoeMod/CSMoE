#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "gamerules.h"

namespace sv {

// global vars
DLL_GLOBAL CBaseEntity *g_pLastSpawn;
DLL_GLOBAL CBaseEntity *g_pLastCTSpawn, *g_pLastTerroristSpawn;

// utils
inline int FNullEnt(CBaseEntity *ent) { return (!ent) || FNullEnt(ent->edict()); }

// main code

BOOL IsSpawnPointValid(CBaseEntity *pPlayer, CBaseEntity *pSpot)
{
	CBaseEntity *ent = NULL;

	if (!pSpot->IsTriggered(pPlayer))
		return FALSE;

	while ((ent = UTIL_FindEntityInSphere(ent, pSpot->pev->origin, 64)) != NULL)
	{
		// if ent is a client, don't spawn on 'em
		if (ent->IsPlayer() && ent != pPlayer)
			return FALSE;
	}

	return TRUE;
}

edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer)
{
	CBaseEntity *pSpot;
	edict_t *player = pPlayer->edict();

	// choose a info_player_deathmatch point
	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_coop");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;

		pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_start");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}
	// VIP spawn point
	else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer *)pPlayer)->m_bIsVIP)
	{
		pSpot = UTIL_FindEntityByClassname(NULL, "info_vip_start");

		// skip over the null point
		if (!FNullEnt(pSpot))
		{
			goto ReturnSpot;
		}

		goto CTSpawn;
	}
	// the counter-terrorist spawns at "info_player_start"
	else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer *)pPlayer)->m_iTeam == CT)
	{
	CTSpawn:
		pSpot = g_pLastCTSpawn;

		if (((CBasePlayer *)pPlayer)->SelectSpawnSpot("info_player_start", pSpot))
		{
			goto ReturnSpot;
		}
	}
	// The terrorist spawn points
	else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer *)pPlayer)->m_iTeam == TERRORIST)
	{
		pSpot = g_pLastTerroristSpawn;

		if (((CBasePlayer *)pPlayer)->SelectSpawnSpot("info_player_deathmatch", pSpot))
		{
			goto ReturnSpot;
		}
	}

	// If startspot is set, (re)spawn there.
	if (FStringNull(gpGlobals->startspot) || !Q_strlen(STRING(gpGlobals->startspot)))
	{
		pSpot = UTIL_FindEntityByClassname(NULL, "info_player_deathmatch");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}
	else
	{
		pSpot = UTIL_FindEntityByTargetname(NULL, STRING(gpGlobals->startspot));

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}

ReturnSpot:
	if (FNullEnt(pSpot))
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	if (((CBasePlayer *)pPlayer)->m_iTeam == TERRORIST)
		g_pLastTerroristSpawn = pSpot;
	else
		g_pLastCTSpawn = pSpot;

	return pSpot->edict();
}

}
