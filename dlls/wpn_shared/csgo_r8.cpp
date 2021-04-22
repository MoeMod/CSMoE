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
#include "csgo_r8.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum CR8::R8_E
{
	IDLE,
	DRAW,
	DRAW_ALT,
	PREPARE,
	UNPREPARE,
	FIRE,
	DRYFIRE,
	ALT1,
	ALT2,
	ALT3,
	LOOKAT01,
	RELOAD,
	LOOKAT01_STICKER,
	LOOKAT01_STICKER_LOOP

};

LINK_ENTITY_TO_CLASS(csgo_r8, CR8)

void CR8::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_r8");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/w_r8.mdl");

	m_iDefaultAmmo = R8_MAX_CLIP;
	m_flAccuracy = 0.92;

	FallInit();
}

void CR8::Precache(void)
{
	PRECACHE_MODEL("models/v_r8.mdl");
	PRECACHE_MODEL("models/alt/v_r8.mdl");
	PRECACHE_MODEL("models/w_r8.mdl");
	PRECACHE_MODEL("models/p_r8.mdl");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8-1.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_siderelease.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_clipout.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_clipin.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_sideback.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_barrelroll.wav");
	PRECACHE_SOUND("weapons/csgo_ports/r8/r8_draw.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireR8 = PRECACHE_EVENT(1, "events/r8.sc");
}

int CR8::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = MAX_AMMO_50AE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = R8_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

BOOL CR8::Deploy(void)
{
	m_flAccuracy = 0.92;
	m_NextInspect = gpGlobals->time + 0.75s;

	if (DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_r8.mdl" : "models/alt/v_r8.mdl", "models/p_r8.mdl", RANDOM_LONG(DRAW, DRAW_ALT), "onehanded", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.04s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.04s;
		return TRUE;
	}
	return FALSE;
}

void CR8::PrimaryAttack(void)
{
	m_iShotsFired++;
	m_NextInspect = gpGlobals->time;

	if (m_iShotsFired > 1)
		return;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	SetThink(&CR8::R8FireA);
	pev->nextthink = gpGlobals->time + 0.01s;
	SendWeaponAnim(PREPARE, UseDecrement() != FALSE);


	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.1s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
	m_tPreTime = gpGlobals->time + 0.4s;
}


void CR8::SecondaryAttack(void)
{
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		R8Fire((1.5) * (1 - m_flAccuracy), 1.04s, FALSE, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		R8Fire((0.25) * (1 - m_flAccuracy), 1.04s, FALSE, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		R8Fire((0.115) * (1 - m_flAccuracy), 1.04s, FALSE, FALSE);
	else
		R8Fire((0.13) * (1 - m_flAccuracy), 1.04s, FALSE, FALSE);
}

void CR8::R8FireA()
{
	if (m_pPlayer->pev->button & IN_ATTACK)
	{
		if(m_tPreTime < gpGlobals->time)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			{
				R8Fire((0.95) * (1 - m_flAccuracy), 0.73s, FALSE, TRUE);
			}
			else if (m_pPlayer->pev->velocity.Length2D() > 0)
			{
				R8Fire((0.15) * (1 - m_flAccuracy), 0.73s, FALSE, TRUE);
			}
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			{
				R8Fire((0.085) * (1 - m_flAccuracy), 0.73s, FALSE, TRUE);
			}
			else
			{
				R8Fire((0.073) * (1 - m_flAccuracy), 0.73s, FALSE, TRUE);
			}
			SetThink(nullptr);
		}
		else
		{
			m_pPlayer->pev->button &= ~IN_ATTACK;
			pev->nextthink = gpGlobals->time + 0.01s;
		}
	}
	else
	{
		SetThink(nullptr);
		SendWeaponAnim(UNPREPARE, UseDecrement() != FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + gpGlobals->time - m_tPreTime;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	}

}

void CR8::R8Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim, BOOL fIsModeA)
{
	

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;
		if (fIsModeA)
		{
			if (m_flAccuracy > 0.92)
				m_flAccuracy = 0.92;
			else if (m_flAccuracy < 0.75)
				m_flAccuracy = 0.75;
		}
		else
		{
			if (m_flAccuracy > 0.75)
				m_flAccuracy = 0.75;
			else if (m_flAccuracy < 0.55)
				m_flAccuracy = 0.55;
		}

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
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 2, BULLET_PLAYER_50AE, GetDamage(fIsModeA), 0.81, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	PLAYBACK_EVENT_FULL(0, ENT(m_pPlayer->pev), m_usFireR8, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, fIsModeA);

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	m_pPlayer->pev->punchangle.x -= 2;
}

void CR8::Reload(void)
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;
	m_NextInspect = gpGlobals->time + R8_RELOAD_TIME;
	if (DefaultReload(R8_MAX_CLIP, RELOAD, R8_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			m_pPlayer->m_flNextAttack = 1.96s;
			m_flTimeWeaponIdle = R8_RELOAD_TIME + 0.5s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + R8_RELOAD_TIME;
		}
#endif
		
		m_flAccuracy = 0.92;
	}
}

void CR8::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
	SendWeaponAnim(IDLE, UseDecrement() != FALSE);
}

float CR8::GetDamage(BOOL fIsModeA)
{
	float flDamage;
	if (fIsModeA)
	{
		flDamage = 86.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 109.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 109.0f;
#endif
	}
	else
	{
		flDamage = 86.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 109.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 109.0f;
#endif
	}

	return flDamage;
}

void CR8::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(LOOKAT01, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
