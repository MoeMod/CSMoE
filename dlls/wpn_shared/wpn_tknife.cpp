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
#include "wpn_tknife.h"
#include "weapons/KnifeAttack.h"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_tknife, CTKnife)

void CTKnife::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_tknife");

	Precache();
	m_iId = WEAPON_TKNIFE;
	SET_MODEL(ENT(pev), "models/w_tknife.mdl");

	m_iDefaultAmmo = 30;
	m_flStartThrow = invalid_time_point;
	FallInit();
}

void CTKnife::Precache(void)
{
	PRECACHE_MODEL("models/p_tknife.mdl");
	PRECACHE_MODEL("models/v_tknife.mdl");
	PRECACHE_MODEL("models/w_tknife.mdl");
}

int CTKnife::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "tknife";
	p->iMaxAmmo1 = 30;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_TKNIFE;
	p->iFlags = ITEM_FLAG_EXHAUSTIBLE;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

int CTKnife::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo)
	{
		if (m_pPlayer)
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
		m_iClip = WEAPON_NOCLIP;
		m_iDefaultAmmo = 0;
		return TRUE;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

BOOL CTKnife::Deploy(void)
{
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_tknife.mdl", "models/p_tknife.mdl", ANIM_DRAW, "grenade", UseDecrement() != FALSE);
}

void CTKnife::Holster(int skiplocal)
{
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}
}

void CTKnife::PrimaryAttack(void)
{
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		ThrowingKnifeFire(0.04 * (1 - m_flAccuracy), 0.8s);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		ThrowingKnifeFire(0.02, 0.8s);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		ThrowingKnifeFire(0.0001, 0.8s);
	else
		ThrowingKnifeFire(0.0003, 0.8s);
}


void CTKnife::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK2);

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	switch (KnifeAttack5(GetDamage(), 80, 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer))
	{
	case HIT_NONE:
	{
		break;
	}
	case HIT_PLAYER:
	{
		switch (RANDOM_LONG(0, 3))
		{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit1.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit2.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit3.wav", VOL_NORM, ATTN_NORM); break;
		case 3: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/knife_hit4.wav", VOL_NORM, ATTN_NORM); break;
		}
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
		break;
	}
	case HIT_WALL:
	{
		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/knife_hitwall1.wav", VOL_NORM, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
		m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
		break;
	}
	}
#endif

	SendWeaponAnim(ANIM_SHOOT2, UseDecrement() != FALSE);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

	
}

void CTKnife::ThrowingKnifeFire(float flSpread, duration_t flCycleTime)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
		return;
	}

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	if (m_pPlayer->pev->effects & EF_MUZZLEFLASH)
		m_pPlayer->pev->effects &= ~EF_MUZZLEFLASH;

	m_pPlayer->m_iWeaponVolume = QUIET_GUN_VOLUME;

	SendWeaponAnim(ANIM_SHOOT1, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDir = m_pPlayer->FireBullets3(vecSrc - gpGlobals->v_up * 2.0, gpGlobals->v_forward, flSpread, 8192.0, 0, BULLET_PLAYER_556MM, 0, 0.0, m_pPlayer->pev, false, m_pPlayer->random_seed);

	Vector vecKnifeDir = gpGlobals->v_forward + gpGlobals->v_right * vecDir.x + gpGlobals->v_up * vecDir.y;
	Vector vecAngles = UTIL_VecToAngles(vecKnifeDir);

	CThrowingKnifeEntity* pKnife = CThrowingKnifeEntity::Create(0, vecSrc, vecAngles, ENT(m_pPlayer->pev), m_pPlayer->m_iTeam);

	if (pKnife)
	{
		float flSpeed;
		if (m_pPlayer->pev->waterlevel == 3)
			flSpeed = 1400.0;
		else
			flSpeed = 1600.0;

		pKnife->pev->velocity = vecKnifeDir * flSpeed;
		pKnife->pev->speed = flSpeed;
	}
#endif

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		KickBack(2.0, 0.7, 0.5, 0.15, 5.0, 3.5, 8);
	else if (m_pPlayer->pev->velocity.Length2D() > 0)
		KickBack(1.4, 0.6, 0.45, 0.075, 4.0, 3.0, 8);
	else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
		KickBack(0.8, 0.35, 0.3, 0.04, 3.5, 2.0, 9);
	else
		KickBack(1.0, 0.4, 0.45, 0.035, 3.75, 3.0, 9);

}

void CTKnife::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		RetireWeapon();
		return;
	}

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

float CTKnife::GetDamage()
{
	float flDamage = 35;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 85;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 120;
#endif
	return flDamage;
}

}
