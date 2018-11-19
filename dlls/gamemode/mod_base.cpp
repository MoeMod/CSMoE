#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "enginecallback.h"

#include "bmodels.h"

#include "mod_base.h"

#include "player/csdm_randomspawn.h"

void IBaseMod::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_Default> up(new CPlayerModStrategy_Default(player));
	player->m_pModStrategy = std::move(up);
}

void IBaseMod_RemoveObjects::CheckMapConditions()
{
	//CHalfLifeMultiplay::CheckMapConditions();
	
	// Check to see if this map has a bomb target in it
	m_bMapHasBombTarget = false;
	m_bMapHasBombZone = false;

	// Check to see if this map has hostage rescue zones
	m_bMapHasRescueZone = false;

	// See if the map has func_buyzone entities
	// Used by CBasePlayer::HandleSignals() to support maps without these entities
	m_bMapHasBuyZone = (UTIL_FindEntityByClassname(NULL, "func_buyzone") != NULL);

	// GOOSEMAN : See if this map has func_escapezone entities
	m_bMapHasEscapeZone = false;

	// Check to see if this map has VIP safety zones
	m_iMapHasVIPSafetyZone = MAP_HAVE_VIP_SAFETYZONE_NO;

	// Hostage
	/*CBaseEntity *hostage = nullptr;
	while ((hostage = UTIL_FindEntityByClassname(hostage, "hostage_entity")) != nullptr)
	{
		// should be removed.
		REMOVE_ENTITY(hostage->edict());
	}*/
}

BOOL IBaseMod_RemoveObjects::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	if(!Q_strcmp(STRING(pEntity->pev->classname), "func_bomb_target") || !Q_strcmp(STRING(pEntity->pev->classname), "info_bomb_target"))
	{ 
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_hostage_rescue"))
	{
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_escapezone"))
	{
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "func_vip_safetyzone"))
	{
		return FALSE;
	}
	if (!Q_strcmp(STRING(pEntity->pev->classname), "hostage_entity"))
	{
		return FALSE;
	}
	return TRUE;
}

void IBaseMod_RemoveObjects::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_NONE);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	// CAN BE EXTENDED.
	MESSAGE_END();
}

edict_t *IBaseMod_RandomSpawn::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
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

	if (IsMultiplayer())
	{
		if (pentSpawnSpot->v.target)
		{
			FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);
		}
	}

	return pentSpawnSpot;
}
