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
#include "z4b_m1887se.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_m1887se, CM1887se)

void CM1887se::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_m1887se");

	Precache();
	m_iId = WEAPON_SCOUT;
	SET_MODEL(ENT(pev), "models/pw_m1887se.mdl");

	m_iDefaultAmmo = M1887SE_MAX_CLIP;
	FallInit();
}

void CM1887se::Precache(void)
{
	PRECACHE_MODEL("models/v_m1887se.mdl");
	PRECACHE_MODEL("models/pw_m1887se.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	m_usFireM1887se = PRECACHE_EVENT(1, "events/m1887se.sc");
}

int CM1887se::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M1887SE_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 9;
	p->iId = m_iId = WEAPON_SCOUT;
	p->iFlags = 0;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}

BOOL CM1887se::Deploy(void)
{
	if (DefaultDeploy("models/v_m1887se.mdl", "models/pw_m1887se.mdl", M1887_DRAW, "rifle", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.25s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
		return TRUE;
	}
	return FALSE;
}

void CM1887se::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV == 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 40;
	else if (m_pPlayer->m_iFOV == 40)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 15;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CM1887se::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		M1887seFire(0.2, 0.7s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 170)
		M1887seFire(0.075, 0.7s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		M1887seFire(0.0, 0.7s, FALSE);
	else
		M1887seFire(0.007, 0.7s, FALSE);
}

void CM1887se::M1887seFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();

		if (m_iClip == 0)
			PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}

	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_bResumeZoom = true;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}
	else
		flSpread += 0.025;

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

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.56s;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_BUCKSHOT, GetDamage(), 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif
	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireM1887se, 0, g_vecZero, g_vecZero, vecDir.x * 1000, vecDir.y * 1000, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.7s;

	m_fInSpecialReload = 0;

	m_pPlayer->pev->punchangle.x -= 2;
	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
}

void CM1887se::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == M1887SE_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 15;
			SecondaryAttack();
		}

		SendWeaponAnim(M1887_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;
		SendWeaponAnim(M1887_RELOAD, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.45s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
	}
}

void CM1887se::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = zero_duration;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != M1887SE_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(M1887_PUMP, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
			}
		}
		else
			SendWeaponAnim(M1887_IDLE, UseDecrement() != FALSE);
	}
}

float CM1887se::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 240;

	return 210;
}


float CM1887se::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZBS)
		return 2250.0f;
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 2250.0f;
#endif
	return 75.0f;
}

}
