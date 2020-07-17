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
#include "csgo_mag7.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum mag7_e
{
	MAG7_IDLE,
	MAG7_FIRE1,
	MAG7_FIRE2,
	MAG7_RELOAD,
	MAG7_DRAW,
	MAG7_INSPECT,
};

LINK_ENTITY_TO_CLASS(csgo_mag7, CMag7)

void CMag7::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_mag7.mdl");

	m_iDefaultAmmo = MAG7_DEFAULT_GIVE;

	FallInit();
}

void CMag7::Precache(void)
{
	PRECACHE_MODEL("models/v_mag7.mdl");
	PRECACHE_MODEL("models/alt/v_mag7.mdl");
	PRECACHE_MODEL("models/w_mag7.mdl");
	PRECACHE_MODEL("models/p_mag7.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7-1.wav");
	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7_clipin.wav");
	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7_clipout.wav");
	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7_boltpull.wav");
	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7_draw.wav");
	PRECACHE_SOUND("weapons/csgo_ports/mag7/mag7_pump.wav");

	m_usFireMag7 = PRECACHE_EVENT(1, "events/mag7.sc");
}

int CMag7::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = 32;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = MAG7_DEFAULT_GIVE;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CMag7::Deploy(void)
{
	m_NextInspect = gpGlobals->time + 0.75s;
	return DefaultDeploy(m_pPlayer->m_iTeam == CT ? "models/v_mag7.mdl" : "models/alt/v_mag7.mdl", "models/p_mag7.mdl", MAG7_DRAW, "m249", UseDecrement() != FALSE);
}

void CMag7::ItemPostFrame(void)
{
	m_iShotsFired = 0;
	return CBasePlayerWeapon::ItemPostFrame();
}

void CMag7::PrimaryAttack(void)
{
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.06, 0.06, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetPrimaryAttackDamage());
#endif
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireMag7, 0, (float *)&g_vecZero, (float *)&g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);



#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_NextInspect = gpGlobals->time;
	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
	else
		m_flTimeWeaponIdle = 0.75s;


	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 13, 16);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 18, 21);
}

void CMag7::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(MAG7_DEFAULT_GIVE, MAG7_RELOAD, 1.2s))
	{
		
#ifndef CLIENT_DLL
		
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
	m_pPlayer->m_flNextAttack = 2.44s;
	m_flTimeWeaponIdle = 2.94s;
}

void CMag7::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(MAG7_IDLE, UseDecrement() != FALSE);
	
}

int CMag7::GetPrimaryAttackDamage() const
{
	int iDamage = 26;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		iDamage = 40;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		iDamage = 40;
#endif
	return iDamage;
}

void CMag7::Inspect()
{
	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			SendWeaponAnim(MAG7_INSPECT, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime() + 0.5s;
		}
	}

}
}
