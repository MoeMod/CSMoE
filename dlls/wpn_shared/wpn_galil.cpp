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
#include "wpn_galil.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_galil, CGalil)

void CGalil::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_galil");

	Precache();
	m_iId = WEAPON_GALIL;
	SET_MODEL(ENT(pev), "models/w_galil.mdl");

	m_iDefaultAmmo = GALIL_DEFAULT_GIVE;

	FallInit();
}

void CGalil::Precache(void)
{
	PRECACHE_MODEL("models/v_galil.mdl");
	PRECACHE_MODEL("models/w_galil.mdl");

	PRECACHE_SOUND("weapons/galil-1.wav");
	PRECACHE_SOUND("weapons/galil-2.wav");
	PRECACHE_SOUND("weapons/galil_clipout.wav");
	PRECACHE_SOUND("weapons/galil_clipin.wav");
	PRECACHE_SOUND("weapons/galil_boltpull.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireGalil = PRECACHE_EVENT(1, "events/galil.sc");
}

int CGalil::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GALIL_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 17;
	p->iId = m_iId = WEAPON_GALIL;
	p->iFlags = 0;
	p->iWeight = GALIL_WEIGHT;

	return 1;
}

BOOL CGalil::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy("models/v_galil.mdl", "models/p_galil.mdl", GALIL_DRAW, "ak47", UseDecrement() != FALSE);
}

void CGalil::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		GalilFire(0.04 + (0.3) * m_flAccuracy, 0.0875s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		GalilFire(0.04 + (0.07) * m_flAccuracy, 0.0875s, FALSE);
	else
		GalilFire((0.0375) * m_flAccuracy, 0.0875s, FALSE);
}

void CGalil::GalilFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;

	if (m_flAccuracy > 1.25)
		m_flAccuracy = 1.25;

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
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireGalil, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 10000000), (int)(m_pPlayer->pev->punchangle.y * 10000000), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;
	m_NextInspect = gpGlobals->time;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
	else
		KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
}

void CGalil::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;
	m_NextInspect = gpGlobals->time + GALIL_RELOAD_TIME;
	if (DefaultReload(GALIL_MAX_CLIP, GALIL_RELOAD, GALIL_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			m_pPlayer->m_flNextAttack = 1.43s;
			m_flTimeWeaponIdle = GALIL_RELOAD_TIME + 0.5s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + GALIL_RELOAD_TIME;
		}
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CGalil::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(GALIL_IDLE1, UseDecrement() != FALSE);
}

float CGalil::GetDamage() const
{
	float flDamage = 30.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 42.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 42.0f;
#endif
	return flDamage;
}

void CGalil::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(6, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
