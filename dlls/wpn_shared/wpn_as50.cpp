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
#include "wpn_as50.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum CAS50::as50_e
{
	AS50_IDLE,
	AS50_SHOOT1,
	AS50_SHOOT2,
	AS50_SHOOT3,
	AS50_RELOAD,
	AS50_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_as50, CAS50)

const int AS50_MAXCLIP = 5;

void CAS50::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_as50");

	Precache();
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_as50.mdl");

	m_iDefaultAmmo = AS50_MAXCLIP;
	FallInit();
}

void CAS50::Precache(void)
{
	PRECACHE_MODEL("models/v_as50.mdl");
	PRECACHE_MODEL("models/p_as50.mdl");
	PRECACHE_MODEL("models/w_as50.mdl");

	PRECACHE_SOUND("weapons/as50-1.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireAS50 = PRECACHE_EVENT(1, "events/as50.sc");
}

int CAS50::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50BMG";
	p->iMaxAmmo1 = MAX_AMMO_50BMG;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AS50_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_AWP;
	p->iFlags = 0;
	p->iWeight = AWP_WEIGHT;

	return 1;
}

BOOL CAS50::Deploy(void)
{
	m_flAccuracy = 0.9;

	return DefaultDeploy("models/v_as50.mdl", "models/p_as50.mdl", AS50_DRAW, "rifle", UseDecrement() != FALSE);
}

void CAS50::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = AS50_ZOOM_FOV_1; break;
	case AS50_ZOOM_FOV_1: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = AS50_ZOOM_FOV_2; break;
	default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	if(m_pPlayer->m_iFOV  != 90)
		m_flAccuracy = 1.0;

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CAS50::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		AS50Fire((0.7) * (1 - m_flAccuracy), 0.4s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		AS50Fire((0.3) * (1 - m_flAccuracy), 0.4s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		AS50Fire((0.03) * (1 - m_flAccuracy), 0.4s, FALSE);
	else
		AS50Fire((0.15) * (1 - m_flAccuracy), 0.4s, FALSE);
}

void CAS50::AS50Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov == 90)
		flSpread += 0.1;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.375 + 0.6;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;
	}

	m_flLastFire = gpGlobals->time;

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

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55s;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_338MAG, GetDamage(), AS50_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireAS50, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 10.0) + m_pPlayer->pev->punchangle.x * 0.75;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 2.0);
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 9.0) + m_pPlayer->pev->punchangle.x * 0.75;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
	}
}

void CAS50::Reload(void)
{
	if (DefaultReload(AS50_MAXCLIP, AS50_RELOAD, AS50_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = AS50_ZOOM_FOV_2;
			SecondaryAttack();
		}
	}
}

void CAS50::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(AS50_IDLE, UseDecrement() != FALSE);
	}
}

float CAS50::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return AS50_MAX_SPEED;

	return AS50_MAX_SPEED_ZOOM;
}

float CAS50::GetDamage() const
{
	float flDamage = 110.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage =  249.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage =  110.0f;
#endif
	return flDamage;
}
}
