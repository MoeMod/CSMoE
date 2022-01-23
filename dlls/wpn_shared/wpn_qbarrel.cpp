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
#include "wpn_qbarrel.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_qbarrel, CQbarrel)

void CQbarrel::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_qbarrel.mdl");

	m_iDefaultAmmo = DEFAULT_GIVE;

	FallInit();
}

void CQbarrel::Precache(void)
{
	PRECACHE_MODEL("models/v_qbarrel.mdl");
	PRECACHE_MODEL("models/w_qbarrel.mdl");
	PRECACHE_MODEL("models/p_qbarrel.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/qbarrel-1.wav");

	m_usFireQbarrel = PRECACHE_EVENT(1, "events/qbarrel.sc");
}

int CQbarrel::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = 32;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CQbarrel::Deploy(void)
{
	m_iInstantShots = 1;
	return DefaultDeploy("models/v_qbarrel.mdl", "models/p_qbarrel.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
}

void CQbarrel::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_iInstantShots = 1;
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(6 * m_iClip, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.03, 0.03, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireQbarrel, 0, g_vecZero, g_vecZero, 0.0527, 0.0, m_iInstantShots, 7, FALSE, FALSE);

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.8s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 7);
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5, 5);
	}
}

void CQbarrel::SecondaryAttack(void)
{
	if (m_iClip == 1)
	{
		PrimaryAttack();

		return;
	}

	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	m_iInstantShots = m_iClip;
	m_iClip = 0;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	/*if (m_iInstantShots > 0 && m_iInstantShots < 5)
	{*/
		float flSpread = GetSpread();

#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(7 * m_iInstantShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0.0), 4800, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireQbarrel, 0.0, g_vecZero, g_vecZero, GetSpread(), 0.0, m_iInstantShots, 7 * m_iInstantShots, FALSE, FALSE);
	//}

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.8s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 7);
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5, 5);
	}
}

void CQbarrel::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(MAX_CLIP, ANIM_RELOAD, 2.23s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CQbarrel::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	
}

int CQbarrel::GetDamage()
{
	int iDamage[5] = { 0, 66, 66, 86, 100 };
	if (m_iInstantShots > 4)
		return iDamage[4] + (m_iInstantShots - 4) * 14;
	return iDamage[m_iInstantShots];
}

float CQbarrel::GetSpread()
{
	float flSpread[5] = { 0.0, 0.0527, 0.0625, 0.085, 0.125 };
	return flSpread[min(m_iInstantShots, 4)];
}

}
