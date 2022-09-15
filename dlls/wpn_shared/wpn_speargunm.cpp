/*
wpn_speargunm.cpp - Black Dragon speargunm from CSO
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "wpn_speargun.h"

#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_speargunm, CSpearGunm)

enum speargunm_e
{
	SPEARGUNM_IDLE1,
	SPEARGUNM_IDLE2,
	SPEARGUNM_IDLE3,
	SPEARGUNM_SHOOT1,
	SPEARGUNM_SHOOT2,
	SPEARGUNM_SHOOT3,
	SPEARGUNM_RELOAD,
	SPEARGUNM_DRAW1,
	SPEARGUNM_DRAW2,
	SPEARGUNM_DRAW3,
	SPEARGUNM_DRAW_EMPTY,
	SPEARGUNM_IDLE_EMPTY,
	SPEARGUNM_SHOOT_EMPTY
};

float UTIL_QuickRandomFloat(float low, float high);

int CSpearGunm::GetItemInfo(ItemInfo* p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SpearmAmmo";
	p->iMaxAmmo1 = 30;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 3;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SPEARGUNM;
	p->iFlags = 0;
	p->iWeight = AK47_WEIGHT;

	return 1;
}

void CSpearGunm::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_speargunm");

	Precache();

	m_iId = WEAPON_SPEARGUNM;
	SET_MODEL(ENT(pev), "models/w_speargunm.mdl");

	FallInit();

	m_flAccuracy = 0.05;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	m_iDefaultAmmo = 3;
}

void CSpearGunm::Precache(void)
{
	PRECACHE_MODEL("models/v_speargunm.mdl");
	PRECACHE_MODEL("models/p_speargunm.mdl");
	PRECACHE_MODEL("models/w_speargunm.mdl");

	PRECACHE_MODEL("models/spearm.mdl");
	PRECACHE_MODEL("models/spearm2.mdl");

	PRECACHE_SOUND("weapons/speargunm-1.wav");

	PRECACHE_SOUND("weapons/speargun_wood1.wav");
	PRECACHE_SOUND("weapons/speargun_wood2.wav");
	PRECACHE_SOUND("weapons/speargun_metal1.wav");
	PRECACHE_SOUND("weapons/speargun_metal2.wav");
	PRECACHE_SOUND("weapons/speargun_stone1.wav");
	PRECACHE_SOUND("weapons/speargun_stone2.wav");

	m_usFireSpearGunm = PRECACHE_EVENT(1, "events/speargunm.sc");
}

BOOL CSpearGunm::Deploy(void)
{
	m_flAccuracy = 0.05;
	m_iShotsFired = 0;
	m_bDelayFire = false;

	iShellOn = 0;

	int iAnim;

	if (m_iClip >= 3)
		iAnim = SPEARGUNM_DRAW1;
	else if(m_iClip == 2)
		iAnim = SPEARGUNM_DRAW2;
	else if (m_iClip == 1)
		iAnim = SPEARGUNM_DRAW3;
	else
		iAnim = SPEARGUNM_DRAW_EMPTY;

		return DefaultDeploy("models/v_speargunm.mdl", "models/p_speargunm.mdl", iAnim, "carbine", UseDecrement() != FALSE);
}

float CSpearGunm::GetSpread(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		return m_flAccuracy * 0.035 + 0.1;
	else if (m_pPlayer->pev->velocity.Length2D() > 120)
		return m_flAccuracy * 0.006 + 0.02;
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		return m_flAccuracy * 0.001 + 0.0075;
	else
		return m_flAccuracy * 0.001 + 0.01;
}

void CSpearGunm::PrimaryAttack(void)
{
	int index;

	for (index = 0; index < SPEARGUNM_MAX_CLIP; index++)
	{
		if (!m_hSpears[index].Get())
			break;
	}

	if (index == -1 || index >= SPEARGUNM_MAX_CLIP)
		return;

	if (m_iClip <= 0)
		return;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;

	m_iClip--;

	m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

	m_flTimeReloadSpear = gpGlobals->time + 0.6s;

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() - gpGlobals->v_up * 2;
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, GetSpread(), 8192, 0, BULLET_PLAYER_556MM, 0, 0, m_pPlayer->pev, false, m_pPlayer->random_seed);
	Vector vecSpearDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	Vector vecAngles = UTIL_VecToAngles(vecSpearDir);

	CSpear* pSpear = CSpear::Create(1);

	if (pSpear)
	{
		m_hSpears[index] = pSpear;

		pSpear->m_iTeam = m_pPlayer->m_iTeam;
		pSpear->pev->origin = vecSrc;
		pSpear->pev->angles = vecAngles;
		pSpear->pev->owner = m_pPlayer->edict();
		pSpear->m_vecVelocity = pSpear->pev->velocity = vecSpearDir * 2000;
		pSpear->pev->avelocity.z = 5;
		pSpear->m_flTimeExplode = gpGlobals->time + 4s;
		pSpear->m_vecDirection = pSpear->pev->velocity.Normalize();

		pSpear->m_flHitDamage = 95;
		pSpear->m_flExplodeDamage = 25;
		pSpear->m_flOwnerDamageModifier = 0.1;
		pSpear->m_flCrowdDamage = 15;
		pSpear->m_flPushDuration = 0s;
		pSpear->m_flExplodeForce = 100;
		pSpear->m_flPushForce = 0;
		pSpear->m_flPushBoostVel = 0;
		pSpear->m_flCrowdForce = 0;
		pSpear->m_flCrowdBoostVel = 0;
		pSpear->m_pOwner = m_pPlayer;

		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			pSpear->m_flPushDuration = 0.4s;
			pSpear->m_flHitDamage = 560 + 1440;
			pSpear->m_flExplodeDamage = 250 + 60;
			pSpear->m_flOwnerDamageModifier = 0.1;
			pSpear->m_flCrowdDamage = 125 + 3;
			pSpear->m_flExplodeForce = 350;
			pSpear->m_flPushForce = 650;
			pSpear->m_flPushBoostVel = 50;
			pSpear->m_flCrowdForce = 500;
			pSpear->m_flCrowdBoostVel = 125;
		}
	}
#endif

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireSpearGunm, 0, g_vecZero, g_vecZero, 0, 0, 0, m_iClip, FALSE, FALSE);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6s;

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(15.0, 10.0, 0.6, 0.35, 9.0, 6.0, 5);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(15.0, 10.0, 0.225, 0.05, 6.5, 2.5, 7);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(4.0, 3.0, 0.125, 0.02, 5.0, 1.35, 9);
	else
		KickBack(10.0, 7.0, 0.22, 0.038, 5.9, 1.9, 8);
}

void CSpearGunm::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	for (int i = 0; i < SPEARGUNM_MAX_CLIP; i++)
	{
		if (m_hSpears[i].Get() != NULL)
		{
			((CSpear*)((CBaseEntity*)m_hSpears[i]))->Explode(0.0s, false);
		}
	}
#endif
}


void CSpearGunm::Reload(void)
{

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(SPEARGUNM_MAX_CLIP, SPEARGUNM_RELOAD, 1.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_flAccuracy = 0.05;
		m_iShotsFired = 0;
		m_bDelayFire = false;
	}
}

void CSpearGunm::ItemPostFrame(void)
{
	if(!m_iClip)
		m_flTimeReloadSpear = invalid_time_point;

	if (m_flTimeReloadSpear != invalid_time_point && gpGlobals->time > m_flTimeReloadSpear && m_iClip > 0)
	{
		PrimaryAttack();	
	}
	else
	{
		CBasePlayerWeapon::ItemPostFrame();
	}
}

void CSpearGunm::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	int iAnim;

	if (m_iClip >= 3)
		iAnim = SPEARGUNM_IDLE1;
	else if (m_iClip == 2)
		iAnim = SPEARGUNM_IDLE2;
	else if (m_iClip == 1)
		iAnim = SPEARGUNM_IDLE3;
	else
		iAnim = SPEARGUNM_IDLE_EMPTY;

	SendWeaponAnim(iAnim);

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
}

}
