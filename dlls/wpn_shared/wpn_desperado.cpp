/***
*	Made by MoeMod & TmNine!~
*	Modification by JustANoobQAQ
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
#include "wpn_desperado.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum desperado_e
{
	ANIM_IDLE_M = 0,
	ANIM_RUN_START_M,
	ANIM_RUN_IDLE_M,
	ANIM_RUN_END_M,
	ANIM_DRAW_M,
	ANIM_SHOOT_M,
	ANIM_RELOAD_M,
	ANIM_SWAB_M,	// Gold
	ANIM_IDLE_W,
	ANIM_RUN_START_W,
	ANIM_RUN_IDLE_W,
	ANIM_RUN_END_W,
	ANIM_DRAW_W,
	ANIM_SHOOT_W,
	ANIM_RELOAD_W,
	ANIM_SWAB_W	// Blue
};

LINK_ENTITY_TO_CLASS(weapon_desperado, CDesperado)

void CDesperado::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_desperado");

	Precache();
	m_iId = WEAPON_ELITE;
	SET_MODEL(ENT(pev), "models/w_desperado.mdl");

	m_iDefaultAmmo = DESPERADO_MAX_CLIP;
	m_flAccuracy = 1.0;
	m_iShotsFired = 0;

	FallInit();
}

void CDesperado::Precache(void)
{
	PRECACHE_MODEL("models/v_desperado.mdl");
	PRECACHE_MODEL("models/w_desperado.mdl");
	PRECACHE_MODEL("models/p_desperado_m.mdl");
	PRECACHE_MODEL("models/p_desperado_w.mdl");

	PRECACHE_SOUND("weapons/dprd-1.wav");
	PRECACHE_SOUND("weapons/dprd_reload_m.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireDesperado = PRECACHE_EVENT(1, "events/desperado.sc");
}

int CDesperado::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "44magnum";
	p->iMaxAmmo1 = 200;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = DESPERADO_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_ELITE;
	p->iFlags = 0;
	p->iWeight = ELITE_WEIGHT;

	return 1;
}

BOOL CDesperado::Deploy(void)
{
	ResetRunIdle();
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.01s;

	m_flAccuracy = 1.0f;
	m_iShotsFired = 0;
	m_fMaxSpeed = 250.0f;

	if (!m_iClip)
	{
		if (pev->iuser1)
		{
			if (DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_m.mdl", ANIM_RELOAD_W, "onehanded", UseDecrement() != FALSE))
			{
				Reload();
				return TRUE;
			}
		}
		else
		{
			if (DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_w.mdl", ANIM_RELOAD_M, "onehanded", UseDecrement() != FALSE))
			{
				Reload();
				return TRUE;
			}
		}
	}


	if (pev->iuser1)
	{
		if (DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_m.mdl", ANIM_DRAW_W, "onehanded", UseDecrement() != FALSE))
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2s;
			return TRUE;
		}
	}
	else
	{
		if (DefaultDeploy("models/v_desperado.mdl", "models/p_desperado_w.mdl", ANIM_DRAW_M, "onehanded", UseDecrement() != FALSE))
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.2s;
			return TRUE;
		}
	}
	return FALSE;
}

void CDesperado::SecondaryAttack(void)
{
	if (m_pPlayer->pev->button & IN_ATTACK)
		return;

	if (pev->iuser1 != 1)
	{
		int j = Q_min(DESPERADO_MAX_CLIP - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		m_flAccuracy = 1;

		SendWeaponAnim(ANIM_SWAB_M, UseDecrement() != FALSE);
		pev->iuser1 = 1;
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_desperado_w.mdl");

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.17s;
		ResetRunIdle();
		return;
	}

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		DesperadoFire(0.15 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		DesperadoFire(0.12 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		DesperadoFire(0.1 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else
		DesperadoFire(0.11 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
}

void CDesperado::PrimaryAttack(void)
{
	if (m_pPlayer->pev->button & IN_ATTACK2)
		return;

	if (pev->iuser1 != 0)
	{
		int j = Q_min(DESPERADO_MAX_CLIP - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
		m_flAccuracy = 1;

		SendWeaponAnim(ANIM_SWAB_W, UseDecrement() != FALSE);
		pev->iuser1 = 0;
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_desperado_m.mdl");

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.17s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.17s;
		ResetRunIdle();
		return;
	}

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		DesperadoFire(0.15 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		DesperadoFire(0.12 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		DesperadoFire(0.1 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
	else
		DesperadoFire(0.11 * (1.0 - m_flAccuracy), FALSE, pev->iuser1);
}

void CDesperado::DesperadoFire(float flSpread, BOOL fUseAutoAim, int iMode)
{
	m_iShotsFired++;
	m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.2;

	if (m_flAccuracy > 1.0)
		m_flAccuracy = 1.0;
	else if (m_flAccuracy < 0.5)
		m_flAccuracy = 0.5;

	m_flLastFire = gpGlobals->time;

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
		}

		return;
	}

	m_iClip--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 6, BULLET_PLAYER_50AE, GetDamage(), 0.9, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireDesperado, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, pev->iuser1);
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (pev->iuser1)
	{
		if (m_iClip <= 0)
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
		}
		else
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
		}
	}
	else
	{
		if (m_iClip <= 0)
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
		}
		else
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
		}
	}

	m_pPlayer->pev->punchangle.x -= 0.33;
}

void CDesperado::Reload(void)
{

	if (m_flNextPrimaryAttack <= 0.0s && m_flNextSecondaryAttack <= 0.0s)
	{
		if (DefaultReload(DESPERADO_MAX_CLIP, pev->iuser1 ? ANIM_RELOAD_W : ANIM_RELOAD_M, 0.47s))
		{
			ResetRunIdle();
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 1;
			m_iShotsFired = 0;
		}
	}
}

void CDesperado::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_pPlayer->pev->button & IN_FORWARD)
		{
			if (pev->iuser2)
			{
				SendWeaponAnim(pev->iuser1 ? ANIM_RUN_START_W : ANIM_RUN_START_M, UseDecrement() != FALSE);
				pev->iuser2 = 0;
				m_flNextRunIdle = gpGlobals->time;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.33s;
				return;
			}
			else if (m_flNextRunIdle <= gpGlobals->time)
			{
				SendWeaponAnim(pev->iuser1 ? ANIM_RUN_IDLE_W : ANIM_RUN_IDLE_M, UseDecrement() != FALSE);
				m_flNextRunIdle = gpGlobals->time + 0.57s;
			}
			m_flNextRunEnd = gpGlobals->time;
		}
		else
		{
			if (!pev->iuser2)
			{
				SendWeaponAnim(pev->iuser1 ? ANIM_RUN_END_W : ANIM_RUN_END_M, UseDecrement() != FALSE);
				pev->iuser2 = 1;
				m_flNextRunEnd = gpGlobals->time;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.3s;
				return;
			}
			else if( m_flNextRunEnd<= gpGlobals->time)
			{
				SendWeaponAnim(pev->iuser1 ? ANIM_IDLE_W : ANIM_IDLE_M, UseDecrement() != FALSE);
				m_flNextRunEnd = gpGlobals->time + 3.0s;
			}
			m_flNextRunIdle = gpGlobals->time;
		}
	}
}

void CDesperado::ResetRunIdle()
{
	pev->iuser2 = 1;
	m_flNextRunIdle = m_flNextRunEnd = gpGlobals->time;
}

int CDesperado::GetDamage() const
{
	int iDamage = 40;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		iDamage = 95;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		iDamage = 95;
#endif
	return iDamage;
}

}
