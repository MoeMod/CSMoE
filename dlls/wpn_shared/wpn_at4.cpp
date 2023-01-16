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
#include "wpn_at4.h"


#ifndef CLIENT_DLL
#include "soundent.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_at4, CAT4)

void CAT4::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_at4");

	Precache();
	m_iId = WEAPON_AT4;
	SET_MODEL(ENT(pev), "models/w_at4.mdl");

	m_iDefaultAmmo = 1;
	FallInit();
}

void CAT4::Precache(void)
{
	m_usFireAT4 = PRECACHE_EVENT(1, "events/at4.sc");

	PRECACHE_SOUND("weapons/at4-1.wav");

	PRECACHE_MODEL("models/p_at4.mdl");
	PRECACHE_MODEL("models/v_at4.mdl");
	PRECACHE_MODEL("models/w_at4.mdl");
}

int CAT4::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "84mmRocket";
	p->iMaxAmmo1 = 5;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = 1;
	p->iSlot = 0;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_AT4;
	p->iFlags = 0;
	p->iWeight = SCOUT_WEIGHT;

	return 1;
}


float CAT4::GetMaxSpeed(void)
{
	if (m_pPlayer->m_iFOV == 90)
		return 200;

	return 75;
}


BOOL CAT4::Deploy(void)
{
	return DefaultDeploy("models/v_at4.mdl", "models/p_at4.mdl", ANIM_DRAW, "at4", UseDecrement() != FALSE);
}

void CAT4::PrimaryAttack(void)
{
	AT4Fire(false);
}

void CAT4::SecondaryAttack(void)
{
	switch (m_pPlayer->m_iFOV)
	{
	case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
	default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
	}

	m_pPlayer->ResetMaxSpeed();
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
}


void CAT4::AT4Fire(bool autoanim)
{
	if (m_pPlayer->pev->fov != 90)
	{
		m_pPlayer->m_bResumeZoom = true;
		m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
		m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
	}

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

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
#ifndef CLIENT_DLL
	m_pPlayer->m_iExtraSoundTypes = bits_SOUND_DANGER;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecVelocity = gpGlobals->v_forward * 1000;

	int iType = 0;
	if (m_iId == WEAPON_AT4EX)
	{
		if (autoanim)
			iType = 1;
		else
			iType = 2;
	}
	

	CAT4Rocket* pRocket = CAT4Rocket::Create(iType, m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16 + gpGlobals->v_right * 8 + gpGlobals->v_up * -8, m_pPlayer->pev->v_angle, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

	if (pRocket)
	{
		pRocket->pev->velocity = vecVelocity;
		pRocket->pev->owner = m_pPlayer->edict();
		pRocket->m_iTeam = m_pPlayer->m_iTeam;
	}
#endif

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/at4-1.wav", 0.7, ATTN_NORM, 0, PITCH_NORM);

	int flags;

#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFireAT4, 0, g_vecZero, g_vecZero, 0.0, 0.0, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);

}


void CAT4::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (DefaultReload(1, ANIM_RELOAD, 3.8s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		m_bDelayFire = false;
		m_iShotsFired = 0;
		if (m_pPlayer->pev->fov != 90)
			SecondaryAttack();
	}
}

void CAT4::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

}
