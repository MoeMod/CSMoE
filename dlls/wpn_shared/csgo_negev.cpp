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
#include "csgo_negev.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum CNegev::negev_e
{
	NEGEV_IDLE1,
	NEGEV_SHOOT1,
	NEGEV_SHOOT2,
	NEGEV_RELOAD,
	NEGEV_DRAW,
	NEGEV_INSPECT
};

LINK_ENTITY_TO_CLASS(csgo_negev, CNegev)

static const int NEGEV_AMMO_GIVE = 150;

void CNegev::Spawn(void)
{
	pev->classname = MAKE_STRING("csgo_negev");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_negev.mdl");

	m_iDefaultAmmo = NEGEV_AMMO_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CNegev::Precache(void)
{
	PRECACHE_MODEL("models/v_negev.mdl");
	PRECACHE_MODEL("models/alt/v_negev.mdl");
	PRECACHE_MODEL("models/w_negev.mdl");
	PRECACHE_MODEL("models/p_negev.mdl");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negv-1.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_boxin.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_boxout.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_chain.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_coverdown.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_coverup.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_pump.wav");
	PRECACHE_SOUND("weapons/csgo_ports/negev/negev_draw.wav");


	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireNegev = PRECACHE_EVENT(1, "events/negev.sc");
}

int CNegev::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = 150;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 150;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_M249;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL CNegev::Deploy(void)
{
#ifndef CLIENT_DLL
	if (m_pPlayer->IsAlive())
		CheckWeapon(m_pPlayer, this);
#endif

	iShellOn = 1;
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_negev.mdl" : "models/alt/v_negev.mdl", "models/p_negev.mdl", NEGEV_DRAW, "m249", UseDecrement() != FALSE);
}

void CNegev::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		NegevFire(0.045 + (0.5) * m_flAccuracy, 0.077s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		NegevFire(0.045 + (0.095) * m_flAccuracy, 0.077s, FALSE);
	else
		NegevFire((0.03) * m_flAccuracy, 0.077s, FALSE);
}

void CNegev::NegevFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 6750.0) + 0.4;

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

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireNegev, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
	m_NextInspect = gpGlobals->time;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.65, 0.45, 0.125, 5.0, 0.5, 8);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.1, 0.5, 0.3, 0.06, 4.0, 0.5, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.75, 0.325, 0.25, 0.025, 3.5, 0.5, 9);
	else
		KickBack(0.8, 0.35, 0.3, 0.03, 3.75, 0.5, 9);
}

void CNegev::Reload(void)
{
	if (m_pPlayer->ammo_556natobox <= 0)
		return;

	if (DefaultReload( 150, NEGEV_RELOAD, 5.64s))
	{
		
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
		{
			m_pPlayer->m_flNextAttack = 3.92s;
			m_flTimeWeaponIdle = 6.64s;
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 5.64s;
		}
#endif
		
		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void CNegev::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 70s;
	SendWeaponAnim(NEGEV_IDLE1, UseDecrement() != FALSE);
}

float CNegev::GetDamage() const
{
	float flDamage = 35.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 41.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 41.0f;
#endif
	return flDamage;
}


void CNegev::Inspect()
{
	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(NEGEV_INSPECT, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime() + 0.5s;
		}
	}

}
}
