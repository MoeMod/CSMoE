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
#include "csgo_tec9.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum tec9_e
{
	TEC9_IDLE,
	TEC9_RELOAD,
	TEC9_DRAW,
	TEC9_SHOOT1,
	TEC9_SHOOT2,
	TEC9_SHOOT3,
	TEC9_INSPECT

};

LINK_ENTITY_TO_CLASS(csgo_tec9, CTec9)

void CTec9::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_tec9");

	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_tec9.mdl");

	m_iDefaultAmmo = TEC9_MAX_CLIP;
	m_flAccuracy = 0.88;

	FallInit();
}

void CTec9::Precache(void)
{
	PRECACHE_MODEL("models/v_tec9.mdl");
	PRECACHE_MODEL("models/alt/v_tec9.mdl");
	PRECACHE_MODEL("models/w_tec9.mdl");
	PRECACHE_MODEL("models/p_tec9.mdl");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9-1.wav");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9_clipout.wav");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9_clipin.wav");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9_boltrelease.wav");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9_boltpull.wav");
	PRECACHE_SOUND("weapons/csgo_ports/tec9/tec9_draw.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireTec9 = PRECACHE_EVENT(1, "events/tec9.sc");
}

int CTec9::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = 90;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = TEC9_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_ELITE;
	p->iFlags = 0;
	p->iWeight = ELITE_WEIGHT;

	return 1;
}

BOOL CTec9::Deploy(void)
{
	m_flAccuracy = 0.88;
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_tec9.mdl" : "models/alt/v_tec9.mdl", "models/p_tec9.mdl", TEC9_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CTec9::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Tec9Fire((1.3) * (1 - m_flAccuracy), 0.125s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Tec9Fire((0.175) * (1 - m_flAccuracy), 0.125s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Tec9Fire((0.08) * (1 - m_flAccuracy), 0.125s, FALSE);
	else
		Tec9Fire((0.1) * (1 - m_flAccuracy), 0.125s, FALSE);
}

void CTec9::Tec9Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.325 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.275;

		if (m_flAccuracy > 0.88)
			m_flAccuracy = 0.88;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
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
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096, 1, BULLET_PLAYER_57MM, GetDamage(), 0.75, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireTec9, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_pPlayer->pev->punchangle.x -= 2;
}

void CTec9::Reload(void)
{
	if (m_pPlayer->ammo_9mm <= 0)
		return;
	m_NextInspect = gpGlobals->time + TEC9_RELOAD_TIME;
	if (DefaultReload(TEC9_MAX_CLIP, TEC9_RELOAD, TEC9_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			m_pPlayer->m_flNextAttack = 1.38s;
			m_flTimeWeaponIdle = TEC9_RELOAD_TIME + 0.5s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + TEC9_RELOAD_TIME;
		}
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif	
		m_flAccuracy = 0.88;
	}
}

void CTec9::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
	SendWeaponAnim(TEC9_IDLE, UseDecrement() != FALSE);
}

float CTec9::GetDamage()
{
	float flDamage = 36.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 36.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 36.0f;
#endif
	return flDamage;
}

void CTec9::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(TEC9_INSPECT, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
