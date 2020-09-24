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
#include "wpn_aug.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum aug_e
{
	AUG_IDLE1,
	AUG_RELOAD,
	AUG_DRAW,
	AUG_SHOOT1,
	AUG_SHOOT2,
	AUG_SHOOT3,
	AUG_INSPECT,
	AUG_AIM,
	AUG_BACK
};

LINK_ENTITY_TO_CLASS(weapon_aug, CAUG)

void CAUG::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_aug");
	Precache();
	m_iId = WEAPON_AUG;
	SET_MODEL(ENT(pev), "models/w_aug.mdl");

	m_iDefaultAmmo = AUG_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CAUG::Precache(void)
{
	PRECACHE_MODEL("models/v_aug.mdl");
	PRECACHE_MODEL("models/w_aug.mdl");
	PRECACHE_MODEL("models/w_aug.mdl");
#if SPECIALSCOPE
	PRECACHE_MODEL("models/v_augscope.mdl");
#endif
	PRECACHE_SOUND("weapons/aug-1.wav");
	PRECACHE_SOUND("weapons/aug_clipout.wav");
	PRECACHE_SOUND("weapons/aug_clipin.wav");
	PRECACHE_SOUND("weapons/aug_boltpull.wav");
	PRECACHE_SOUND("weapons/aug_boltslap.wav");
	PRECACHE_SOUND("weapons/aug_forearm.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireAug = PRECACHE_EVENT(1, "events/aug.sc");
}

int CAUG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = AUG_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 14;
	p->iId = m_iId = WEAPON_AUG;
	p->iFlags = 0;
	p->iWeight = AUG_WEIGHT;

	return 1;
}

BOOL CAUG::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy("models/v_aug.mdl", "models/p_aug.mdl", AUG_DRAW, "carbine", UseDecrement() != FALSE);
}

void CAUG::SecondaryAttack(void)
{
#if SPECIALSCOPE
	if (m_pPlayer->m_iFOV != 90)
	{
		SpecialScope();
		SendWeaponAnim(AUG_BACK, 0);
		m_NextInspect = gpGlobals->time + 0.16s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.16s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
	}
	else
	{
		SendWeaponAnim(AUG_AIM, 0);
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.16s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		SetThink(&CAUG::SpecialScope);
		pev->nextthink = gpGlobals->time + 0.15s;	
	}
#else
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
#endif
}
#if SPECIALSCOPE
void CAUG::SpecialScope()
{
	if (m_pPlayer->m_iFOV != 90)
	{
#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(TRUE);
#endif
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_aug.mdl");
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(FALSE);
#endif
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_augscope.mdl");
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;
	}
}
#endif
void CAUG::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		AUGFire(0.035 + (0.4) * m_flAccuracy, 0.0825s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		AUGFire(0.035 + (0.07) * m_flAccuracy, 0.0825s, FALSE);
	else if (m_pPlayer->pev->fov == 90)
		AUGFire((0.02) * m_flAccuracy, 0.0825s, FALSE);
	else
		AUGFire((0.02) * m_flAccuracy, 0.135s, FALSE);
}

void CAUG::AUGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215.0) + 0.3;

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
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireAug, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;
	m_NextInspect = gpGlobals->time;
	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.0, 0.45, 0.275, 0.05, 4.0, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.25, 0.45, 0.22, 0.18, 5.5, 4.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.575, 0.325, 0.2, 0.011, 3.25, 2.0, 8);
	else
		KickBack(0.625, 0.375, 0.25, 0.0125, 3.5, 2.25, 8);
}

void CAUG::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;
	m_NextInspect = gpGlobals->time + AUG_RELOAD_TIME;
#if SPECIALSCOPE
	if (m_pPlayer->m_iFOV != 90)
	{
#ifndef CLIENT_DLL
		m_pPlayer->UpdateShieldCrosshair(TRUE);
#endif
		m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_aug.mdl");
	}
	if (DefaultReload(AUG_MAX_CLIP, AUG_RELOAD, 3.3s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
#else
	if (DefaultReload(AUG_MAX_CLIP, AUG_RELOAD, 3.3s))
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
#endif
}

void CAUG::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(AUG_IDLE1, UseDecrement() != FALSE);

}

float CAUG::GetDamage() const
{
	float flDamage = 32.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 70.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}

void CAUG::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect && m_pPlayer->m_iFOV == 90)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(AUG_INSPECT, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
