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
#include "wpn_xm8c.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

enum xm8_e
{
	CARBINE_XM8_IDLE1,
	CARBINE_XM8_RELOAD,
	CARBINE_XM8_DRAW,
	CARBINE_XM8_SHOOT1,
	CARBINE_XM8_SHOOT2,
	CARBINE_XM8_SHOOT3,
	XM8_CHANGE_SHARPSHOOTER,
	SHARPSHOOTER_XM8_IDLE1,
	SHARPSHOOTER_XM8_RELOAD,
	SHARPSHOOTER_XM8_DRAW,
	SHARPSHOOTER_XM8_SHOOT1,
	SHARPSHOOTER_XM8_SHOOT2,
	SHARPSHOOTER_XM8_SHOOT3,
	XM8_CHANGE_CARBINE,
};

LINK_ENTITY_TO_CLASS(weapon_xm8c, CXM8Carbine)

void CXM8Carbine::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_xm8c");

	Precache();
	m_iId = WEAPON_SG552;
	SET_MODEL(ENT(pev), "models/w_xm8.mdl");

	m_iDefaultAmmo = SG552_DEFAULT_GIVE;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();

	m_pLink = nullptr;
	m_iSwing = 0;
}

#ifndef CLIENT_DLL
void CXM8Carbine::AttachToPlayer(CBasePlayer *pPlayer)
{
	CBasePlayerWeapon::AttachToPlayer(pPlayer);

	// find the linked weapon
	while (!m_pLink)
	{
		for (int i = 0; i < MAX_ITEM_TYPES; ++i)
		{
			CBasePlayerItem *pItem = m_pPlayer->m_rgpPlayerItems[i];
			while (pItem != NULL)
			{
				if (!Q_stricmp(pItem->pszName(), "weapon_xm8s"))
				{
					m_pLink = pItem;

					m_pLink->m_iSwing = 1;
					m_iSwing = 0;
				}

				pItem = pItem->m_pNext;
			}
		}
		if(!m_pLink)
			pPlayer->GiveNamedItem("weapon_xm8s");

	} 
}

void CXM8Carbine::ItemPostFrame()
{
	m_iSwing = 0;
	current_ammo = m_iClip;
	if (m_pLink)
		m_pLink->m_iSwing = 1;
	return CBasePlayerWeapon::ItemPostFrame();
}
#endif

void CXM8Carbine::Precache(void)
{
	PRECACHE_MODEL("models/v_xm8.mdl");
	PRECACHE_MODEL("models/w_xm8.mdl");
	PRECACHE_MODEL("models/p_xm8.mdl");

	PRECACHE_SOUND("weapons/xm8_carbine.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireSG552 = PRECACHE_EVENT(1, "events/xm8c.sc");
}

int CXM8Carbine::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SG552_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 10;
	p->iId = m_iId = WEAPON_SG552;
	p->iFlags = 0;
	p->iWeight = 25;

	return 1;
}

BOOL CXM8Carbine::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	if (m_iSwing)
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", XM8_CHANGE_CARBINE, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 6.0f;
		// sync ammo
		if (m_pLink)
			m_iClip = m_pLink->current_ammo;
	}
	else
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", CARBINE_XM8_DRAW, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 1.5f;
	}
	return TRUE;
}

void CXM8Carbine::SecondaryAttack(void)
{
	if (m_pPlayer->m_iFOV != 90)
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
	else
		m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 55;

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3;
}

void CXM8Carbine::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		XM8CarbineFire(0.04 + (0.4) * m_flAccuracy, 0.0955, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		XM8CarbineFire(0.04 + (0.07) * m_flAccuracy, 0.0955, FALSE);
	else if (m_pPlayer->pev->fov == 90)
		XM8CarbineFire((0.0275) * m_flAccuracy, 0.0955, FALSE);
	else
		XM8CarbineFire((0.0275) * m_flAccuracy, 0.12, FALSE);
}

void CXM8Carbine::XM8CarbineFire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
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
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.955, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSG552, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.425, 0.44999999, 0.2, 0.045000002, 6.25, 2.5, (signed int)7.0);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.825, 1.2, 0.40000001, 0.30000001, 8.5, 6.125, (signed int)5.0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.82499999, 0.32499999, 0.12, 0.0275, 5.125, 1.5, (signed int)8.0);
	else
		KickBack(0.85000002, 0.35499999, 0.18000001, 0.035999998, 5.25, 1.85, (signed int)7.0);
}

float CXM8Carbine::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() != DT_NONE)
		return 48;
#endif
	return 32;
}

void CXM8Carbine::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(SG552_MAX_CLIP, CARBINE_XM8_RELOAD, 3.2))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		if (m_pPlayer->m_iFOV != 90)
			SecondaryAttack();

		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CXM8Carbine::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(CARBINE_XM8_IDLE1, UseDecrement() != FALSE);
}

float CXM8Carbine::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 235;

	return 200;
}
