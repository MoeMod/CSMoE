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
#include "wpn_sfgun.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sfgun, CSFGUN)

void CSFGUN::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sfgun");

	Precache();
	m_iId = WEAPON_SFGUN;
	SET_MODEL(ENT(pev), "models/w_sfgun.mdl");

	m_iDefaultAmmo = SFGUN_MAX_CLIP;

	FallInit();
}

void CSFGUN::Precache(void)
{
	PRECACHE_MODEL("models/v_sfgun.mdl");
	PRECACHE_MODEL("models/w_sfgun.mdl");
	PRECACHE_MODEL("models/p_sfgun.mdl");

	PRECACHE_MODEL("sprites/muzzleflash16.spr");

	PRECACHE_SOUND("weapons/sfgun-1.wav");

	m_iShellId = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireSFGUN = PRECACHE_EVENT(1, "events/sfgun.sc");
}

int CSFGUN::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SFGUN_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 17;
	p->iId = m_iId = WEAPON_SFGUN;
	p->iFlags = 0;
	p->iWeight = GALIL_WEIGHT;

	return 1;
}

BOOL CSFGUN::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	return DefaultDeploy("models/v_sfgun.mdl", "models/p_sfgun.mdl", SFGUN_DRAW, "ak47", UseDecrement() != FALSE);
}

void CSFGUN::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		SFGUNFire(0.04 + (0.4) * m_flAccuracy, 0.073s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		SFGUNFire(0.04 + (0.07) * m_flAccuracy, 0.073s, FALSE);
	else
		SFGUNFire((0.0275) * m_flAccuracy, 0.073s, FALSE);
}

void CSFGUN::SFGUNFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.3;

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

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSFGUN, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.52, 0.2, 0.22, 0.015, 2.7, 2.0, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(2.0, 1.0, 0.4, 0.35, 9.0, 6.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.48, 0.16, 0.2, 0.011, 2.0, 1.65, 9);
	else
		KickBack(0.49, 0.18, 0.2, 0.013, 2.4, 1.74, 8);
}

void CSFGUN::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(SFGUN_MAX_CLIP, SFGUN_RELOAD, SFGUN_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CSFGUN::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(SFGUN_IDLE1, UseDecrement() != FALSE);
}

float CSFGUN::GetDamage() const
{
	float flDamage = 30.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 60.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 88.0f;
#endif
	return flDamage;
}

}
