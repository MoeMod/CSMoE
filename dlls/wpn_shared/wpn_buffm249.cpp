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
#include "wpn_buffm249.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_buffm249, CBUFFM249)

const int BUFFM249_AMMO_GIVE = 100;

void CBUFFM249::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_buffm249");

	Precache();
	m_iId = WEAPON_M249;
	SET_MODEL(ENT(pev), "models/w_buffm249.mdl");
	m_tNextPlayEvent = invalid_time_point;
	m_iDefaultAmmo = BUFFM249_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CBUFFM249::ItemPostFrame()
{
	CBasePlayerWeapon::ItemPostFrame();
	//if (m_fInReload)
		//m_tNextPlayEvent = invalid_time_point;
#if 0
	if (gpGlobals->time > m_tNextPlayEvent && m_tNextPlayEvent != invalid_time_point)
	{
		PLAYBACK_EVENT_FULL(1, m_pPlayer->edict(), PRECACHE_EVENT(1, "events/wpneffects.sc"), 0, (float*)&g_vecZero, (float*)&g_vecZero, 0.0, 0.0, 9, 0, FALSE, FALSE);
		m_tNextPlayEvent = gpGlobals->time + 0.1s;
	}
#endif
}

void CBUFFM249::Precache(void)
{
	PRECACHE_MODEL("models/v_buffm249.mdl");
	PRECACHE_MODEL("models/w_buffm249.mdl");
	PRECACHE_MODEL("models/p_buffm249.mdl");
	PRECACHE_MODEL("sprites/muzzleflash43.spr");
	PRECACHE_MODEL("sprites/buffm249_fire01.spr");
	PRECACHE_MODEL("sprites/buffm249_fire02.spr");
	PRECACHE_MODEL("sprites/buffm249_fire03.spr");
	PRECACHE_MODEL("sprites/buffm249_fire04.spr");
	PRECACHE_MODEL("sprites/buffm249_fire05.spr");

	PRECACHE_SOUND("weapons/buffm249-1.wav");
	PRECACHE_SOUND("weapons/buffm249-2.wav");
	//PRECACHE_SOUND("weapons/buffm249_scream.wav");	unknown
	//PRECACHE_SOUND("weapons/buffm249_loop.wav");	idle

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireBUFFM249 = PRECACHE_EVENT(1, "events/buffm249.sc");
}

int CBUFFM249::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50az";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BUFFM249_AMMO_GIVE;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CBUFFM249::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_tNextPlayEvent = gpGlobals->time;

	return DefaultDeploy("models/v_buffm249.mdl", "models/p_buffm249.mdl", BUFFM249_DRAW, "m249", UseDecrement() != FALSE);
}

void CBUFFM249::PrimaryAttack(void)
{
	duration_t flCycleTime;

	if (m_pPlayer->pev->fov == 90)
		flCycleTime = 0.1s;
	else
		flCycleTime = 0.5s;

	if (m_pPlayer->pev->fov != 90)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			BUFFM249Fire(0.05 + (0.2) * m_flAccuracy, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 195)
			BUFFM249Fire(0.04 + (0.03) * m_flAccuracy, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			BUFFM249Fire(0.01 + (0.02) * m_flAccuracy, flCycleTime, FALSE);
		else
			BUFFM249Fire(0.03 + (0.0) * m_flAccuracy, flCycleTime, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			BUFFM249Fire(0.04 + 0.4 * m_flAccuracy, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 195)
			BUFFM249Fire(0.025 + 0.07 * m_flAccuracy, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			BUFFM249Fire(0.0 + 0.0 * m_flAccuracy, flCycleTime, FALSE);
		else
			BUFFM249Fire(0.02 * m_flAccuracy, flCycleTime, FALSE);
	}
}

void CBUFFM249::BUFFM249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
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
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, m_pPlayer->pev->fov != 90 ? 4 : 2, BULLET_PLAYER_556MM, m_pPlayer->pev->fov != 90 ? GetDamageB() : GetDamageA(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireBUFFM249, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->pev->fov != 90 ? TRUE : FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (m_pPlayer->pev->fov != 90)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.5, 0.55, 0.3, 0.3, 3.75, 1.5, 5);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.1, 0.3, 0.2, 0.06, 3.75, 1.5, 8);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.75, 0.1, 0.1, 0.018, 3.75, 1.5, 9);
		else
			KickBack(0.85, 0.2, 0.2, 0.02, 3.75, 1.5, 7);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.1, 0.47, 0.27, 0.135, 3.0, 1.5, 5);
		else
		{
			if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.4, 0.2, 0.15, 0.01, 3.0, 1.5, 5);
			else
				KickBack(0.5, 0.3, 0.2, 0.01, 3.0, 1.5, 5);
		}
	}
}

void CBUFFM249::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(100, BUFFM249_RELOAD, 3.6s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}


void CBUFFM249::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV == 90)
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 80;
	}
	else
	{
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}

	m_pPlayer->ResetMaxSpeed();

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}


void CBUFFM249::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
	SendWeaponAnim(BUFFM249_IDLE1, UseDecrement() != FALSE);
}

float CBUFFM249::GetDamageA() const
{
	float flDamage = 37.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 170.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 550.0f;
#endif
	return flDamage;
}

float CBUFFM249::GetDamageB() const
{
	float flDamage = 38.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 500.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 1070.0f;
#endif
	return flDamage;
}

KnockbackData CBUFFM249::GetKnockBackData()
{
	if (m_pPlayer->pev->fov != 90)
		return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };
	else
		return { 400.0f, 300.0f, 350.0f, 150.0f, 0.5f };
}

float CBUFFM249::GetMaxSpeed(void)
{
	return 220;
}

}
