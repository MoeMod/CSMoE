
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "skill.h"
#include "items.h"

namespace sv {

CHalfLifeRules::CHalfLifeRules()
{
	m_bFreezePeriod = FALSE;
	RefreshSkillData();
}

void CHalfLifeRules::Think()
{
	;
}

BOOL CHalfLifeRules::IsMultiplayer()
{
	return FALSE;
}

BOOL CHalfLifeRules::IsDeathmatch()
{
	return FALSE;
}

BOOL CHalfLifeRules::IsCoOp()
{
	return FALSE;
}

BOOL CHalfLifeRules::FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (!pPlayer->m_pActiveItem)
	{
		// player doesn't have an active item!
		return TRUE;
	}

	if (!pPlayer->m_pActiveItem->CanHolster())
	{
		return FALSE;
	}

	return TRUE;
}

BOOL CHalfLifeRules::GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon)
{
	return FALSE;
}

BOOL CHalfLifeRules::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	return TRUE;
}

void CHalfLifeRules::InitHUD(CBasePlayer *pl)
{
	;
}

void CHalfLifeRules::ClientDisconnected(edict_t *pClient)
{
	;
}

float CHalfLifeRules::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	// subtract off the speed at which a player is allowed to fall without being hurt,
	// so damage will be based on speed beyond that, not the entire fall
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED;
}

void CHalfLifeRules::PlayerSpawn(CBasePlayer *pPlayer)
{
	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

	pPlayer->GiveNamedItem("weapon_knife");
	pPlayer->GiveNamedItem("weapon_usp");
	pPlayer->GiveNamedItem("weapon_mp5navy");
	pPlayer->GiveNamedItem("weapon_hegrenade");

	pPlayer->GiveAmmo(60, "9mm", 120);
	pPlayer->GiveAmmo(24, "45acp", 100);
}

BOOL CHalfLifeRules::AllowAutoTargetCrosshair()
{
	return (g_iSkillLevel == SKILL_EASY);
}

void CHalfLifeRules::PlayerThink(CBasePlayer *pPlayer)
{
	;
}

BOOL CHalfLifeRules::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	return TRUE;
}

time_point_t CHalfLifeRules::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gpGlobals->time;
}

int CHalfLifeRules::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	return 1;
}

void CHalfLifeRules::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	;
}

void CHalfLifeRules::DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	;
}

void CHalfLifeRules::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	;
}

time_point_t CHalfLifeRules::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	return invalid_time_point;
}

time_point_t CHalfLifeRules::FlWeaponTryRespawn(CBasePlayerItem *pWeapon)
{
	return invalid_time_point;
}

Vector CHalfLifeRules::VecWeaponRespawnSpot(CBasePlayerItem *pWeapon)
{
	return pWeapon->pev->origin;
}

edict_t *CHalfLifeRules::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	CBaseEntity *pSpot = UTIL_FindEntityByClassname(NULL, "info_player_start");

	if (!pSpot)
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	pPlayer->pev->origin = pSpot->pev->origin + Vector(0, 0, 1);
	pPlayer->pev->v_angle  = g_vecZero;
	pPlayer->pev->velocity = g_vecZero;
	pPlayer->pev->angles = pSpot->pev->angles;
	pPlayer->pev->punchangle = g_vecZero;
	pPlayer->pev->fixangle = 1;

	return pSpot->edict();
}

int CHalfLifeRules::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	return GR_WEAPON_RESPAWN_NO;
}

BOOL CHalfLifeRules::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	return TRUE;
}

void CHalfLifeRules::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
	;
}

int CHalfLifeRules::ItemShouldRespawn(CItem *pItem)
{
	return GR_ITEM_RESPAWN_NO;
}

time_point_t CHalfLifeRules::FlItemRespawnTime(CItem *pItem)
{
	return invalid_time_point;
}

Vector CHalfLifeRules::VecItemRespawnSpot(CItem *pItem)
{
	return pItem->pev->origin;
}

BOOL CHalfLifeRules::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	return TRUE;
}

void CHalfLifeRules::PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount)
{
	;
}

int CHalfLifeRules::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	return GR_AMMO_RESPAWN_NO;
}

time_point_t CHalfLifeRules::FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo)
{
	return invalid_time_point;
}

Vector CHalfLifeRules::VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo)
{
	return pAmmo->pev->origin;
}

float CHalfLifeRules::FlHealthChargerRechargeTime()
{
	return 0;
}

int CHalfLifeRules::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_NO;
}

int CHalfLifeRules::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_NO;
}

int CHalfLifeRules::PlayerRelationship(CBasePlayer *pPlayer, CBaseEntity *pTarget)
{
	// why would a single player in half life need this?
	return GR_NOTTEAMMATE;
}

BOOL CHalfLifeRules::FAllowMonsters()
{
	return TRUE;
}

}
