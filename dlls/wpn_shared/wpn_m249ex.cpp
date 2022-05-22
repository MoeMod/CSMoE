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
#include "wpn_m249ex.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_m249ex, CM249EX)

const int M249EX_AMMO_GIVE = 120;

void CM249EX::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m249ex");

	Precache();
	m_iId = WEAPON_M249EX;
	SET_MODEL(ENT(pev), "models/w_m249ex.mdl");

	m_iDefaultAmmo = M249EX_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CM249EX::Precache(void)
{
	PRECACHE_MODEL("models/v_m249ex.mdl");
	PRECACHE_MODEL("models/w_m249ex.mdl");
	PRECACHE_MODEL("models/p_m249ex.mdl");

	PRECACHE_SOUND("weapons/m249ex-1.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireM249EX = PRECACHE_EVENT(1, "events/m249ex.sc");
}

int CM249EX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556az";
	p->iMaxAmmo1 = 240;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M249EX_AMMO_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249EX;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CM249EX::Deploy(void)
{
	m_flAccuracy = 0.25;
	m_iShotsFired = 0;

	return DefaultDeploy("models/v_m249ex.mdl", "models/p_m249ex.mdl", M249EX_DRAW, "m249", UseDecrement() != FALSE);
}

void CM249EX::PrimaryAttack(void)
{
	duration_t flCycleTime;

	if (m_pPlayer->pev->fov == 90)
		flCycleTime = 0.125s;
	else
		flCycleTime = 0.15s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		M249EXFire(0.05 + (0.2) * m_flAccuracy, flCycleTime, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 195)
		M249EXFire(0.04 + (0.03) * m_flAccuracy, flCycleTime, FALSE);
	else if (m_pPlayer->pev->fov != 90)
		M249EXFire(0.02 + (0.01) * m_flAccuracy, flCycleTime, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		M249EXFire(0.01 + (0.02) * m_flAccuracy, flCycleTime, FALSE);
	else
		M249EXFire(0.03 + (0.0) * m_flAccuracy, flCycleTime, FALSE);
}

void CM249EX::M249EXFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.3;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

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
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 7, BULLET_PLAYER_556MM, GetDamage(), m_pPlayer->m_iFOV == 90 ? 0.97 : 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM249EX, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.5, 0.55, 0.3, 0.3, 5.0, 5.0, 5);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.1, 0.3, 0.2, 0.06, 4.0, 3.0, 8);
	else if (m_pPlayer->pev->fov != 90)
		KickBack(0.8, 0.2, 0.08, 0.0, 2.0, 0.2, 10);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.75, 0.1, 0.1, 0.018, 3.5, 1.8, 9);
	else
		KickBack(0.85, 0.2, 0.2, 0.02, 3.2, 2.5, 7);
}

void CM249EX::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(120, M249EX_RELOAD, 3.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0.25;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}


void CM249EX::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV == 90)
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 65;
	}
	else
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}

	m_pPlayer->ResetMaxSpeed();

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4, 0, PITCH_NORM);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}


void CM249EX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
	SendWeaponAnim(M249EX_IDLE1, UseDecrement() != FALSE);
}

float CM249EX::GetDamage() const
{
	float flDamage = 33.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 44.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 55.0f;
#endif
	return flDamage;
}

float CM249EX::GetMaxSpeed(void)
{
	if (m_pPlayer->pev->fov != 90)
	{
		return 200;
	}

	return 220;
}

}
