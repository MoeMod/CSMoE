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
#include "wpn_scarl.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_scarl, CScarLight)

const float SCARL_AMMO = 30;

void CScarLight::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_scarl");

	Precache();
	m_iId = WEAPON_GALIL;
	SET_MODEL(ENT(pev), "models/w_scar.mdl");

	m_iDefaultAmmo = SCARL_AMMO;

	FallInit();

	m_pLink = nullptr;
	m_iSwing = 0;
}

#ifndef CLIENT_DLL
void CScarLight::AttachToPlayer(CBasePlayer *pPlayer)
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
				if (!Q_stricmp(pItem->pszName(), "weapon_scarh"))
				{
					m_pLink = pItem;

					m_pLink->m_iSwing = 1;
					m_iSwing = 0;
				}

				pItem = pItem->m_pNext;
			}
		}
		if (!m_pLink)
			pPlayer->GiveNamedItem("weapon_scarh");

	}
}

void CScarLight::ItemPostFrame()
{
	m_iSwing = 0;
	if (m_pLink)
		m_pLink->m_iSwing = 1;
	return CBasePlayerWeapon::ItemPostFrame();
}

CScarLight::~CScarLight()
{
	CBaseEntity *pOther = m_pLink;
	if (pOther && pOther->m_pLink == this)
	{
		pOther->m_pLink = m_pLink = nullptr;
		pOther->SUB_Remove();
	}
}
#endif

void CScarLight::Precache(void)
{
	PRECACHE_MODEL("models/v_scar.mdl");
	PRECACHE_MODEL("models/p_scar.mdl");
	PRECACHE_MODEL("models/w_scar.mdl");

	PRECACHE_SOUND("weapons/scar_h.wav");
	PRECACHE_SOUND("weapons/scar_l.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireGalil = PRECACHE_EVENT(1, "events/scarl.sc");
}

int CScarLight::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556Nato";
	p->iMaxAmmo1 = MAX_AMMO_556NATO;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCARL_AMMO;
	p->iSlot = 0;
	p->iPosition = 17;
	p->iId = m_iId = WEAPON_GALIL;
	p->iFlags = 0;
	p->iWeight = 25;

	return 1;
}

BOOL CScarLight::Deploy(void)
{
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	iShellOn = 1;

	if (m_iSwing)
	{
		DefaultDeploy("models/v_scar.mdl", "models/p_scar.mdl", CHANGE_SCARL, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 6.0s;
	}
	else
	{
		DefaultDeploy("models/v_scar.mdl", "models/p_scar.mdl", SCARL_DRAW, "rifle", UseDecrement() != FALSE);
	}
	return TRUE;
}

void CScarLight::PrimaryAttack(void)
{
	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		GalilFire(0.035 + (0.4) * m_flAccuracy, 0.087s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		GalilFire(0.035 + (0.07) * m_flAccuracy, 0.087s, FALSE);
	else
		GalilFire((0.025) * m_flAccuracy, 0.087s, FALSE);
}

void CScarLight::GalilFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.25;

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
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireGalil, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 10000000), (int)(m_pPlayer->pev->punchangle.y * 10000000), FALSE, FALSE);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9s;

	if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.3, 0.45, 0.325, 0.05, 4.5, 2.5, 7);
	else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.5, 0.75, 0.35, 0.25, 7, 4, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.7, 0.325, 0.25, 0.015, 3.75, 2.5, 7);
	else
		KickBack(0.75, 0.375, 0.25, 0.02, 4.75, 2.75, 8);
}

float CScarLight::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() != DT_NONE)
		return 44;
#endif
	return 32;
}

void CScarLight::Reload(void)
{
	if (m_pPlayer->ammo_556nato <= 0)
		return;

	if (DefaultReload(SCARL_AMMO, SCARL_RELOAD, 3.05s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.2;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CScarLight::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(SCARL_IDLE1, UseDecrement() != FALSE);
}

}
