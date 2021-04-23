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
#include "wpn_mg3.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_mg3, CMG3)

static const int MG3_AMMO_GIVE = 200;

void CMG3::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_mg3");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_mg3.mdl");

	m_iDefaultAmmo = MG3_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CMG3::Precache(void)
{
	PRECACHE_MODEL("models/v_mg3.mdl");
	PRECACHE_MODEL("models/w_mg3.mdl");
	PRECACHE_MODEL("models/p_mg3.mdl");
	PRECACHE_SOUND("weapons/mg3-1.wav");
	PRECACHE_SOUND("weapons/mg3-2.wav");
	PRECACHE_SOUND("weapons/mg3_clipout.wav");
	PRECACHE_SOUND("weapons/mg3_clipin.wav");
	PRECACHE_SOUND("weapons/mg3_cliplock.wav");
	PRECACHE_SOUND("weapons/mg3_open.wav");
	PRECACHE_SOUND("weapons/mg3_close.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireMG3 = PRECACHE_EVENT(1, "events/mg3.sc");
}

int CMG3::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MG3_AMMO_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CMG3::Deploy(void)
{
	iShellOn = 1;

	return DefaultDeploy("models/v_mg3.mdl", "models/p_mg3.mdl", M249_DRAW, "m249", UseDecrement() != FALSE);
}

void CMG3::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		MG3Fire(0.045 + (0.5) * m_flAccuracy, 0.095s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		MG3Fire(0.045 + (0.095) * m_flAccuracy, 0.095s, FALSE);
	else
		MG3Fire((0.03) * m_flAccuracy, 0.095s, FALSE);
}

void CMG3::MG3Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 195.0) + 0.4;

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
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireMG3, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.85, 0.65, 0.45, 0.125, 5.0, 3.0, 1);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.3125, 0.15, 0.075, 0.06, 4.0, 0.5, 3);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.3, 0.125, 0.25, 0.015, 2.5, 0.5, 4);
	else
		KickBack(0.4, 0.15, 0.525, 0.015, 2.5, 0.5, 3);
}

void CMG3::Reload(void)
{
	if (m_pPlayer->ammo_556natobox <= 0)
		return;

	if (DefaultReload( 400, M249_RELOAD, 4.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void CMG3::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
	SendWeaponAnim(M249_IDLE1, UseDecrement() != FALSE);
}

float CMG3::GetDamage() const
{
	float flDamage = 33.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 35.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 35.0f;
#endif
	return flDamage;
}
}
