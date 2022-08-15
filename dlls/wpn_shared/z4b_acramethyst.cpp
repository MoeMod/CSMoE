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
#include "z4b_acramethyst.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

	LINK_ENTITY_TO_CLASS(z4b_acramethyst, CACRAMETHYST)

	void CACRAMETHYST::Spawn(void)
	{
		pev->classname = MAKE_STRING("z4b_acramethyst");

		Precache();
		m_iId = WEAPON_Z4B_ACRAMETHYST;
		SET_MODEL(ENT(pev), "models/w_acr_amethyst.mdl");

		m_iDefaultAmmo = 9999;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
#ifndef CLIENT_DLL
		if ((int)CVAR_GET_FLOAT("mp_csgosilencedwpn"))
		{
			m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
		}
#endif
		m_bDelayFire = true;

		FallInit();
	}

	void CACRAMETHYST::Precache(void)
	{
		PRECACHE_MODEL("models/v_acr_amethyst.mdl");
		PRECACHE_MODEL("models/p_acr_amethyst.mdl");
		PRECACHE_MODEL("models/w_acr_amethyst.mdl");

		PRECACHE_SOUND("weapons/m4a1mw-1.wav");
		PRECACHE_SOUND("weapons/m4a1buff-1.wav");
		PRECACHE_SOUND("weapons/m4a1buff-1.wav");

		m_iShell = PRECACHE_MODEL("models/rshell.mdl");
		m_usFireACRAMETHYST = PRECACHE_EVENT(1, "events/acramethyst.sc");
	}

	int CACRAMETHYST::GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "ammo_amethyst";
		p->iMaxAmmo1 = 9999;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 6;
		p->iId = m_iId = WEAPON_Z4B_ACRAMETHYST;
		p->iFlags = 0;
		p->iWeight = M4A1_WEIGHT;

		return 1;
	}

	BOOL CACRAMETHYST::Deploy(void)
	{
#ifndef CLIENT_DLL
		if (m_pPlayer->IsAlive())
			CheckWeapon(m_pPlayer, this);
#endif
		m_bDelayFire = true;
		iShellOn = 1;
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_NextInspect = gpGlobals->time + 0.75s;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			return DefaultDeploy("models/v_acr_amethyst.mdl", "models/p_acr_amethyst.mdl", 9, "rifle", UseDecrement() != FALSE);
		else
			return DefaultDeploy("models/v_acr_amethyst.mdl", "models/p_acr_amethyst.mdl", 2, "rifle", UseDecrement() != FALSE);
	}

	void CACRAMETHYST::SecondaryAttack(void)
	{
		m_flLastFire = invalid_time_point;
		m_NextInspect = gpGlobals->time + 2s;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
			SendWeaponAnim(13, UseDecrement() != FALSE);
			strcpy(m_pPlayer->m_szAnimExtention, "rifle");
		}
		else
		{
			m_iWeaponState |= WPNSTATE_M4A1_SILENCED;
			SendWeaponAnim(6, UseDecrement() != FALSE);
			strcpy(m_pPlayer->m_szAnimExtention, "rifle");
		}
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
	}

	void CACRAMETHYST::PrimaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				ACRAMETHYSTFire(0.035 + (0.4) * m_flAccuracy, 0.0875s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 140)
				ACRAMETHYSTFire(0.035 + (0.07) * m_flAccuracy, 0.0875s, FALSE);
			else
				ACRAMETHYSTFire((0.025) * m_flAccuracy, 0.0875s, FALSE);
		}
		else
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				ACRAMETHYSTFire(0.035 + (0.4) * m_flAccuracy, 0.0875s, FALSE);
			else if (m_pPlayer->pev->velocity.Length2D() > 140)
				ACRAMETHYSTFire(0.035 + (0.07) * m_flAccuracy, 0.0875s, FALSE);
			else
				ACRAMETHYSTFire((0.02) * m_flAccuracy, 0.0875s, FALSE);
		}
	}

	void CACRAMETHYST::ACRAMETHYSTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		m_bDelayFire = true;
		m_iShotsFired++;
		m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

		float iDamage = 1.0f;
		if (iDamage < 3.0f)
		{
			if (m_iShots < 10)
			{
				m_iShots++;
			}
			else
			{
				m_iShots = 0;
				iDamage += 0.5f;
			}
		}
		else
		{
			m_iShots = 0;
			iDamage = 1.0f;
		}

		if (m_flAccuracy > 1)
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
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		{
			vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamageA() * iDamage, 0.95, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
		}
		else
		{
			vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamageB() * iDamage, 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
			m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
		}

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireACRAMETHYST, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_M4A1_SILENCED) ? true : false, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
		m_NextInspect = gpGlobals->time;
		if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7);
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7);
		else
			KickBack(0.65, 0.35, 0.25, 0.015, 3.5, 2.25, 7);
	}

	void CACRAMETHYST::Reload(void)
	{

		int iAnim;
		m_NextInspect = gpGlobals->time + 3.03s;
		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			iAnim = 8;
		else
			iAnim = 1;

		if (DefaultReload(30, iAnim, 3.03s))
		{
#ifndef CLIENT_DLL
			if ((int)CVAR_GET_FLOAT("mp_csgospecialeffect"))
			{
				m_pPlayer->m_flNextAttack = 1.84s;
				m_flTimeWeaponIdle = 3.03s + 0.5s;
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.03s;
			}
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			m_flAccuracy = 0.2;
			m_iShotsFired = 0;
			m_bDelayFire = false;
		}
	}

	void CACRAMETHYST::WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
			SendWeaponAnim(7, UseDecrement() != FALSE);
		else
			SendWeaponAnim(0, UseDecrement() != FALSE);
	}

	float CACRAMETHYST::GetDamageA()
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 71.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 38.0f;
#endif
		return 32.0f;
	}

	float CACRAMETHYST::GetDamageB()
	{
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZBS)
			return 72.0f;
		if (g_pModRunning->DamageTrack() == DT_ZB)
			return 39.0f;
#endif
		return 33.0f;
	}
}
