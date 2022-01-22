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
#include "wpn_skull5.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_skull5, CSkull5)

void CSkull5::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_skull5");

	Precache();
	m_iId = WEAPON_G3SG1;
	SET_MODEL(ENT(pev), "models/w_skull5.mdl");
	m_iDefaultAmmo = SKULL5_MAX_CLIP;

	FallInit();
}

void CSkull5::Precache(void)
{
	PRECACHE_MODEL("models/v_skull5.mdl");
	PRECACHE_MODEL("models/w_skull5.mdl");
	PRECACHE_MODEL("models/p_skull5.mdl");

	PRECACHE_SOUND("weapons/skull5-1.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireSkull5 = PRECACHE_EVENT(1, "events/skull5.sc");
}

int CSkull5::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556az";
	p->iMaxAmmo1 = 240;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SKULL5_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SKULL5;
	p->iFlags = 0;
	p->iWeight = G3SG1_WEIGHT;

	return 1;
}

BOOL CSkull5::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif
	m_flAccuracy = 0.2;
	return DefaultDeploy("models/v_skull5.mdl", "models/p_skull5.mdl", SKULL5_DRAW, "mp5", UseDecrement() != FALSE);
}

void CSkull5::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 80; break;	
		case 80: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 35; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CSkull5::PrimaryAttack(void)
{
	duration_t flCycleTime = m_pPlayer->m_iFOV == 90 ? 0.28s : 0.33s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Skull5Fire(1.0, flCycleTime, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Skull5Fire(0.06, flCycleTime, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Skull5Fire(0.02, flCycleTime, FALSE);
	else
		Skull5Fire(0.04, flCycleTime, FALSE);
}

void CSkull5::Skull5Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.5 + 0.6;

		if (m_flAccuracy > 1)
			m_flAccuracy = 1;
	}
	else
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

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_762MM, GetDamage(), 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSkull5, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), TRUE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	m_flLastFire = gpGlobals->time;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	if (m_pPlayer->m_iFOV == 90)
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 3.0, 4.0) + m_pPlayer->pev->punchangle.x * 0.25;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -2.0, 2.0);
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 2.5, 4.0) + m_pPlayer->pev->punchangle.x * 0.25;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -1.4, 1.4);
	}
}

void CSkull5::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(SKULL5_MAX_CLIP, SKULL5_RELOAD, SKULL5_RELOAD_TIME))
	{
		m_flAccuracy = 0.2;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 15;
			SecondaryAttack();
		}
	}
}

void CSkull5::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(SKULL5_IDLE, UseDecrement() != FALSE);
	}
}

float CSkull5::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 210;

	return 150;
}

float CSkull5::GetDamage() const
{
	float flDamage = 75.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 92.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 150.0f;
#endif
	return flDamage;
}

}
