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

LINK_ENTITY_TO_CLASS(weapon_tknifeex2, CTKnifeEX2)

void CTKnifeEX2::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_tknifeex2");

	Precache();
	m_iId = WEAPON_TKNIFEEX2;
	SET_MODEL(ENT(pev), "models/w_tknifeex2.mdl");

	m_iKnifeHas = 0;
	m_iDefaultAmmo = 30;
	m_flStartThrow = invalid_time_point;
	FallInit();
}

void CTKnifeEX2::Precache(void)
{
	PRECACHE_MODEL("models/p_tknifeex2.mdl");
	PRECACHE_MODEL("models/v_tknifeex2.mdl");
	PRECACHE_MODEL("models/w_tknifeex2.mdl");
}

void CTKnifeEX2::Holster(int skiplocal)
{
	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] && !m_iKnifeHas)
	{
		m_pPlayer->RemoveWeapon(m_iId);
		DestroyItem();
	}
}

int CTKnifeEX2::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "tknife";
	p->iMaxAmmo1 = 30;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = -1;
	p->iSlot = 1;
	p->iPosition = 2;
	p->iId = m_iId = WEAPON_TKNIFEEX2;
	p->iFlags = ITEM_FLAG_EXHAUSTIBLE;
	p->iWeight = GLOCK18_WEIGHT;

	return 1;
}

BOOL CTKnifeEX2::Deploy(void)
{
	m_fMaxSpeed = 250;

	return DefaultDeploy("models/v_tknifeex2.mdl", "models/p_tknifeex2.mdl", ANIM_DRAW, "grenade", UseDecrement() != FALSE);

}

void CTKnifeEX2::PrimaryAttack(void)
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

void CTKnifeEX2::ThrowingKnifeFire(float flSpread, duration_t flCycleTime)
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
	ThrowingKnife(flSpread);
#endif
	m_iKnifeHas = 2;
	m_flStartThrow = gpGlobals->time + 0.05s;
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

void CTKnifeEX2::ItemPostFrame(void)
{
	if (m_iKnifeHas > 0)
	{
		if (m_flStartThrow != invalid_time_point && m_flStartThrow < gpGlobals->time)
		{
#ifndef CLIENT_DLL
			float flSpread;
			if (m_iKnifeHas > 1)
			{
				if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
					flSpread = 0.5;
				else if (m_pPlayer->pev->velocity.Length2D() > 0)
					flSpread = 0.2;
				else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
					flSpread = 0.01;
				else
					flSpread = 0.05;
			}
			else
			{
				if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
					flSpread = 0.5;
				else if (m_pPlayer->pev->velocity.Length2D() > 0)
					flSpread = 0.2;
				else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
					flSpread = 0.03;
				else
					flSpread = 0.05;
			}

			ThrowingKnife(flSpread);
#endif
			m_iKnifeHas--;

			if (m_iKnifeHas <= 0)
			{
				m_flStartThrow = invalid_time_point;
				if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
				{
					RetireWeapon();
					return;
				}
			}
			else
				m_flStartThrow = gpGlobals->time + 0.05s;

			return;
		}
	}

	CBasePlayerWeapon::ItemPostFrame();
}


void CTKnifeEX2::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
	SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
}

}
