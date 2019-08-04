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
#include "wpn_kriss.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum kriss_e
{
	KRISS_IDLE,
	KRISS_SHOOT1,
	KRISS_SHOOT2,
	KRISS_SHOOT3,
	KRISS_RELOAD,
	KRISS_DRAW,
	KRISS_ATTACH_SILENCER,
	KRISS_UNSIL_IDLE,
	KRISS_UNSIL_SHOOT1,
	KRISS_UNSIL_SHOOT2,
	KRISS_UNSIL_SHOOT3,
	KRISS_UNSIL_RELOAD,
	KRISS_UNSIL_DRAW,
	KRISS_DETACH_SILENCER
};

LINK_ENTITY_TO_CLASS(weapon_kriss, CKRISS)

const int KRISS_MAXCLIP = 30;

void CKRISS::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_kriss");

	Precache();
	m_iId = WEAPON_UMP45;
	SET_MODEL(ENT(pev), "models/w_kriss.mdl");

	m_iDefaultAmmo = KRISS_MAXCLIP;
	m_flAccuracy = 0;
	m_iShotsFired = 0;
	m_bDelayFire = true;

	FallInit();
}

void CKRISS::Precache(void)
{
	PRECACHE_MODEL("models/v_kriss.mdl");
	PRECACHE_MODEL("models/p_kriss.mdl");
	PRECACHE_MODEL("models/w_kriss.mdl");

	PRECACHE_SOUND("weapons/kriss-1.wav");
	PRECACHE_SOUND("weapons/kriss_sil-1.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireKRISS = PRECACHE_EVENT(1, "events/kriss.sc");
}

int CKRISS::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "45acp";
	p->iMaxAmmo1 = MAX_AMMO_45ACP;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = KRISS_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 15;
	p->iId = m_iId = WEAPON_UMP45;
	p->iFlags = 0;
	p->iWeight = UMP45_WEIGHT;

	return 1;
}

BOOL CKRISS::Deploy(void)
{
	m_bDelayFire = true;
	iShellOn = 1;
	m_flAccuracy = 0;
	m_iShotsFired = 0;

	if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
		return DefaultDeploy("models/v_kriss.mdl", "models/p_kriss.mdl", KRISS_DRAW, "carbine", UseDecrement() != FALSE);
	else
		return DefaultDeploy("models/v_kriss.mdl", "models/p_kriss.mdl", KRISS_UNSIL_DRAW, "carbine", UseDecrement() != FALSE);
}

void CKRISS::SecondaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
	{
		m_iWeaponState &= ~WPNSTATE_KRISS_SILENCED;
		SendWeaponAnim(KRISS_DETACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "carbine");
	}
	else
	{
		m_iWeaponState |= WPNSTATE_KRISS_SILENCED;
		SendWeaponAnim(KRISS_ATTACH_SILENCER, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "carbine");
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.5s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.5s;
}

void CKRISS::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KRISSFire(0.09 * m_flAccuracy, KRISS_FIRE_RATE, FALSE);
	else
	{
		if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
			KRISSFire(0.06 * m_flAccuracy, KRISS_FIRE_RATE, FALSE);
		else
			KRISSFire(0.05 * m_flAccuracy, KRISS_FIRE_RATE, FALSE);
	}
}

void CKRISS::KRISSFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired) / 250) + 0.3;

	if (m_flAccuracy > 1.4)
		m_flAccuracy = 1.4;

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
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, KRISS_DAMAGE_SIL, KRISS_RANGE_MODIFER_SIL, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
	}
	else
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, KRISS_DAMAGE, KRISS_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	}

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireKRISS, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), (m_iWeaponState & WPNSTATE_KRISS_SILENCED) ? true : false, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.55, 0.18, 0.23, 0.03, 2.5, 1.2, 5);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.62, 0.6, 0.5, 0.04, 5, 3, 10);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.4, 0.14, 0.13, 0.02, 1.8, 0.8, 10);
	else
		KickBack(0.52, 0.16, 0.2, 0.02, 2, 1, 10);
}

void CKRISS::Reload(void)
{
	if (m_pPlayer->ammo_45acp <= 0)
		return;

	int iAnim;

	if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
		iAnim = KRISS_RELOAD;
	else
		iAnim = KRISS_UNSIL_RELOAD;

	if (DefaultReload(KRISS_MAXCLIP, iAnim, KRISS_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CKRISS::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

	if (m_iWeaponState & WPNSTATE_KRISS_SILENCED)
		SendWeaponAnim(KRISS_IDLE, UseDecrement() != FALSE);
	else
		SendWeaponAnim(KRISS_UNSIL_IDLE, UseDecrement() != FALSE);
}

}
