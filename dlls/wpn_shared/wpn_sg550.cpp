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
#include "wpn_sg550.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum sg550_e
{
	SG550_IDLE,
	SG550_SHOOT,
	SG550_SHOOT2,
	SG550_RELOAD,
	SG550_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_sg550, CSG550)

void CSG550::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sg550");

	Precache();
	m_iId = WEAPON_SG550;
	SET_MODEL(ENT(pev), "models/w_sg550.mdl");
	m_iDefaultAmmo = SG550_DEFAULT_GIVE;

	FallInit();
}

void CSG550::Precache(void)
{
	PRECACHE_MODEL("models/v_sg550.mdl");
	PRECACHE_MODEL("models/w_sg550.mdl");

	PRECACHE_SOUND("weapons/sg550-1.wav");
	PRECACHE_SOUND("weapons/sg550_boltpull.wav");
	PRECACHE_SOUND("weapons/sg550_clipin.wav");
	PRECACHE_SOUND("weapons/sg550_clipout.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireSG550 = PRECACHE_EVENT(1, "events/sg550.sc");
}

int CSG550::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SG550_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 16;
	p->iId = m_iId = WEAPON_SG550;
	p->iFlags = 0;
	p->iWeight = SG550_WEIGHT;

	return 1;
}

BOOL CSG550::Deploy(void)
{
	m_flAccuracy = 0.2;

	return DefaultDeploy("models/v_sg550.mdl", "models/p_sg550.mdl", SG550_DRAW, "rifle", UseDecrement() != FALSE);
}

void CSG550::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
		case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CSG550::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		SG550Fire((0.45) * (1 - m_flAccuracy), 0.25s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		SG550Fire(0.15, 0.25s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		SG550Fire((0.04) * (1 - m_flAccuracy), 0.25s, FALSE);
	else
		SG550Fire((0.05) * (1 - m_flAccuracy), 0.25s, FALSE);
}

void CSG550::SG550Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov == 90)
		flSpread += 0.025;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.35 + 0.65;

		if (m_flAccuracy > 0.98)
			m_flAccuracy = 0.98;
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

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 70, 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSG550, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 1.5, 1.75) + m_pPlayer->pev->punchangle.x * 0.25;
	m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -1.0, 1.0);
}

void CSG550::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(SG550_MAX_CLIP, SG550_RELOAD, 3.35s))
	{
		m_flAccuracy = 0.2;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
			SecondaryAttack();
		}
	}
}

void CSG550::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(SG550_IDLE, UseDecrement() != FALSE);
	}
}

float CSG550::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 210;

	return 150;
}

}
