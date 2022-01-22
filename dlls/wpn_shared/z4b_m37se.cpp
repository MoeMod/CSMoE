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
#include "z4b_m37se.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum m37se_e
{
	M37SE_IDLE,
	M37SE_FIRE1,
	M37SE_FIRE2,
	M37SE_RELOAD,
	M37SE_PUMP,
	M37SE_START_RELOAD,
	M37SE_DRAW,
};

LINK_ENTITY_TO_CLASS(z4b_m37se, CM37se)

void CM37se::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_m37se");

	Precache();
	m_iId = WEAPON_M3;
	SET_MODEL(ENT(pev), "models/w_m37se.mdl");
	m_iDefaultAmmo = 8;

	FallInit();
}

void CM37se::Precache(void)
{
	PRECACHE_MODEL("models/v_m37se.mdl");
	PRECACHE_MODEL("models/p_m37se.mdl");
	PRECACHE_MODEL("models/w_m37se.mdl");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("CK_WPN/WEAPON/NANOM37/G_SHOOT_NANOM37.wav");
	PRECACHE_SOUND("CK_WPN/WEAPON/NANOM37/NANOM37_CLIPIN.WAV");
	PRECACHE_SOUND("CK_WPN/WEAPON/NANOM37/NANOM37_RELOAD.WAV");
	PRECACHE_SOUND("CK_WPN/WEAPON/NANOM37/NANOM37_SELECT.WAV");

	m_usFireM37se = PRECACHE_EVENT(1, "addons/luash/events/event_m37se.lua");
}

int CM37se::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = 32;
	p->pszAmmo2 = "HaveyAmmo";
	p->iMaxAmmo2 = 0;
	p->iMaxClip = 8;
	p->iSlot = 0;
	p->iPosition = 5;
	p->iId = m_iId = WEAPON_M3;
	p->iFlags = 0;
	p->iWeight = M3_WEIGHT;

	return 1;
}

BOOL CM37se::Deploy(void)
{
	return DefaultDeploy("models/v_m37se.mdl", "models/p_m37se.mdl", M37SE_DRAW, "shotgun", UseDecrement() != FALSE);
}

void CM37se::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0 && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
	{
		Reload();

		if (m_iClip == 0)
			PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}


	float flSpread;
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		flSpread = 0.85;
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		flSpread = 0.015;
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		flSpread = 0.0;
	else
		flSpread = 0.01;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	{

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
#ifndef CLIENT_DLL
		pev->iuser1 = 1;
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_338MAG, 115, 0.96, m_pPlayer->pev, FALSE, 0);
		pev->iuser1 = 0;
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFireM37se, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, TRUE);
#endif
	}
	else
	{
		m_iClip--;
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0);
		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFireM37se, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);
#endif
	}
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	else
		m_flTimeWeaponIdle = 0.875s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 11);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45s;
}

void CM37se::SecondaryAttack(void)
{
	if (!m_iClip)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase() || m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
		return;

	SetThink(&CM37se::SlotReload);
	SendWeaponAnim(M37SE_START_RELOAD, UseDecrement() != FALSE);

	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
	m_fInReloadSlot = 1;
	m_fInSpecialReload = 0;
	pev->nextthink = gpGlobals->time + 0.425s;

}

void CM37se::SlotReload(void)
{

	if (m_fInReloadSlot)
	{
		SendWeaponAnim(M37SE_RELOAD, UseDecrement() != FALSE);
		m_flNextReload = UTIL_WeaponTimeBase() + 0.54s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.54s;
		SetThink(&CM37se::SlotReload);
		m_fInReloadSlot = 0;
		pev->nextthink = gpGlobals->time + 0.54s;
	}
	else
	{
		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
			m_iClip++;
		}
		else
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
			m_iClip--;
		}
		SendWeaponAnim(M37SE_PUMP, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	}


}

void CM37se::Reload(void)
{

	if (m_iClip == 7 && m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == 8 )
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(M37SE_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.425s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.425s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.425s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.425s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;
		SendWeaponAnim(M37SE_RELOAD, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.54s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.54s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
		m_pPlayer->ammo_buckshot--;
	}

}

void CM37se::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = zero_duration;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			if (m_iClip + m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 0)
				Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != 8 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{				
				if (m_iClip + m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] == 8)
				{
					SendWeaponAnim(M37SE_PUMP, UseDecrement() != FALSE);
					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
				}
				else
					Reload();
			}
			else
			{
				SendWeaponAnim(M37SE_PUMP, UseDecrement() != FALSE);
				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
			}
		}
		else
			SendWeaponAnim(M37SE_IDLE, UseDecrement() != FALSE);
	}

}

}
