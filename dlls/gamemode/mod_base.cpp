#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "enginecallback.h"

#include "bmodels.h"

#include "mod_base.h"

#include "player/csdm_randomspawn.h"
#include "player/player_mod_strategy.h"

void IBaseMod::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_Default> up(new CPlayerModStrategy_Default(player));
	player->m_pModStrategy = std::move(up);
}

void _IBaseMod_RemoveObjects_CheckMapConditions_impl(IBaseMod *mod)
{
	//CHalfLifeMultiplay::CheckMapConditions();
	
	// Check to see if this map has a bomb target in it
	mod->m_bMapHasBombTarget = false;
	mod->m_bMapHasBombZone = false;

	// Check to see if this map has hostage rescue zones
	mod->m_bMapHasRescueZone = false;

	// See if the map has func_buyzone entities
	// Used by CBasePlayer::HandleSignals() to support maps without these entities
	mod->m_bMapHasBuyZone = (UTIL_FindEntityByClassname(NULL, "func_buyzone") != NULL);

	// GOOSEMAN : See if this map has func_escapezone entities
	mod->m_bMapHasEscapeZone = false;

	// Check to see if this map has VIP safety zones
	mod->m_iMapHasVIPSafetyZone = MAP_HAVE_VIP_SAFETYZONE_NO;

	// Hostage
	/*CBaseEntity *hostage = nullptr;
	while ((hostage = UTIL_FindEntityByClassname(hostage, "hostage_entity")) != nullptr)
	{
		// should be removed.
		REMOVE_ENTITY(hostage->edict());
	}*/
}

BOOL _IBaseMod_RemoveObjects_IsAllowedToSpawn_impl(IBaseMod *mod, CBaseEntity *pEntity) {
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_bomb_target") ||
	    !Q_strcmp(STRING(pEntity->pev->classname), "info_bomb_target")) {
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_hostage_rescue")) {
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_escapezone")) {
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_vip_safetyzone")) {
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "hostage_entity")) {
		return FALSE;
	}
	return TRUE;
}

edict_t *_IBaseMod_RandomSpawn_GetPlayerSpawnSpot_impl(IBaseMod *mod, CBasePlayer *pPlayer)
{
	// completely rewrites it

	// select spawnpoint from both teams.
	TeamName iBackupTeam = pPlayer->m_iTeam;
	pPlayer->m_iTeam = static_cast<TeamName>(RANDOM_LONG(TERRORIST, CT));

	// gat valid spawn point
	edict_t *pentSpawnSpot = EntSelectSpawnPoint(pPlayer);

	pPlayer->m_iTeam = iBackupTeam;

	// Move the player to the place it said.
	// Note that here has been modified
	if (!CSDM_DoRandomSpawn(pPlayer))
	{
		pPlayer->pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
		pPlayer->pev->v_angle = g_vecZero;
		pPlayer->pev->velocity = g_vecZero;
		pPlayer->pev->angles = VARS(pentSpawnSpot)->angles;
	}

	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = 1;

	if (mod->IsMultiplayer())
	{
		if (pentSpawnSpot->v.target)
		{
			FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);
		}
	}

	return pentSpawnSpot;
}
