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
#include "wpn_ozwpnset1.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#include "gamemode/mod_zb1.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_ozwpnset1, COZWpnSet1)

void COZWpnSet1::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_ozwpnset1");

	Precache();
	m_iId = WEAPON_OZWPNSET1;
	SET_MODEL(ENT(pev), "models/w_ozwpnset1.mdl");
	m_tNextPlayEvent = invalid_time_point;
	m_iDefaultAmmo = 100;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;

	FallInit();
}

void COZWpnSet1::Precache(void)
{
	PRECACHE_MODEL("models/v_ozwpnset1.mdl");
	PRECACHE_MODEL("models/w_ozwpnset1.mdl");
	PRECACHE_MODEL("models/p_ozwpnset1.mdl");

	PRECACHE_SOUND("weapons/ozwpnset1-1.wav");

	m_iShell = PRECACHE_MODEL("models/rshell.mdl");
	m_usFire = PRECACHE_EVENT(1, "events/ozwpnset1.sc");
}

int COZWpnSet1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "556NatoBox";
	p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 100;
	p->iSlot = 0;
	p->iPosition = 4;
	p->iId = m_iId = WEAPON_OZWPNSET1;
	p->iFlags = 0;
	p->iWeight = M249_WEIGHT;

	return 1;
}

BOOL COZWpnSet1::Deploy(void)
{
	m_flNextReload = invalid_time_point;
	m_flAccuracy = 0.2;
	m_iShotsFired = 0;
	m_tNextPlayEvent = gpGlobals->time;

	return DefaultDeploy("models/v_ozwpnset1.mdl", "models/p_ozwpnset1.mdl", ANIM_DRAW, "m134", UseDecrement() != FALSE);
}

void COZWpnSet1::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		OzWpnSet1Fire(0.045 + 0.5 * m_flAccuracy, 0.12s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 140)
		OzWpnSet1Fire(0.045 + 0.095 * m_flAccuracy, 0.12s, FALSE);
	else
		OzWpnSet1Fire(0.0275 * m_flAccuracy, 0.12s, FALSE);
	
}

void COZWpnSet1::OzWpnSet1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175.0) + 0.35;

	if (m_flAccuracy > 0.9)
		m_flAccuracy = 0.9;

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
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.65, 0.45, 0.125, 5, 3.5, 8);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1, 0.4, 0.3, 0.06, 4, 3, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.65, 0.125, 0.25, 0.025, 3.5, 2.5, 9);
	else
		KickBack(0.8, 0.2, 0.25, 0.038, 3.75, 3, 10);
	
	
}

void COZWpnSet1::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

#ifndef CLIENT_DLL
	if (m_iClip >= GetMaxClip())
		return;
#endif

	if (DefaultReload(GetReloadClip(), ANIM_RELOAD, 1.1s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

		m_flNextReload = gpGlobals->time + 1.1s;
		m_flAccuracy = 0.2;
		m_bDelayFire = false;
		m_iShotsFired = 0;
	}
}

void COZWpnSet1::ItemPostFrame(void)
{
	if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
	{
		// complete the reload.
		int j = Q_min(Q_min(GetMaxClip() - m_iClip, GetReloadClip()), m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#ifndef CLIENT_DLL
		j = m_pPlayer->m_pModStrategy->AdjustDefaultReloadClip(this, j);
#endif

		// Add them to the clip
		m_iClip += j;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#ifndef CLIENT_DLL
		m_pPlayer->TabulateAmmo();
#endif
		m_fInReload = FALSE;
		m_flNextReload = invalid_time_point;
	}

	if (m_fInReload)
		return;

	return CBasePlayerWeapon::ItemPostFrame();
}

void COZWpnSet1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

float COZWpnSet1::GetDamage() const
{
	float flDamage = 37.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 80.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 550.0f;
#endif
	return flDamage;
}

KnockbackData COZWpnSet1::GetKnockBackData()
{
	return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };
}

float COZWpnSet1::GetMaxSpeed(void)
{
	return 210;
}

}
