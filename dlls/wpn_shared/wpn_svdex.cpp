/*
wpn_svdex.cpp - CSMoE Gameplay server : Zombie Hero
Copyright (C) 2019 TmNine!~

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_svdex.h"

enum svdex_e
{
	SVDEX_IDLEA,
	SVDEX_SHOOTA,
	SVDEX_RELOAD,
	SVDEX_DRAWA,
	SVDEX_IDLEB,
	SVDEX_SHOOTB_1,
	SVDEX_SHOOTB_LAST,
	SVDEX_DRAWB,
	SVDEX_MOVE_GRENADE,
	SVDEX_MOVE_CARBINE
};

LINK_ENTITY_TO_CLASS(weapon_svdex, CSVDEX)

void CSVDEX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_svdex");

	Precache();
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), "models/w_svdex.mdl");

	m_iDefaultAmmo = 20;  //????
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = true;

	FallInit();
}

void CSVDEX::Precache(void)
{
	PRECACHE_MODEL("models/v_svdex.mdl");
	PRECACHE_MODEL("models/w_svdex.mdl");

	PRECACHE_SOUND("weapons/svdex-1.wav");
	PRECACHE_SOUND("weapons/svdex-launcher.wav");
	PRECACHE_SOUND("weapons/svdex_exp.wav");
	PRECACHE_SOUND("weapons/svdex_foley1.wav");
	PRECACHE_SOUND("weapons/svdex_foley2.wav");
	PRECACHE_SOUND("weapons/svdex_foley3.wav");
	PRECACHE_SOUND("weapons/svdex_foley4.wav");
	PRECACHE_SOUND("weapons/svdex_draw.wav");
	PRECACHE_SOUND("weapons/svdex_clipon.wav");
	PRECACHE_SOUND("weapons/svdex_clipin.wav");
	PRECACHE_SOUND("weapons/svdex_clipout.wav");

	m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireSVDEX = PRECACHE_EVENT(1, "events/svdex.sc");
}

int CSVDEX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 20;
	p->iSlot = 0;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M4A1_WEIGHT;

	return 1;
}

BOOL CSVDEX::Deploy(void)
{
	m_bDelayFire = true;
	iShellOn = 1;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
	return DefaultDeploy("models/v_svdex.mdl", "models/p_svdex.mdl", SVDEX_DRAWA, "rifle", UseDecrement() != FALSE);
}

void CSVDEX::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SVDEX_MOVE_CARBINE, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SVDEX_MOVE_GRENADE, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "rifle");
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.9;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2;
}

void CSVDEX::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		/*if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SVDEXFire(0.035 + (0.4) * m_flAccuracy, 0.0875, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			SVDEXFire(0.035 + (0.07) * m_flAccuracy, 0.0875, FALSE);
		else
			SVDEXFire((0.025) * m_flAccuracy, 0.0875, FALSE);*/
		SVDEXFire2(3.0, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SVDEXFire1(0.8, 0.346, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			SVDEXFire1(0.15, 0.346, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			SVDEXFire1(0.1, 0.346, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			SVDEXFire1(0.003, 0.346, FALSE);
		else
			SVDEXFire1(0.007, 0.346, FALSE);
	}
}

void CSVDEX::SVDEXFire1(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35;

	if (m_flAccuracy > 1)
		m_flAccuracy = 1;

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
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 490, 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSVDEX, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;

	m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 0.55, 1.0);
	m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.65, 0.95);
}

void CSVDEX::SVDEXFire2(float flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	//m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;	// 1000
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;	// 512

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	//vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 490, 0.99, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	const bool bEmpty = false;
	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSVDEX, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, bEmpty);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
}

void CSVDEX::Reload(void)
{
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return;

	if (m_pPlayer->ammo_556nato <= 0)
		return;


	if (DefaultReload(20, SVDEX_RELOAD, 3.8))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CSVDEX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		SendWeaponAnim(SVDEX_IDLEB, UseDecrement() != FALSE);
	else
		SendWeaponAnim(SVDEX_IDLEA, UseDecrement() != FALSE);
}
