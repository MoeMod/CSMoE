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
#include "z4b_frequency1.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(z4b_frequency1, CFrequency1)

void CFrequency1::Spawn(void)
{
	pev->classname = MAKE_STRING("z4b_frequency1");

	Precache();
	m_iId = WEAPON_DEAGLE;
	SET_MODEL(ENT(pev), "models/z4b/pw_mccree.mdl");

	m_iDefaultAmmo = FREQUENCY1_MAX_CLIP;
	m_flAccuracy = 0.9;
	m_iDefaultAmmo2 = 0;
	m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
	m_fMaxSpeed = 250;

	FallInit();
}

void CFrequency1::Precache(void)
{
	PRECACHE_MODEL("models/z4b/pw_mccree.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_deagle.mdl");
#endif
	PRECACHE_MODEL("models/z4b/v_mccree.mdl");

	PRECACHE_SOUND("weapons/skull1-1.wav");
	PRECACHE_SOUND("weapons/skull1-2.wav");
	PRECACHE_SOUND("z4/z4_mccree_skill.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_iSprBeam = PRECACHE_MODEL("sprites/laserbeam.spr");
	m_usFireFrequency1 = PRECACHE_EVENT(1, "events/frequency1.sc");
}

int CFrequency1::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "50AE";
	p->iMaxAmmo1 = MAX_AMMO_50AE;
	p->pszAmmo2 = "SVDEXGrenade";
	p->iMaxAmmo2 = -1;
	p->iMaxClip = FREQUENCY1_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DEAGLE;
	p->iFlags = 0;
	p->iWeight = DEAGLE_WEIGHT;

	return 1;
}

int CFrequency1::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CFrequency1::Deploy(void)
{
	m_flAccuracy = 0.9;
	m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
	m_pPlayer->m_bShieldDrawn = false;
	m_fMaxSpeed = 250;
	return DefaultDeploy("models/z4b/v_mccree.mdl", "models/z4b/pw_mccree.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CFrequency1::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Frequency1Fire((1.5) * (1 - m_flAccuracy), 0.5s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Frequency1Fire((0.25) * (1 - m_flAccuracy), 0.5s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Frequency1Fire((0.115) * (1 - m_flAccuracy), 0.5s, FALSE);
	else
		Frequency1Fire((0.13) * (1 - m_flAccuracy), 0.5s, FALSE);
}

void CFrequency1::SecondaryAttack(void)
{
	if (m_iWeaponState & ~WPNSTATE_GLOCK18_BURST_MODE)
	{

	}
	m_iWeaponState |= WPNSTATE_GLOCK18_BURST_MODE;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		Frequency1FireSP((1.5) * (1 - m_flAccuracy), 0.17s, FALSE);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		Frequency1FireSP((0.25) * (1 - m_flAccuracy), 0.17s, FALSE);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		Frequency1FireSP((0.115) * (1 - m_flAccuracy), 0.17s, FALSE);
	else
		Frequency1FireSP((0.13) * (1 - m_flAccuracy), 0.17s, FALSE);
}


void CFrequency1::Frequency1FireSP(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.0)
			m_flAccuracy = 0.0;
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

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 10, BULLET_PLAYER_50AE, GetDamage2(), 0.89, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireFrequency1, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(1.8, 0.75, 0.35, 0.075, 6.5, 3.5, 6);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(2.0, 0.9, 0.5, 0.1, 7.0, 5.0, 5);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(1.5, 0.7, 0.3, 0.05, 6.0, 3.0, 7);
	else
		KickBack(3.0, 2.0, 1.0, 0.7, 8.0, 12.0, 4);

#if 0
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 2048;
	Effect_Beam(vecSrc, vecEnd, 7, 50, 75, 150, 150, 4, 998);
#endif
}

void CFrequency1::Frequency1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
{
	flCycleTime -= 0.075s;
	m_iShotsFired++;

	if (m_iShotsFired > 1)
		return;

	if (m_flLastFire != invalid_time_point)
	{
		m_flAccuracy -= (0.4 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.35;

		if (m_flAccuracy > 0.9)
			m_flAccuracy = 0.9;
		else if (m_flAccuracy < 0.55)
			m_flAccuracy = 0.55;
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
	SetPlayerShieldAnim();
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 4096, 10, BULLET_PLAYER_50AE, GetDamage1(), 0.89, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireFrequency1, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, TRUE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;
	m_pPlayer->pev->punchangle.x -= 2;
#if 0
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 2048;
	Effect_Beam(vecSrc, vecEnd, 7, 50, 75, 150, 150, 4, 998);
#endif

}

#ifndef CLIENT_DLL
void CFrequency1::Effect_Beam(Vector vecStart, Vector vecEnd, int iLife, int iBrightness, int r, int g, int b, int iWidth, int iScollSpeed)
{
	MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
	WRITE_BYTE(TE_BEAMPOINTS);
	WRITE_COORD(vecStart.x);
	WRITE_COORD(vecStart.y);
	WRITE_COORD(vecStart.z);
	WRITE_COORD(vecEnd.x);
	WRITE_COORD(vecEnd.y);
	WRITE_COORD(vecEnd.z); // coord coord coord (end position) 
	WRITE_SHORT(m_iSprBeam); // short (sprite index) 
	WRITE_BYTE(0); // byte (starting frame) 
	WRITE_BYTE(1); // byte (frame rate in 0.1's) 
	WRITE_BYTE(iLife); // byte (life in 0.1's) 
	WRITE_BYTE(iWidth); // byte (line width in 0.1's) 
	WRITE_BYTE(0); // byte (noise amplitude in 0.01's) 
	WRITE_BYTE(r); // byte,byte,byte (color)
	WRITE_BYTE(g);
	WRITE_BYTE(b);
	WRITE_BYTE(iBrightness); // byte (brightness)
	WRITE_BYTE(iScollSpeed); // byte (scroll speed in 0.1's)
	MESSAGE_END();
}
#endif

void CFrequency1::Reload(void)
{
	if (m_pPlayer->ammo_50ae <= 0)
		return;

	if (DefaultReload(FREQUENCY1_MAX_CLIP, ANIM_RELOAD, FREQUENCY1_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.9;
		m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
	}
}

void CFrequency1::ItemPostFrame()
{
	return CBasePlayerWeapon::ItemPostFrame();
}

void CFrequency1::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
}

float CFrequency1::GetDamage1() const
{
	float flDamage = 56.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 280.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 350.0f;
#endif
	return flDamage;
}

float CFrequency1::GetDamage2() const
{
	float flDamage = 40.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 190.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 280.0f;
#endif
	return flDamage;
}

float CFrequency1::GetDamage3() const
{
	float flDamage = 56.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 280.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 350.0f;
#endif
	return flDamage;
}

}
