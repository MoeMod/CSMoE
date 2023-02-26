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
#include "wpn_dartpistol.h"


#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_dartpistol, CDartPistol)

void CDartPistol::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_dartpistol");

	Precache();
	m_iId = WEAPON_DARTPISTOL;
	SET_MODEL(ENT(pev), "models/w_dartpistol.mdl");

	m_iDefaultAmmo = 5;
	FallInit();
}

void CDartPistol::Precache(void)
{
	m_usFireDartPistol = PRECACHE_EVENT(1, "events/dartpistol.sc");

	PRECACHE_MODEL("sprites/muzzleflash12.spr");

	PRECACHE_SOUND("weapons/dartpistol-1.wav");

	PRECACHE_MODEL("models/p_dartpistol.mdl");
	PRECACHE_MODEL("models/v_dartpistol.mdl");
	PRECACHE_MODEL("models/w_dartpistol.mdl");
}

int CDartPistol::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "dart";
	p->iMaxAmmo1 = 35;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 7;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_DARTPISTOL;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CDartPistol::Deploy(void)
{
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_dartpistol.mdl", "models/p_dartpistol.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CDartPistol::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		DartPistolFire((1.2) * (1 - m_flAccuracy), 1.0s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		DartPistolFire((0.25) * (1 - m_flAccuracy), 1.0s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		DartPistolFire((0.1) * (1 - m_flAccuracy), 1.0s, FALSE);
	else
		DartPistolFire((0.15) * (1 - m_flAccuracy), 1.0s, FALSE);

}

void CDartPistol::DartPistolFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

		if (m_flAccuracy > 0.95)
			m_flAccuracy = 0.95;
		else if (m_flAccuracy < 0.6)
			m_flAccuracy = 0.6;
	}

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;
	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
		m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up, gpGlobals->v_forward, flSpread, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
	Vector vecDartDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
	Vector vecAngles = UTIL_VecToAngles(vecDartDir);

	float flSpeed;
	if (m_pPlayer->pev->waterlevel == 3)
		flSpeed = 1800.0;
	else
		flSpeed = 2000.0;
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDartPistol, 0.0, g_vecZero, g_vecZero, 0.0, 0.0, m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType], m_iClip != 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif

	m_pPlayer->pev->punchangle.x -= 2;

}

void CDartPistol::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(7, ANIM_RELOAD, 4s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.95;
	}
}

void CDartPistol::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

}
