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
#include "wpn_zombiebomb.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_zombibomb, CZombieBomb)

void CZombieBomb::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_zombibomb");
	Precache();
	m_iId = WEAPON_HEGRENADE;

	SET_MODEL(ENT(pev), "models/w_zombibomb.mdl");

	pev->dmg = 4;
	m_iDefaultAmmo = HEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;

	FallInit();
}

void CZombieBomb::Precache(void)
{
	PRECACHE_MODEL("models/v_zombibomb.mdl");
	PRECACHE_MODEL("models/w_zombibomb.mdl");
	PRECACHE_MODEL("models/p_zombibomb.mdl");
#ifdef ENABLE_SHIELD
	PRECACHE_MODEL("models/shield/v_shield_hegrenade.mdl");
#endif
	PRECACHE_SOUND("weapons/zombi_bomb_deploy.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_idle_1.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_idle_2.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_idle_3.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_idle_4.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_pull_1.wav");
	PRECACHE_SOUND("weapons/zombi_bomb_throw.wav");

	m_usCreateExplosion = PRECACHE_EVENT(1, "events/createexplo.sc");
}

int CZombieBomb::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "HEGrenade";
	p->iMaxAmmo1 = MAX_AMMO_HEGRENADE;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_HEGRENADE;
	p->iWeight = HEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CZombieBomb::Deploy(void)
{
	m_flReleaseThrow = invalid_time_point;
	m_fMaxSpeed = 250;
	m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
	m_pPlayer->m_bShieldDrawn = false;
#ifdef ENABLE_SHIELD
	if (m_pPlayer->HasShield() != false)
		return DefaultDeploy("models/shield/v_shield_hegrenade.mdl", "models/shield/p_shield_hegrenade.mdl", HEGRENADE_DRAW, "shieldgren", UseDecrement() != FALSE);
	else
#endif
		return DefaultDeploy("models/v_zombibomb.mdl", "models/p_zombibomb.mdl", HEGRENADE_DRAW, "grenade", UseDecrement() != FALSE);
}

BOOL CZombieBomb::CanHolster(void)
{
	//return m_flStartThrow != invalid_time_point;
	return CBasePlayerWeapon::CanHolster();
}

void CZombieBomb::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5s;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_HEGRENADE);
		DestroyItem();
	}

	m_flStartThrow = invalid_time_point;
	m_flReleaseThrow = invalid_time_point;
}

void CZombieBomb::PrimaryAttack(void)
{
	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
		return;

	if (m_flStartThrow == invalid_time_point && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = invalid_time_point;
		SendWeaponAnim(HEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
	}
}

void CZombieBomb::SetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shield");
		else
			strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
	}
}

void CZombieBomb::ResetPlayerShieldAnim(void)
{
	if (m_pPlayer->HasShield() == true)
	{
		if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
			strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
	}
}

bool CZombieBomb::ShieldSecondaryFire(int up_anim, int down_anim)
{
	if (m_pPlayer->HasShield() == false)
		return false;

	if (m_flStartThrow != invalid_time_point)
		return false;

	if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
	{
		m_iWeaponState &= ~WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(down_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shieldgren");
		m_fMaxSpeed = 250;
		m_pPlayer->m_bShieldDrawn = false;
	}
	else
	{
		m_iWeaponState |= WPNSTATE_SHIELD_DRAWN;
		SendWeaponAnim(up_anim, UseDecrement() != FALSE);
		strcpy(m_pPlayer->m_szAnimExtention, "shielded");
		m_fMaxSpeed = 180;
		m_pPlayer->m_bShieldDrawn = true;
	}

#ifndef CLIENT_DLL
	m_pPlayer->UpdateShieldCrosshair((m_iWeaponState & WPNSTATE_SHIELD_DRAWN) == 0);
	m_pPlayer->ResetMaxSpeed();
#endif
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.4s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.6s;
	return true;
}

void CZombieBomb::SecondaryAttack(void)
{
	ShieldSecondaryFire(SHIELDGUN_DRAW, SHIELDGUN_DRAWN_IDLE);
}

void CZombieBomb::WeaponIdle(void)
{
	if (m_flReleaseThrow == invalid_time_point && m_flStartThrow != invalid_time_point)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow != invalid_time_point)
	{
#ifndef CLIENT_DLL
		m_pPlayer->Radio("%!MRAD_FIREINHOLE_ZB", "#Fire_in_the_hole");
#endif
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = (90 - angThrow.x) * 6;

		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		auto time = 1.6s;
#ifndef CLIENT_DLL
		CGrenade::ShootZombieBomb(m_pPlayer->pev, vecSrc, vecThrow, time, m_pPlayer->m_iTeam, NULL);
#endif
		SendWeaponAnim(HEGRENADE_THROW, UseDecrement() != FALSE);
		SetPlayerShieldAnim();

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_flStartThrow = invalid_time_point;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			m_flTimeWeaponIdle = m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5s;

		ResetPlayerShieldAnim();
		return;
	}
	else if (m_flReleaseThrow != invalid_time_point)
	{
		m_flStartThrow = invalid_time_point;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(HEGRENADE_DRAW, UseDecrement() != FALSE);
			m_flReleaseThrow = invalid_time_point;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		if (m_pPlayer->HasShield() != false)
		{
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

			if (m_iWeaponState & WPNSTATE_SHIELD_DRAWN)
				SendWeaponAnim(SHIELDREN_IDLE, UseDecrement() != FALSE);

			return;
		}

		SendWeaponAnim(HEGRENADE_IDLE, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RandomDuration<float>(10s, 15s);
	}
}

BOOL CZombieBomb::CanDeploy(void)
{
	return m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] != 0;
}

}
