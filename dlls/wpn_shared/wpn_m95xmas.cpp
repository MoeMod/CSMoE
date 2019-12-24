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
#include "wpn_m95xmas.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum m95xmas_e
{
	M95_IDLE,
	M95_SHOOT1,
	M95_SHOOT2,
	M95_RELOAD,
	M95_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_m95xmas, CM95xmas)

const int M95_MAXCLIP = 5;

void CM95xmas::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m95xmas");

	Precache();
	m_iId = WEAPON_AWP;
	SET_MODEL(ENT(pev), "models/w_m95xmas.mdl");

	m_iDefaultAmmo = M95_MAXCLIP;
	FallInit();
}

void CM95xmas::Precache(void)
{
	PRECACHE_MODEL("models/v_m95xmas.mdl");
	PRECACHE_MODEL("models/p_m95xmas.mdl");
	PRECACHE_MODEL("models/w_m95xmas.mdl");

	PRECACHE_SOUND("weapons/m95-1.wav");
	PRECACHE_SOUND("weapons/boltpull1.wav");
	PRECACHE_SOUND("weapons/boltup.wav");
	PRECACHE_SOUND("weapons/boltdown.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell_big.mdl");
	m_usFireM95xmas = PRECACHE_EVENT(1, "events/m95xmas.sc");
}

int CM95xmas::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50BMG";
	p->iMaxAmmo1 = MAX_AMMO_50BMG;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M95_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_AWP;
	p->iFlags = 0;
	p->iWeight = AWP_WEIGHT;

	return 1;
}

BOOL CM95xmas::Deploy(void)
{
	if (DefaultDeploy("models/v_m95xmas.mdl", "models/p_m95xmas.mdl", M95_DRAW, "rifle", UseDecrement() != FALSE))
	{
		m_flNextPrimaryAttack = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.25s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
		return TRUE;
	}

	return FALSE;
}

void CM95xmas::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
		case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
		case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 10; break;
		default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}

void CM95xmas::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		M95xmasFire(1.0, 1.5s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 120)
		M95xmasFire(0.3, 1.5s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 10)
		M95xmasFire(0.1, 1.5s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		M95xmasFire(0.0, 1.5s, FALSE);
	else
		M95xmasFire(0.002, 1.5s, FALSE);
}

void CM95xmas::M95xmasFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_bResumeZoom = true;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}
	else // not zoomed, so bullets diverts more
		flSpread += 0.03;

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

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 4, BULLET_PLAYER_338MAG, GetDamage(), M95_RANGE_MODIFER, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireM95xmas, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_pPlayer->pev->punchangle.x -= 12;
}

void CM95xmas::Reload(void)
{
	if (DefaultReload(M95_MAXCLIP, M95_RELOAD, M95_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 10;
			SecondaryAttack();
		}
	}
}

void CM95xmas::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;
		SendWeaponAnim(M95_IDLE, UseDecrement() != FALSE);
	}
}

float CM95xmas::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return M95_MAX_SPEED;

	return M95_MAX_SPEED_ZOOM;
}

float CM95xmas::GetDamage() const
{
	float flDamage = 120.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 399.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 399.0f;
#endif
	return flDamage;
}
}
