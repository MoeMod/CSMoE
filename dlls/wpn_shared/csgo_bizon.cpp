/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "csgo_bizon.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum bizon_e
{
	BIZON_IDLE1,
	BIZON_RELOAD,
	BIZON_DRAW,
	BIZON_SHOOT1,
	BIZON_SHOOT2,
	BIZON_SHOOT3,
	BIZON_INSPECT
};

LINK_ENTITY_TO_CLASS(csgo_bizon, CBizon)

void CBizon::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_bizon");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_bizon.mdl");

	m_iDefaultAmmo = BIZON_MAX_CLIP;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	FallInit();
}

void CBizon::Precache(void)
{
	PRECACHE_MODEL("models/v_bizon.mdl");
	PRECACHE_MODEL("models/w_bizon.mdl");
	PRECACHE_MODEL("models/p_bizon.mdl");
	PRECACHE_MODEL("models/alt/v_bizon.mdl");

	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon-1.wav");
	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon_clipout.wav");
	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon_clipin.wav");
	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon_boltforward.wav");
	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon_boltback.wav");
	PRECACHE_SOUND("weapons/weapons/csgo_ports/bizon/bizon_draw.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireBizon = PRECACHE_EVENT(1, "events/bizon.sc");
}

int CBizon::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "57mm";
	p->iMaxAmmo1 = 100;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BIZON_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 8;
	p->iId = m_iId = WEAPON_P90;
	p->iFlags = 0;
	p->iWeight = P90_WEIGHT;

	return 1;
}

BOOL CBizon::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif

	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_flAccuracy = 0.2;
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_bizon.mdl" : "models/alt/v_bizon.mdl", "models/p_bizon.mdl", BIZON_DRAW, "carbine", UseDecrement() != FALSE);
}

void CBizon::PrimaryAttack(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		BizonFire((0.3) * m_flAccuracy, 0.083s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 170)
		BizonFire((0.115) * m_flAccuracy, 0.083s, FALSE);
	else
		BizonFire((0.045) * m_flAccuracy, 0.083s, FALSE);
}

void CBizon::BizonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175) + 0.45;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_57MM, GetDamage(), 0.885, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireBizon, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_NextInspect = gpGlobals->time;
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.9, 0.45, 0.35, 0.04, 5.25, 3.5, 4);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.45, 0.3, 0.2, 0.0275, 4.0, 2.25, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.275, 0.2, 0.125, 0.02, 3.0, 1.0, 9);
	else
		KickBack(0.3, 0.225, 0.125, 0.02, 3.25, 1.25, 8);
}

void CBizon::Reload(void)
{
	if (m_pPlayer->ammo_57mm <= 0)
		return;
	m_NextInspect = gpGlobals->time + BIZON_RELOAD_TIME;
	if (DefaultReload(BIZON_MAX_CLIP, BIZON_RELOAD, BIZON_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
	}
}

void CBizon::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BIZON_IDLE1, UseDecrement() != FALSE);
}

void CBizon::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(BIZON_INSPECT, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}

float CBizon::GetDamage()
{
	float flDamage = 27.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 31.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 40.0f;
#endif
	return flDamage;
}
}
