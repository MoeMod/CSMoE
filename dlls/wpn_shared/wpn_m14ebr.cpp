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
#include "wpn_m14ebr.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_m14ebr, CM14EBR)

const int M14EBR_MAXCLIP = 20;

void CM14EBR::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_m14ebr");

	Precache();
	m_iId = WEAPON_AK47;
	SET_MODEL(ENT(pev), "models/w_m14ebr.mdl");

	m_iDefaultAmmo = M14EBR_MAXCLIP;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void CM14EBR::Precache(void)
{
	PRECACHE_MODEL("models/v_m14ebr.mdl");
	PRECACHE_MODEL("models/w_m14ebr.mdl");
	PRECACHE_MODEL("models/p_m14ebr.mdl");

	PRECACHE_SOUND("weapons/m14ebr-1.wav");
	PRECACHE_SOUND("weapons/m14_clipout.wav");
	PRECACHE_SOUND("weapons/m14_clipin.wav");
	PRECACHE_SOUND("weapons/m14_draw.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireM14EBR = PRECACHE_EVENT(1, "events/m14ebr.sc");
}

int CM14EBR::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "762Nato";
	p->iMaxAmmo1 = MAX_AMMO_762NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = M14EBR_MAXCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_AK47;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

BOOL CM14EBR::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	return DefaultDeploy("models/v_m14ebr.mdl", "models/p_m14ebr.mdl", M14EBR_DRAW, "m14ebr", UseDecrement() != FALSE);
}

void CM14EBR::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		M14EBRFire(0.0325 + (0.4) * m_flAccuracy, 0.0955s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		M14EBRFire(0.0325 + (0.07) * m_flAccuracy, 0.0955s, FALSE);
	else
		M14EBRFire((0.0225), 0.0955s, FALSE);
}

void CM14EBR::M14EBRFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;

	if (m_flAccuracy > 1.25)
		m_flAccuracy = 1.25;

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
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, 46, 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireM14EBR, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(2.0, 1.0, 0.6, 0.35, 9.0, 6.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.8, 0.3, 0.125, 0.02, 5.0, 1.35, 9);
	else
		KickBack(1.1, 0.33, 0.22, 0.038, 5.9, 1.9, 8);
}

void CM14EBR::Reload(void)
{
	if (m_pPlayer->ammo_762nato <= 0)
		return;

	if (DefaultReload(M14EBR_MAXCLIP, M14EBR_RELOAD, 2.45s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CM14EBR::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(M14EBR_IDLE1, UseDecrement() != FALSE);
}

}
