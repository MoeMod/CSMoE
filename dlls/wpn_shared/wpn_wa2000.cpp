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
#include "wpn_wa2000.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum wa2000_e
{
	WA2000_IDLE,
	WA2000_SHOOT,
	WA2000_SHOOT2,
	WA2000_RELOAD,
	WA2000_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_wa2000, CWA2000)

const int WA2000_MAX_CLIP = 12;

const char *CWA2000::GetCSModelName()
{
	return "models/w_wa2000.mdl";
}

void CWA2000::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_wa2000");

	Precache();
	m_iId = WEAPON_G3SG1;
	SET_MODEL(ENT(pev), GetCSModelName());
	m_iDefaultAmmo = WA2000_MAX_CLIP;

	FallInit();
}

void CWA2000::Precache(void)
{
	PRECACHE_MODEL("models/v_wa2000.mdl");
	PRECACHE_MODEL("models/p_wa2000.mdl");
	PRECACHE_MODEL("models/w_wa2000.mdl");

	PRECACHE_SOUND("weapons/wa2000-1.wav");
	PRECACHE_SOUND("weapons/wa2000_boltpull.wav");
	PRECACHE_SOUND("weapons/wa2000_clipin1.wav");
	PRECACHE_SOUND("weapons/wa2000_clipin2.wav");
	PRECACHE_SOUND("weapons/wa2000_clipout1.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireWA2000 = PRECACHE_EVENT(1, "events/wa2000.sc");
}

int CWA2000::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = MAX_AMMO_762NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WA2000_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_G3SG1;
	p->iFlags = 0;
	p->iWeight = G3SG1_WEIGHT;

	return 1;
}

BOOL CWA2000::Deploy(void)
{
	m_flAccuracy = 0.9;

	return DefaultDeploy("models/v_wa2000.mdl", "models/p_wa2000.mdl", WA2000_DRAW, "rifle", UseDecrement() != FALSE);
}

void CWA2000::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 38; break;
	case 38: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 12; break;
	default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CWA2000::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		WA2000Fire((0.8) * (1 - m_flAccuracy), 0.5, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		WA2000Fire((0.5) * (1 - m_flAccuracy), 0.5, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		WA2000Fire((0.04) * (1 - m_flAccuracy), 0.5, FALSE);
	else
		WA2000Fire((0.055) * (1 - m_flAccuracy), 0.5, FALSE);
}

void CWA2000::WA2000Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov == 90)
		flSpread += 0.025;

	if (m_flLastFire)
	{
		m_flAccuracy = (gpGlobals->time - m_flLastFire) * 0.5 + 0.7;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, 110, 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireWA2000, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;

	m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 1.5, 3.0) + m_pPlayer->pev->punchangle.x * 0.75;
	m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -1.3, 1.3);
}

void CWA2000::Reload(void)
{
	if (m_pPlayer->ammo_762nato <= 0)
		return;

	if (DefaultReload(WA2000_MAX_CLIP, WA2000_RELOAD, 3.45))
	{
		m_flAccuracy = 0.9;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 12;
			SecondaryAttack();
		}
	}
}

void CWA2000::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60;
		SendWeaponAnim(WA2000_IDLE, UseDecrement() != FALSE);
	}
}

float CWA2000::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 210;

	return 150;
}

}
