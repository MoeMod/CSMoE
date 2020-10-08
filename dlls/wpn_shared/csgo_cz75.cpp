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
#include "csgo_cz75.h"
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum cz75_e
{
	CZ75_IDLE_1,
	CZ75_IDLE_2,
	CZ75_DRAW_1,
	CZ75_DRAW_2,
	CZ75_SHOOT_1,
	CZ75_SHOOT_2,
	CZ75_SHOOT_EMPTY_1,
	CZ75_SHOOT_EMPTY_2,
	CZ75_RELOAD_1,
	CZ75_RELOAD_2,
	CZ75_INSPECT_1,
	CZ75_INSPECT_2
};

LINK_ENTITY_TO_CLASS(csgo_cz75, CCz75)

void CCz75::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_cz75");

	Precache();
	m_iId = WEAPON_FIVESEVEN;
	SET_MODEL(ENT(pev), "models/w_cz75.mdl");
	m_bHasReload = FALSE;
	m_iDefaultAmmo = CZ75_DEFAULT_GIVE;
	m_flAccuracy = 0.92;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CCz75::Precache(void)
{
	PRECACHE_MODEL("models/v_cz75.mdl");
	PRECACHE_MODEL("models/alt/v_cz75.mdl");
	PRECACHE_MODEL("models/w_cz75.mdl");
	PRECACHE_MODEL("models/p_cz75.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_cz75.mdl");
#endif

	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75-1.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_clipout.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_clipin.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_magattach.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_magdrop.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_maggrab.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_draw.wav");
	PRECACHE_SOUND("weapons/csgo_ports/cz75/cz75_slidepull.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireCz75 = PRECACHE_EVENT(1, "events/cz75.sc");
}

int CCz75::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "57mm";
	p->iMaxAmmo1 = CZ75_DEFAULT_GIVE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = CZ75_DEFAULT_GIVE;
	p->iSlot = 1;
	p->iPosition = 6;
	p->iId = m_iId = WEAPON_FIVESEVEN;
	p->iFlags = 0;
	p->iWeight = FIVESEVEN_WEIGHT;

	return 1;
}

BOOL CCz75::Deploy(void)
{
	m_flAccuracy = 0.92;
	m_fMaxSpeed = 250;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
	m_NextInspect = gpGlobals->time + 0.75s;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_cz75.mdl", "models/shield/p_shield_cz75.mdl", cz75_DRAW, "shieldgun", UseDecrement() != FALSE);
	else
#endif
	{
		if(m_bHasReload)
			return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_cz75.mdl" : "models/alt/v_cz75.mdl", "models/p_cz75.mdl", CZ75_DRAW_2, "onehanded", UseDecrement() != FALSE);
		else
			return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_cz75.mdl" : "models/alt/v_cz75.mdl", "models/p_cz75.mdl", CZ75_DRAW_1, "onehanded", UseDecrement() != FALSE);
	}
}

void CCz75::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Cz75Fire((1.5) * (1 - m_flAccuracy), 0.1s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Cz75Fire((0.255) * (1 - m_flAccuracy), 0.1s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Cz75Fire((0.075) * (1 - m_flAccuracy), 0.1s, FALSE);
	else
		Cz75Fire((0.15) * (1 - m_flAccuracy), 0.1s, FALSE);
}

void CCz75::Cz75Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_iShotsFired++;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.275 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.25;

		if (m_flAccuracy > 0.92)
			m_flAccuracy = 0.92;
		else if (m_flAccuracy < 0.725)
			m_flAccuracy = 0.725;
	}

	m_NextInspect = gpGlobals->time;

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
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096, 1, BULLET_PLAYER_57MM, 20, 0.885, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireCz75, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_bHasReload, m_iClip != 0);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

void CCz75::Reload(void)
{
	if (m_pPlayer->ammo_57mm <= 0)
		return;
	if (m_bHasReload)
	{
		if (DefaultReload(CZ75_MAX_CLIP, CZ75_RELOAD_2, CZ75_RELOAD_TIME_2))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
			if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
			{
				m_pPlayer->m_flNextAttack = 1.26s;
				m_flTimeWeaponIdle = CZ75_RELOAD_TIME_2 + 0.5s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + CZ75_RELOAD_TIME_2;
			}
#endif
		
			m_flAccuracy = 0.92;
		}
		m_NextInspect = gpGlobals->time + CZ75_RELOAD_TIME_2;
		
	}
	else
	{
		if (DefaultReload(CZ75_MAX_CLIP, CZ75_RELOAD_1, CZ75_RELOAD_TIME_1))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
			if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
			{
				m_pPlayer->m_flNextAttack = 1.3s;
				m_flTimeWeaponIdle = CZ75_RELOAD_TIME_1 + 0.5s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + CZ75_RELOAD_TIME_1;
			}
#endif
			
			m_flAccuracy = 0.92;
		}
		m_NextInspect = gpGlobals->time + CZ75_RELOAD_TIME_1;
		m_bHasReload = TRUE;
	}
}

void CCz75::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_bHasReload)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625s;
		SendWeaponAnim(CZ75_IDLE_2, UseDecrement() != FALSE);
	}
	else
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0625s;
		SendWeaponAnim(CZ75_IDLE_1, UseDecrement() != FALSE);
	}
}

float CCz75::GetDamage()
{
	float flDamage = 31.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 40.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 40.0f;
#endif
	return flDamage;
}

void CCz75::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
			
#ifndef CLIENT_DLL
			if (m_bHasReload)
				SendWeaponAnim(CZ75_INSPECT_2, 0);
			else
				SendWeaponAnim(CZ75_INSPECT_1, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
