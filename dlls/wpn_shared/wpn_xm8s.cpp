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
#include "wpn_xm8s.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_xm8s, CXM8SharpShooter)

void CXM8SharpShooter::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_xm8s");

	Precache();
	m_iId = WEAPON_SG550;
	SET_MODEL(ENT(pev), "models/w_xm8.mdl");
	current_ammo = m_iDefaultAmmo = SG550_DEFAULT_GIVE;

	FallInit();

	m_pLink = nullptr;
	m_iSwing = 0;
}

#ifndef CLIENT_DLL
void CXM8SharpShooter::AttachToPlayer(CBasePlayer *pPlayer)
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
				if (!Q_stricmp(pItem->pszName(), "weapon_xm8c"))
				{
					m_pLink = pItem;

					m_pLink->m_iSwing = 1;
					m_iSwing = 0;
				}

				pItem = pItem->m_pNext;
			}
		}
		if (!m_pLink)
			pPlayer->GiveNamedItem("weapon_xm8c");

	}
}

void CXM8SharpShooter::ItemPostFrame()
{
	m_iSwing = 0;
	current_ammo = m_iClip;
	if (m_pLink)
		m_pLink->m_iSwing = 1;
	return CBasePlayerWeapon::ItemPostFrame();
}

CXM8SharpShooter::~CXM8SharpShooter()
{
	CBaseEntity *pOther = m_pLink;
	if (pOther && pOther->m_pLink == this)
	{
		pOther->m_pLink = m_pLink = nullptr;
		pOther->SUB_Remove();
	}
}
#endif

void CXM8SharpShooter::Precache(void)
{
	PRECACHE_MODEL("models/v_xm8.mdl");
	PRECACHE_MODEL("models/w_xm8.mdl");
	PRECACHE_MODEL("models/p_xm8.mdl");

	PRECACHE_SOUND("weapons/xm8_shooter.wav");
	PRECACHE_SOUND("weapons/zoom.wav");

	m_iShellId = m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFireSG550 = PRECACHE_EVENT(1, "events/xm8s.sc");
}

BOOL CXM8SharpShooter::Deploy(void)
{
	m_flAccuracy = 0.2;

	if (m_iSwing)
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", XM8_CHANGE_SHARPSHOOTER, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack =  6.0s;
		// sync ammo
		if(m_pLink)
			m_iClip = m_pLink->current_ammo;
	}
	else
	{
		DefaultDeploy("models/v_xm8.mdl", "models/p_xm8.mdl", SHARPSHOOTER_XM8_DRAW, "rifle", UseDecrement() != FALSE);
		m_pPlayer->m_flNextAttack = 1.5s;
	}
	return TRUE;
}

void CXM8SharpShooter::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		XM8SharpShooterFire((0.65) * (1 - m_flAccuracy), 0.375s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		XM8SharpShooterFire((0.25) * (1 - m_flAccuracy), 0.25s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 10)
		XM8SharpShooterFire((0.1) * (1 - m_flAccuracy), 0.25s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		XM8SharpShooterFire((0.01), 0.375s, FALSE);
	else
		XM8SharpShooterFire((0.03) * (1 - m_flAccuracy), 0.375s, FALSE);
}

void CXM8SharpShooter::XM8SharpShooterFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	if (m_pPlayer->pev->fov == 90)
		flSpread += 0.08;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.3 + 0.55;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
	}

	m_flLastFire = gpGlobals->time;

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

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.55;
	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	Vector vecDir = FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, 70, 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSG550, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 0.8, 1.3) + m_pPlayer->pev->punchangle.x * 0.25;
	m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.85, -0.85);
}

float CXM8SharpShooter::GetDamage()
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() != DT_NONE)
		return 66;
#endif
	return 50;
}

float CXM8SharpShooter::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 210;

	return 160;
}

}
