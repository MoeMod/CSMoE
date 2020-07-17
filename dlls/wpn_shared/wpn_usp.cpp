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
#include "wpn_usp.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum usp_e
{
	USP_IDLE,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOT_EMPTY,
	USP_RELOAD,
	USP_DRAW,
	USP_ATTACH_SILENCER,
	USP_UNSIL_IDLE,
	USP_UNSIL_SHOOT1,
	USP_UNSIL_SHOOT2,
	USP_UNSIL_SHOOT3,
	USP_UNSIL_SHOOT_EMPTY,
	USP_UNSIL_RELOAD,
	USP_UNSIL_DRAW,
	USP_DETACH_SILENCER
};

enum usp_shield_e
{
	USP_SHIELD_IDLE,
	USP_SHIELD_SHOOT1,
	USP_SHIELD_SHOOT2,
	USP_SHIELD_SHOOT_EMPTY,
	USP_SHIELD_RELOAD,
	USP_SHIELD_DRAW,
	USP_SHIELD_UP_IDLE,
	USP_SHIELD_UP,
	USP_SHIELD_DOWN
};

LINK_ENTITY_TO_CLASS(weapon_usp, CUSP)

void CUSP::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_usp");

	Precache();
	m_iId = WEAPON_USP;
	SET_MODEL(ENT(pev), "models/w_usp.mdl");
#ifndef CLIENT_DLL
	if ((int)CVAR_GET_FLOAT("mp_csgosilencedwpn"))
	{
		m_iWeaponState |= WPNSTATE_USP_SILENCED;
	}
#endif
	m_iDefaultAmmo = USP_DEFAULT_GIVE;
	m_flAccuracy = 0.92;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CUSP::Precache(void)
{
	PRECACHE_MODEL("models/v_usp.mdl");
	PRECACHE_MODEL("models/w_usp.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_usp.mdl");
#endif

	PRECACHE_SOUND("weapons/usp1.wav");
	PRECACHE_SOUND("weapons/usp2.wav");
	PRECACHE_SOUND("weapons/usp_unsil-1.wav");
	PRECACHE_SOUND("weapons/usp_clipout.wav");
	PRECACHE_SOUND("weapons/usp_clipin.wav");
	PRECACHE_SOUND("weapons/usp_silencer_on.wav");
	PRECACHE_SOUND("weapons/usp_silencer_off.wav");
	PRECACHE_SOUND("weapons/usp_sliderelease.wav");
	PRECACHE_SOUND("weapons/usp_slideback.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireUSP = PRECACHE_EVENT(1, "events/usp.sc");
}

int CUSP::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45acp";
	p->iMaxAmmo1 = MAX_AMMO_45ACP;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = USP_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_USP;
	p->iFlags = 0;
	p->iWeight = USP_WEIGHT;

	return 1;
}

BOOL CUSP::Deploy(void)
{
   m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
   m_flAccuracy = 0.92f;
   m_fMaxSpeed = 250.0f;
   m_pPlayer->m_bShieldDrawn = false;
   m_NextInspect = gpGlobals->time + 0.75s;
#ifdef ENABLE_SHIELD
   if (m_pPlayer->HasShield())
   {
      m_iWeaponState &= ~WPNSTATE_USP_SILENCED;
      return DefaultDeploy("models/shield/v_shield_usp.mdl", "models/shield/p_shield_usp.mdl", USP_SHIELD_DRAW, "shieldgun", UseDecrement() != FALSE);
   }
   else
#endif
   {
		if (m_iWeaponState & WPNSTATE_USP_SILENCED)
			return DefaultDeploy("models/v_usp.mdl", "models/p_usp.mdl", USP_DRAW, "onehanded", UseDecrement() != FALSE);
		else
			return DefaultDeploy("models/v_usp.mdl", "models/p_usp.mdl", USP_UNSIL_DRAW, "onehanded", UseDecrement() != FALSE);
   }
}

void CUSP::SecondaryAttack(void)
{
	if (ShieldSecondaryFire(USP_SHIELD_UP, USP_SHIELD_DOWN) == true)
		return;
	m_NextInspect = gpGlobals->time + 3s;
	if (m_iWeaponState & WPNSTATE_USP_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_USP_SILENCED;
		SendWeaponAnim(USP_DETACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "onehanded");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_USP_SILENCED;
		SendWeaponAnim(USP_ATTACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "onehanded");
	}
	
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3s;
}

void CUSP::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_USP_SILENCED)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			USPFire((1.3) * (1 - m_flAccuracy), 0.225s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			USPFire((0.25) * (1 - m_flAccuracy), 0.225s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			USPFire((0.125) * (1 - m_flAccuracy), 0.225s, FALSE);
		else
			USPFire((0.15) * (1 - m_flAccuracy), 0.225s, FALSE);
	}
	else
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			USPFire((1.2) * (1 - m_flAccuracy), 0.225s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			USPFire((0.225) * (1 - m_flAccuracy), 0.225s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			USPFire((0.08) * (1 - m_flAccuracy), 0.225s, FALSE);
		else
			USPFire((0.1) * (1 - m_flAccuracy), 0.225s, FALSE);
	}
}

void CUSP::USPFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.3 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.275;

		if (m_flAccuracy > 0.92)
			m_flAccuracy = 0.92;
		else if (m_flAccuracy < 0.6)
			m_flAccuracy = 0.6;
	}

	m_flLastFire = gpGlobals->time;
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
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	if (!(m_iWeaponState & WPNSTATE_USP_SILENCED))
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	int iDamage = (m_iWeaponState & WPNSTATE_USP_SILENCED) ? 30 : 34;
	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 4096, 1, BULLET_PLAYER_45ACP, iDamage, 0.79, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireUSP, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), 0, m_iClip != 0, m_iWeaponState & WPNSTATE_USP_SILENCED);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
	m_pPlayer->pev->punchangle.x -= 2;
	ResetPlayerShieldAnim();
}

void CUSP::Reload(void)
{
	if (m_pPlayer->ammo_45acp <= 0)
		return;

	int iAnim;

	if (m_pPlayer->HasShield() != false)
		iAnim = USP_SHIELD_RELOAD;
	else if (m_iWeaponState & WPNSTATE_USP_SILENCED)
		iAnim = USP_RELOAD;
	else
		iAnim = USP_UNSIL_RELOAD;
	m_NextInspect = gpGlobals->time + USP_RELOAD_TIME;
	if (DefaultReload(USP_MAX_CLIP, iAnim, 2.7s))
	{

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.92;
	}
}

void CUSP::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->HasShield())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			SendWeaponAnim(USP_DRAW, UseDecrement() != FALSE);

		return;
	}

	if (m_iClip)
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;

		if (m_iWeaponState & WPNSTATE_USP_SILENCED)
			SendWeaponAnim(USP_IDLE, UseDecrement() != FALSE);
		else
			SendWeaponAnim(USP_UNSIL_IDLE, UseDecrement() != FALSE);
	}
}


void CUSP::Inspect()
{

	if (!m_fInReload)
	{
		if (gpGlobals->time > m_NextInspect)
		{
#ifndef CLIENT_DLL
			if (m_iWeaponState & WPNSTATE_USP_SILENCED)
				SendWeaponAnim(16, 0);
			else
				SendWeaponAnim(17, 0);
#endif
			m_NextInspect = gpGlobals->time + GetInspectTime();
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + GetInspectTime();
		}
	}

}
}
