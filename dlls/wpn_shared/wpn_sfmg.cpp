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
#include "wpn_sfmg.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sfmg, CSFMG)

const int SFMG_AMMO_GIVE = 200;

void CSFMG::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_sfmg");

	Precache();
	m_iId = WEAPON_M4A1;
	SET_MODEL(ENT(pev), "models/w_sfmg.mdl");

	m_iDefaultAmmo = SFMG_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CSFMG::Precache(void)
{
	PRECACHE_MODEL("models/v_sfmg.mdl");
	PRECACHE_MODEL("models/w_sfmg.mdl");
	PRECACHE_MODEL("models/p_sfmg.mdl");

	PRECACHE_MODEL("sprites/muzzleflash18.spr");

	PRECACHE_SOUND("weapons/sfmg-1.wav");

	m_iShell = PRECACHE_MODEL("models/shell762.mdl");
	m_usFireSFMG = PRECACHE_EVENT(1, "events/sfmg.sc");
}

int CSFMG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762NatoBox";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SFMG_AMMO_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M4A1;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CSFMG::Deploy(void)
{
	m_flAccuracy = 0.1;
	m_flAccuracy2 = 0.2;
	m_iShotsFired = 0;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return DefaultDeploy("models/v_sfmg.mdl", "models/p_sfmg.mdl", SFMG_DRAWB, "m249", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_sfmg.mdl", "models/p_sfmg.mdl", SFMG_DRAWA, "m249", UseDecrement() != FALSE);
}

void CSFMG::PrimaryAttack(void)
{
	duration_t flCycleTime;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		flCycleTime = 0.068s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SFMGFire(0.05 + (0.45) * m_flAccuracy2, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() >= 140)
			SFMGFire(0.035 + (0.085) * m_flAccuracy2, flCycleTime, FALSE);
		else
			SFMGFire(0.1 + (0.033) * m_flAccuracy2, flCycleTime, FALSE);
	}
	else
	{
		flCycleTime = 0.125s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			SFMGFire(0.035 + (0.7) * m_flAccuracy, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() >= 140)
			SFMGFire(0.04 + (0.085) * m_flAccuracy, flCycleTime, FALSE);
		else
			SFMGFire(0.04 * m_flAccuracy, flCycleTime, FALSE);
	}

	
}

void CSFMG::SFMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_flAccuracy2 = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210.0) + 0.3;

		if (m_flAccuracy2 > 0.8)
			m_flAccuracy2 = 0.8;
	}
	else
	{
		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.4;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
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

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_556MM, m_iWeaponState & WPNSTATE_M4A1_SILENCED ? GetDamageB(): GetDamageA(), m_iWeaponState & WPNSTATE_M4A1_SILENCED ? 0.96 : 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSFMG, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) ? true : false, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;


	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.2, 0.65, 0.45, 0.125, 5.0, 3.5, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.0, 5.0, 0.25, 0.065, 4.0, 3.0, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.7, 0.3, 0.225, 0.02, 3.5, 2.0, 9);
		else
			KickBack(0.95, 0.35, 0.19, 0.035, 4.0, 3.0, 9);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.6, 0.55, 0.35, 0.125, 5.0, 2.5, 8);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.5, 0.235, 0.055, 4.0, 3.0, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.7, 0.3, 0.225, 0.02, 3.5, 2.0, 9);
		else
			KickBack(0.99, 0.35, 0.153, 0.025, 4.1, 2.7, 9);
	}
}

void CSFMG::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	int iAnim;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		iAnim = SFMG_RELOADB;
	else
		iAnim = SFMG_RELOADA;

	if (DefaultReload(200, iAnim, 4.7s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.1;
		m_flAccuracy2 = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}


void CSFMG::SecondaryAttack(void)
{
	m_flLastFire = invalid_time_point;
	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SFMG_CHANGEB, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "m249");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		SendWeaponAnim(SFMG_CHANGEA, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "m249");
	}
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.7s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.7s;
}


void CSFMG::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		SendWeaponAnim(SFMG_IDLEB, UseDecrement() != FALSE);
	else
		SendWeaponAnim(SFMG_IDLEA, UseDecrement() != FALSE);
}

float CSFMG::GetDamageA() const
{
	float flDamage = 37.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 49.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 75.0f;
#endif
	return flDamage;
}

float CSFMG::GetDamageB() const
{
	float flDamage = 33.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 45.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}

float CSFMG::GetMaxSpeed(void)
{
	return 225;
}

}
