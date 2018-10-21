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
#include "wpn_ak47l.h"

enum ak47_e
{
	AK47_IDLE1,
	AK47_RELOAD,
	AK47_DRAW,
	AK47_SHOOT1,
	AK47_SHOOT2,
	AK47_SHOOT3
};

LINK_ENTITY_TO_CLASS(weapon_ak47l, CAK47_Long)

const int AK47L_MAXCLIP = 60;

const char *CAK47_Long::GetCSModelName()
{
	return "models/w_ak47_long.mdl";
}

void CAK47_Long::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_ak47l");

	Precache();
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), GetCSModelName());

	m_iDefaultAmmo = AK47L_MAXCLIP;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CAK47_Long::Precache(void)
{
	PRECACHE_MODEL("models/v_ak47_long.mdl");
	PRECACHE_MODEL("models/p_ak47_long.mdl");
	PRECACHE_MODEL("models/w_ak47_long.mdl");

	PRECACHE_SOUND("weapons/ak_long-1.wav");
	PRECACHE_SOUND("weapons/ak47_clipout.wav");
	PRECACHE_SOUND("weapons/ak47_clipin.wav");
	PRECACHE_SOUND("weapons/ak47_boltpull.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireAK47 = PRECACHE_EVENT(1, "events/ak47l.sc");
}

int CAK47_Long::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = MAX_AMMO_762NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AK47L_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_AK47;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

BOOL CAK47_Long::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_ak47_long.mdl", "models/p_ak47_long.mdl", AK47_DRAW, "ak47", UseDecrement() != FALSE);
}

void CAK47_Long::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CAK47_Long::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		AK47Fire(0.04 + (0.4) * m_flAccuracy, 0.0955, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		AK47Fire(0.03 + (0.02) * m_flAccuracy, 0.0955, FALSE);
	else
		AK47Fire((0.02), 0.0955, FALSE);
}

void CAK47_Long::AK47Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;

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
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, 97, 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireAK47, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.4125, 0.15, 0.075, 0.015, 2.5, 0.5, 3);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1, 0.5, 0.25, 0.175, 4.5, 3, 1);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.3, 0.15, 0.05, 0.005, 2.5, 0.5, 4);
	else
		KickBack(0.4, 0.15, 0.0625, 0.015, 2.5, 0.5, 3);
}

void CAK47_Long::Reload(void)
{
	if (m_pPlayer->ammo_762nato <= 0)
		return;

	if (DefaultReload(AK47L_MAXCLIP, AK47_RELOAD, 2.45))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CAK47_Long::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(AK47_IDLE1, UseDecrement() != FALSE);
}
