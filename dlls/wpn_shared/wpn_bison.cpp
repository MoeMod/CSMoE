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
#include "wpn_bison.h"

#ifndef CLIENT_DLL
#include "common/netadr.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_bison, CBison)

void CBison::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_bison");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_bison.mdl");

	m_iDefaultAmmo = BISON_MAX_CLIP;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	FallInit();
}

void CBison::Precache(void)
{
	PRECACHE_MODEL("models/v_bison.mdl");
	PRECACHE_MODEL("models/w_bison.mdl");
	PRECACHE_MODEL("models/p_bison.mdl");

	PRECACHE_SOUND("weapons/bison-1.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFireBison = PRECACHE_EVENT(1, "events/bison.sc");
}

int CBison::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "9mm";
	p->iMaxAmmo1 = 120;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = BISON_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 8;
	p->iId = m_iId = WEAPON_P90;
	p->iFlags = 0;
	p->iWeight = P90_WEIGHT;

	return 1;
}

BOOL CBison::Deploy(void)
{
	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_flAccuracy = 0;

	return DefaultDeploy("models/v_bison.mdl", "models/p_bison.mdl", BIZON_DRAW, "mp5", UseDecrement() != FALSE);
}

void CBison::PrimaryAttack(void)
{
	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		BisonFire((0.2) * m_flAccuracy, 0.07s, FALSE);
	else 
		BisonFire((0.05) * m_flAccuracy, 0.07s, FALSE);
}

void CBison::BisonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired) / 200) + 0.55;

	if (m_flAccuracy > 0.85)
		m_flAccuracy = 0.85;

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), BISON_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireBison, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.9, 0.475, 0.35, 0.0425, 5.0, 3.0, 6);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.5, 0.275, 0.2, 0.03, 3.0, 2.0, 10);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.225, 0.15, 0.1, 0.015, 2.0, 1.0, 10);
	else
		KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.25, 10);
}

void CBison::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(BISON_MAX_CLIP, BIZON_RELOAD, BISON_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0;
		m_iShotsFired = 0;
	}
}

void CBison::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BIZON_IDLE1, UseDecrement() != FALSE);
}

float CBison::GetDamage()
{
	float flDamage = 24.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 65.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 75.0f;
#endif
	return flDamage;
}
}
