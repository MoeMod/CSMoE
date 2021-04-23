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
#include "wpn_tar21.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_tar21, CTAR21)

const int TAR21_MAX_CLIP = 30;

void CTAR21::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_tar21");

	Precache();
	m_iId = WEAPON_AUG;
	SET_MODEL(ENT(pev), "models/w_tar21.mdl");

	m_iDefaultAmmo = TAR21_MAX_CLIP;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CTAR21::Precache(void)
{
	PRECACHE_MODEL("models/v_tar21.mdl");
	PRECACHE_MODEL("models/p_tar21.mdl");
	PRECACHE_MODEL("models/w_tar21.mdl");

	PRECACHE_SOUND("weapons/tar21-1.wav");
	PRECACHE_SOUND("weapons/tar21_clipout.wav");
	PRECACHE_SOUND("weapons/tar21_clipin.wav");
	PRECACHE_SOUND("weapons/tar21_boltpull.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireTar21 = PRECACHE_EVENT(1, "events/tar21.sc");
}

int CTAR21::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = TAR21_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 14;
	p->iId = m_iId = WEAPON_AUG;
	p->iFlags = 0;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

BOOL CTAR21::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_tar21.mdl", "models/p_tar21.mdl", TAR21_DRAW, "carbine", UseDecrement() != FALSE);
}

void CTAR21::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CTAR21::PrimaryAttack(void)
{
	const auto FireRate = (m_pPlayer->pev->fov == 90) ? 0.095s : 0.13s;
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		TAR21Fire(0.05 * m_flAccuracy + (0.04), FireRate, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 130)
		TAR21Fire(0.04 * m_flAccuracy + (0.02), FireRate, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		TAR21Fire(0.018 * m_flAccuracy, FireRate, FALSE);
	else if (m_pPlayer->pev->fov == 90)
		TAR21Fire((0.024) * m_flAccuracy, FireRate, FALSE);
	else
		TAR21Fire((0.008) * m_flAccuracy, FireRate, FALSE);
}

void CTAR21::TAR21Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.29;

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
	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 32, 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireTar21, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1, 0.45, 0.26, 0.05, 4, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
	else if (m_pPlayer->pev->fov != 90)
		KickBack(0.4, 0.35, 0.15, 0.01, 3.5, 1.5, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.525, 0.3, 0.26, 0.111, 3.0, 1.9, 9);
	else
		KickBack(0.6, 0.6, 0.3, 0.075, 3.5, 2.0, 8);
}

void CTAR21::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(TAR21_MAX_CLIP, TAR21_RELOAD, 3.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CTAR21::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(TAR21_IDLE1, UseDecrement() != FALSE);
}

}
