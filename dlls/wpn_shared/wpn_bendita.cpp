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
#include "wpn_bendita.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_bendita, CBendita)

const int BENDITA_MAXCLIP = 7;

void CBendita::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_bendita");

	Precache();
	m_iId = WEAPON_SG550;
	SET_MODEL(ENT(pev), "models/w_bendita.mdl");

	m_iDefaultAmmo = BENDITA_MAXCLIP;
	FallInit();
}

void CBendita::Precache(void)
{
	PRECACHE_MODEL("models/v_bendita.mdl");
	PRECACHE_MODEL("models/p_bendita.mdl");
	PRECACHE_MODEL("models/w_bendita.mdl");
	PRECACHE_MODEL("sprites/holybomb_burn.spr");


	PRECACHE_SOUND("weapons/bendita-1.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireBendita = PRECACHE_EVENT(1, "events/bendita.sc");
}

int CBendita::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50bmgholy";
	p->iMaxAmmo1 = 70;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BENDITA_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_SG550;
	p->iFlags = 0;
	p->iWeight = SG550_WEIGHT;

	return 1;
}

BOOL CBendita::Deploy(void)
{
	m_flAccuracy = 0.9;

	return DefaultDeploy("models/v_bendita.mdl", "models/p_bendita.mdl", BENDITA_DRAW, "rifle", UseDecrement() != FALSE);
}

void CBendita::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = BENDITA_ZOOM_FOV_1; break;
	case BENDITA_ZOOM_FOV_1: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = BENDITA_ZOOM_FOV_2; break;
	default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	if(m_pPlayer->m_iFOV  != 90)
		m_flAccuracy = 1.0;

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CBendita::PrimaryAttack(void)
{
	if (m_pPlayer->pev->fov != 90)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			BenditaFire((0.7) * (1 - m_flAccuracy), 0.45s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			BenditaFire((0.3) * (1 - m_flAccuracy), 0.45s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			BenditaFire((0.03) * (1 - m_flAccuracy), 0.45s, FALSE);
		else
			BenditaFire((0.15) * (1 - m_flAccuracy), 0.45s, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			BenditaFire((0.7) * (1 - m_flAccuracy), 0.4s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			BenditaFire((0.3) * (1 - m_flAccuracy), 0.4s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			BenditaFire((0.03) * (1 - m_flAccuracy), 0.4s, FALSE);
		else
			BenditaFire((0.15) * (1 - m_flAccuracy), 0.4s, FALSE);
	}
}

void CBendita::BenditaFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov == 90)
		flSpread += 0.1;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.375 + 0.6;

		if (m_flAccuracy > 1.0)
			m_flAccuracy = 1.0;
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
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55s;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
#ifndef CLIENT_DLL
	int iWeapontype = 4;
	Vector vecDir = m_pPlayer->FireBullets4(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_338MAG, GetDamage(), BENDITA_RANGE_MODIFER, m_pPlayer->pev, TRUE, iWeapontype, m_pPlayer->random_seed);
#else
	Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_338MAG, GetDamage(), BENDITA_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireBendita, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 10.0) + m_pPlayer->pev->punchangle.x * 0.75;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 2.0);
	}
	else
	{
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 9.0) + m_pPlayer->pev->punchangle.x * 0.75;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
	}
}

void CBendita::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(BENDITA_MAXCLIP, BENDITA_RELOAD, BENDITA_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = BENDITA_ZOOM_FOV_2;
			SecondaryAttack();
		}
	}
}

void CBendita::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(BENDITA_IDLE, UseDecrement() != FALSE);
	}
}

float CBendita::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return BENDITA_MAX_SPEED;

	return BENDITA_MAX_SPEED_ZOOM;
}

float CBendita::GetDamage() const
{
	float flDamage = 110.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage =  340.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage =  700.0f;
#endif
	return flDamage;
}
}
