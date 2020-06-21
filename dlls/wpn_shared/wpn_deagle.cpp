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
#include "wpn_deagle.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum deagle_e
{
	DEAGLE_IDLE1,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_SHOOT_EMPTY,
	DEAGLE_RELOAD,
	DEAGLE_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_deagle, CDEAGLE)

void CDEAGLE::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_deagle");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/w_deagle.mdl");

	m_iDefaultAmmo = DEAGLE_DEFAULT_GIVE;
	m_flAccuracy = 0.9;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_fMaxSpeed = 250;

	FallInit();
}

void CDEAGLE::Precache(void)
{
	PRECACHE_MODEL("models/v_deagle.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_deagle.mdl");
#endif
	PRECACHE_MODEL("models/w_deagle.mdl");

	PRECACHE_SOUND("weapons/deagle-1.wav");
	PRECACHE_SOUND("weapons/deagle-2.wav");
	PRECACHE_SOUND("weapons/de_clipout.wav");
	PRECACHE_SOUND("weapons/de_clipin.wav");
	PRECACHE_SOUND("weapons/de_deploy.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireDeagle = PRECACHE_EVENT(1, "events/deagle.sc");
}

int CDEAGLE::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = MAX_AMMO_50AE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DEAGLE_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

BOOL CDEAGLE::Deploy(void)
{
	m_flAccuracy = 0.9;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
	m_fMaxSpeed = 250;
	m_NextInspect = gpGlobals->time + 0.75s;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_deagle.mdl", "models/shield/p_shield_deagle.mdl", DEAGLE_DRAW, "shieldgun", UseDecrement() != FALSE);
	else
#endif
	{
		return DefaultDeploy("models/v_deagle.mdl", "models/p_deagle.mdl", DEAGLE_DRAW, "onehanded", UseDecrement() != FALSE);
	}
}

void CDEAGLE::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		DEAGLEFire((1.5) * (1 - m_flAccuracy), 0.3s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		DEAGLEFire((0.25) * (1 - m_flAccuracy), 0.3s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		DEAGLEFire((0.115) * (1 - m_flAccuracy), 0.3s, FALSE);
	else
		DEAGLEFire((0.13) * (1 - m_flAccuracy), 0.3s, FALSE);
}

void CDEAGLE::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_UP, SHIELDGUN_DOWN);
}

void CDEAGLE::DEAGLEFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
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

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	SetPlayerShieldAnim();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(), 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireDeagle, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

void CDEAGLE::Reload(void)
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;
	m_NextInspect = gpGlobals->time + DEAGLE_RELOAD_TIME;
	if (DefaultReload(DEAGLE_MAX_CLIP, DEAGLE_RELOAD, 2.2s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;
	}
}

void CDEAGLE::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		SendWeaponAnim(SHIELDGUN_DRAWN_IDLE, UseDecrement() != FALSE);
}

float CDEAGLE::GetDamage() const
{
	float flDamage = 56.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 70.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}

void CDEAGLE::Inspect()
{

	if (!m_fInReload)
	{
		if (m_flLastFire != invalid_time_point || gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(RANDOM_LONG(6, 7), 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
			m_flLastFire = invalid_time_point;
		}
	}

}
}
