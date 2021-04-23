/*
z4b_dmp7a1x.cpp
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "z4b_dmp7a1x.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_dmp7a1x, CZ4B_MP7A1DX)

static const int WEAPON_MAXCLIP = 80;

void CZ4B_MP7A1DX::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_dmp7a1x");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/z4b/w_dmp7a1x.mdl");

	m_iDefaultAmmo = WEAPON_MAXCLIP;
	m_flAccuracy = 0.0;

	FallInit();
}

void CZ4B_MP7A1DX::Precache(void)
{
	PRECACHE_MODEL("models/z4b/v_dmp7a1x.mdl");
	PRECACHE_MODEL("models/z4b/p_dmp7a1x.mdl");
	PRECACHE_MODEL("models/z4b/w_dmp7a1x.mdl");

	PRECACHE_SOUND("weapons/dmp7-1.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireELITE_LEFT = PRECACHE_EVENT(1, "events/mp7a1d_left.sc");
	m_usFireELITE_RIGHT = PRECACHE_EVENT(1, "events/mp7a1d_right.sc");
}

int CZ4B_MP7A1DX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = MAX_AMMO_9MM;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 7;
	p->iId = m_iId = WEAPON_MP5N;
	p->iFlags = 0;
	p->iWeight = MP5NAVY_WEIGHT;

	return 1;
}

BOOL CZ4B_MP7A1DX::Deploy(void)
{
	m_flAccuracy = 0.0;

	if (!(m_iClip & 1))
		m_iWeaponState |= WPNSTATE_ELITE_LEFT;

	return DefaultDeploy("models/z4b/v_dmp7a1x.mdl", "models/z4b/p_dmp7a1x.mdl", ELITE_DRAW, "dualpistols", UseDecrement() != FALSE);
}

void CZ4B_MP7A1DX::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		ELITEFire((0.375) * m_flAccuracy, 0.07s, FALSE);
	else
		ELITEFire((0.035) * m_flAccuracy, 0.07s, FALSE);
}

void CZ4B_MP7A1DX::ELITEFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190) + 0.6;

	if (m_flAccuracy > 1.5)
		m_flAccuracy = 1.5;

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

	Vector vecDir;
	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	if (m_iWeaponState & WPNSTATE_ELITE_LEFT)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_iWeaponState &= ~WPNSTATE_ELITE_LEFT;

		vecDir = FireBullets3(m_pPlayer->GetGunPosition() - gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_57MM, 70, 0.83, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireELITE_LEFT, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip, FALSE, FALSE);
	}
	else
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
		m_iWeaponState |= WPNSTATE_ELITE_LEFT;

		vecDir = FireBullets3(m_pPlayer->GetGunPosition() - gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_57MM, 70, 0.83, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireELITE_RIGHT, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip, FALSE, FALSE);
	}

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1, 0.55, 0.4, 0.05, 5, 3.25, 9);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
	else
		KickBack(0.275, 0.2, 0.15, 0.02, 3, 1.75, 9);
}

void CZ4B_MP7A1DX::Reload(void)
{
	if (DefaultReload(WEAPON_MAXCLIP, ELITE_RELOAD, 3.6s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.0;
	}
}

void CZ4B_MP7A1DX::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;

		if (m_iClip == 1)
			SendWeaponAnim(ELITE_IDLE_LEFTEMPTY, UseDecrement() != FALSE);
		else
			SendWeaponAnim(ELITE_IDLE, UseDecrement() != FALSE);
	}
}

float CZ4B_MP7A1DX::GetDamage() const
{
	float flDamage = 27.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 46.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 33.0f;
#endif
	return flDamage;
}

}
