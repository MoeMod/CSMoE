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
#include "wpn_waterpistol.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#ifdef CLIENT_DLL
extern bool g_bGlockBurstMode;
#endif

LINK_ENTITY_TO_CLASS(weapon_waterpistol, CWaterPistol)

#define WATERPISTOL_MAX_CLIP	40
#define WATERPISTOL_MAX_SPEED		250
#define WATERPISTOL_RANGE_MODIFER		0.75
#define WATERPISTOL_RELOAD_TIME		2.2s

void CWaterPistol::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_waterpistol");

	Precache();
	m_iId = WEAPON_WATERPISTOL;
	SET_MODEL(ENT(pev), "models/w_waterpistol.mdl");

	m_bBurstFire = 0;
	m_iGlock18ShotsFired = 0;
	m_flGlock18Shoot = invalid_time_point;
	m_iDefaultAmmo = 40;
	m_flAccuracy = 0.9;
	m_iWeaponState = 0;

	FallInit();
}

void CWaterPistol::Precache(void)
{
	PRECACHE_MODEL("models/p_waterpistol.mdl");
	PRECACHE_MODEL("models/v_waterpistol.mdl");
	PRECACHE_MODEL("models/w_waterpistol.mdl");

	PRECACHE_MODEL("sprites/muzzleflash15.spr");

	PRECACHE_SOUND("weapons/waterpistol-1.wav");

	m_usFireWaterPistol = PRECACHE_EVENT(1, "events/waterpistol.sc");
}

int CWaterPistol::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "waterpistol";
	p->iMaxAmmo1 = MAX_AMMO_9MM;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WATERPISTOL_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_WATERPISTOL;
	p->iFlags = 0;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CWaterPistol::Deploy(void)
{
	m_bBurstFire = 0;
	m_iGlock18ShotsFired = 0;
	m_flGlock18Shoot = invalid_time_point;
	m_flAccuracy = 0.9;
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_waterpistol.mdl", "models/p_waterpistol.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
	
}

void CWaterPistol::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		WaterPistolFire((1.2) * (1 - m_flAccuracy), 0.8s);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		WaterPistolFire((0.185) * (1 - m_flAccuracy), 0.8s);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		WaterPistolFire((0.095) * (1 - m_flAccuracy), 0.8s);
	else
		WaterPistolFire((0.15) * (1 - m_flAccuracy), 0.8s);
}


void CWaterPistol::FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
{
	m_iClip--;
	shotsFired++;

	if (shotsFired != 5)
	{
		shootTime = gpGlobals->time + 0.015s;
	}
	else
		shootTime = invalid_time_point;

	if (m_iClip < 0)
	{
		m_iClip = 0;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamageB(), 0.9, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireWaterPistol, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

}

void CWaterPistol::WaterPistolFire(float flSpread, duration_t flCycleTime)
{

	m_iGlock18ShotsFired = 0;
	
	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.185 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.2;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.5)
			m_flAccuracy = 0.5;
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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamageA(), 0.75, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireWaterPistol, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;


	m_iGlock18ShotsFired++;
	m_flGlock18Shoot = gpGlobals->time + 0.33s;
	

	ResetPlayerShieldAnim();
}

void CWaterPistol::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(WATERPISTOL_MAX_CLIP, ANIM_RELOAD1, WATERPISTOL_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;
	}
}

void CWaterPistol::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

float CWaterPistol::GetDamageA()
{
	float flDamage = 22;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 26;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 50;
#endif
	return flDamage;
}

float CWaterPistol::GetDamageB()
{
	float flDamage = 15;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 26;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 50;
#endif
	return flDamage;
}
}
