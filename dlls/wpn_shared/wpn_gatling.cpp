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
#include "wpn_gatling.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_gatling, CGatling)

void CGatling::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_gatling.mdl");

	m_iDefaultAmmo = GATLING_DEFAULT_GIVE;

	FallInit();
}

void CGatling::Precache(void)
{
	PRECACHE_MODEL("models/v_gatling.mdl");
	PRECACHE_MODEL("models/w_gatling.mdl");
	PRECACHE_MODEL("models/p_gatling.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/gatling-1.wav");
	PRECACHE_SOUND("weapons/gatling_clipin1.wav");
	PRECACHE_SOUND("weapons/gatling_clipin2.wav");
	PRECACHE_SOUND("weapons/gatling_clipout1.wav");
	PRECACHE_SOUND("weapons/gatling_clipout2.wav");
	PRECACHE_SOUND("weapons/gatling_boltpull.wav");
	PRECACHE_SOUND("weapons/usas_draw.wav");

	m_usFireGatling = PRECACHE_EVENT(1, "events/gatling.sc");
}

int CGatling::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "12gauge";
	p->iMaxAmmo1 = 80;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = GATLING_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CGatling::Deploy(void)
{
	return DefaultDeploy("models/v_gatling.mdl", "models/p_gatling.mdl", GATLING_DRAW, "m249", UseDecrement() != FALSE);
}

void CGatling::PrimaryAttack(void)
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
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(7, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.03, 0.03, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetPrimaryAttackDamage());
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireGatling, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);



#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.23s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.23s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
	else
		m_flTimeWeaponIdle = 0.75s;


	if (m_pPlayer->pev->velocity.Length2D() > 0)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 1, 3.0, 4.0);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 5.0, 6.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 1.5, 2.0);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2.0, 3.0);
}

void CGatling::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(GATLING_MAX_CLIP, GATLING_RELOAD, 4.55s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}

void CGatling::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(GATLING_IDLE, UseDecrement() != FALSE);
	
}

int CGatling::GetPrimaryAttackDamage() const
{
	int iDamage = 18;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		iDamage = 26;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		iDamage = 26;
#endif
	return iDamage;
}

}
