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
#include "wpn_laserfist.h"

#ifndef CLIENT_DLL
#include "common/netadr.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_laserfist, CLaserFist)

void CLaserFist::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_laserfist");

	Precache();
	m_iId = WEAPON_P90;
	SET_MODEL(ENT(pev), "models/w_laserfist.mdl");

	m_iDefaultAmmo = 500;
	m_flAccuracy = 0;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	FallInit();
}

void CLaserFist::Precache(void)
{
	PRECACHE_MODEL("models/v_laserfist.mdl");
	PRECACHE_MODEL("models/w_laserfist.mdl");
	PRECACHE_MODEL("models/p_laserfist.mdl");

	PRECACHE_SOUND("weapons/laserfist-1.wav");

	m_iShell = PRECACHE_MODEL("models/pshell.mdl");
	m_usFirelaserfist = PRECACHE_EVENT(1, "events/laserfist.sc");
}

int CLaserFist::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "LaserBeam";
	p->iMaxAmmo1 = 500;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 500;
	p->iSlot = 0;
	p->iPosition = 8;
	p->iId = m_iId = WEAPON_LASERFIST;
	p->iFlags = 0;
	p->iWeight = P90_WEIGHT;

	return 1;
}

BOOL CLaserFist::Deploy(void)
{
	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_flAccuracy = 0;

	return DefaultDeploy("models/v_laserfist.mdl", "models/p_laserfist.mdl", ANIM_DRAW, "mp5", UseDecrement() != FALSE);
}

void CLaserFist::PrimaryAttack(void)
{
	m_iChargeCount++;
	pev->iuser2 = m_iClip;	//iuser2 ClipBeforeShoot
	if (m_iClip < 7)
		m_iDecreasdShoot = m_iClip - 1;
	else
		m_iDecreasdShoot = 7;

	if (!(m_pPlayer->pev->flags & FL_ONGROUND))
		LaserFistFire(0.3 * m_flAccuracy, FALSE);
	else
		LaserFistFire(0.03 * m_flAccuracy, FALSE);

	m_flNextShootTime = gpGlobals->time + 0.1s;
	m_Clip = iClipBeforeShoot - 1 - m_iDecreasdShoot;	//actually cost 
	pev->iuser1 = 1;	//mark
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;
}

void CLaserFist::SecondaryAttack
{

}

void CLaserFist::ItemPostFrame(void)
{
	if (pev->iuser1)
	{
		pev->iuser1 = 0;
		m_iClip = pev->iuser2;
	}

	if (m_flGlock18Shoot != invalid_time_point)
	{
#ifndef CLIENT_DLL
		int iClipBeforeAttack = m_iClip;
		FireRemaining(m_iGlock18ShotsFired, m_flGlock18Shoot, TRUE);
		m_pPlayer->m_pModStrategy->OnAttack_Post(this, iClipBeforeAttack, m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#else
		m_iClip--;
		if (m_iClip < 0)
		{
			m_iClip = m_iGlock18ShotsFired = 0;
		}
		FireRemaining(m_iGlock18ShotsFired, m_flGlock18Shoot, TRUE);
#endif
	}


	if (m_iDecreasdShoot && m_flNextShootTime != invalid_time_point && m_flNextShootTime <= gpGlobals->time)
	{
		if (!(m_pPlayer->pev->flags & FL_ONGROUND))
			LaserFistFire(0.3 * m_flAccuracy, FALSE);
		else
			LaserFistFire(0.03 * m_flAccuracy, FALSE);

		m_iDecreasdShoot--;
		m_flNextShootTime = gpGlobals->time + 0.1s;
	}

}

void CLaserFist::FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL bIsGlock)
{
	auto nexttime = 0.1s;

	m_iClip--;

	if (m_iClip < 0)
	{
		m_iClip = 0;
		shotsFired = 3;
		shootTime = invalid_time_point;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir;

	int flag;
#ifdef CLIENT_WEAPONS
	flag = FEV_NOTHOST;
#else
	flag = 0;
#endif // CLIENT_WEAPONS

	if (bIsGlock)
	{
		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, 0.05, 8192, 1, BULLET_PLAYER_9MM, 18, 0.9, m_pPlayer->pev, true, m_pPlayer->random_seed);
		--m_pPlayer->ammo_9mm;

		PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireGlock18, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y,
			(int)(m_pPlayer->pev->punchangle.x * 10000), (int)(m_pPlayer->pev->punchangle.y * 10000), m_iClip == 0, FALSE);
	}
	else
	{

		vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, 30, 0.96, m_pPlayer->pev, false, m_pPlayer->random_seed);
		--m_pPlayer->ammo_556nato;

		PLAYBACK_EVENT_FULL(flag, m_pPlayer->edict(), m_usFireFamas, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y,
			(int)(m_pPlayer->pev->punchangle.x * 10000000), (int)(m_pPlayer->pev->punchangle.y * 10000000), FALSE, FALSE);
	}

	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	shotsFired++;

	if (shotsFired != 3)
	{
		shootTime = gpGlobals->time + nexttime;
	}
	else
		shootTime = invalid_time_point;
}

void CLaserFist::LaserFistFire(float flSpread, BOOL fUseAutoAim)
{
	m_iChargeCount++;
	pev->iuser = 1;
	m_bDelayFire = true;
	m_iShotsFired++;
	m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired) / 200) + 0.2;

	if (m_flAccuracy > 0.8)
		m_flAccuracy = 0.8;

	if (m_iClip <= 0)
	{
		m_iDecreasdShoot = 0;
		m_flNextShootTime = invalid_time_point;
		pev->iuser1 = 0;
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
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
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), laserfist_RANGE_MODIFER, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFirelaserfist, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
	
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(0.3, 0.21, 0.03, 0.021, 3.0, 10.0, 0);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(0.15, 0.1, 0.015, 0.01, 1.5, 5.5, 0);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.08, 0.05, 0.008, 0.005, 0.8, 4.8, 0);
	else
		KickBack(0.1, 0.07, 0.01, 0.007, 1, 5, 0);
}

void CLaserFist::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(laserfist_MAX_CLIP, BIZON_RELOAD, laserfist_RELOAD_TIME))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0;
		m_iShotsFired = 0;
	}
}

void CLaserFist::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
	SendWeaponAnim(BIZON_IDLE1, UseDecrement() != FALSE);
}

float CLaserFist::GetPrimaryAttackDamage()
{
	float flDamage = 3.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 33.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 110.0f;
#endif
	return flDamage;
}

float CLaserFist::GetSecondaryAttackDamage()
{
	float flDamage = 130.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 3739.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 15000.0f;
#endif
	return flDamage;
}
}
