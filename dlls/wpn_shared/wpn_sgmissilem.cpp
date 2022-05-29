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
#include "wpn_sgmissilem.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sgmissilem, CSGMissilem)

void CSGMissilem::Spawn(void)
{
	Precache();
	m_iId = WEAPON_SGMISSILEM;
	SET_MODEL(ENT(pev), "models/w_sgmissilem.mdl");
	m_iDefaultAmmo2 = 0;
	m_iDefaultAmmo = 30;

	pev->skin = 0;
	pev->body = 0;

	InitChargeState();
	SendChargeState();

	FallInit();
}

void CSGMissilem::Precache(void)
{
	PRECACHE_MODEL("models/p_sgmissilem.mdl");
	PRECACHE_MODEL("sprites/ef_sgmissilem_line.spr");
	m_iModelIndex = PRECACHE_MODEL("models/v_sgmissilem.mdl");
	PRECACHE_MODEL("models/w_sgmissilem.mdl");
	PRECACHE_MODEL("sprites/muzzleflash115.spr");
	m_iShellId = PRECACHE_MODEL("models/shell_sgdrill.mdl");

	PRECACHE_SOUND("weapons/sgmissilem-2.wav");

	m_usFireSGMissilem = PRECACHE_EVENT(1, "events/sgmissile.sc");
}

int CSGMissilem::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "sgdrill_buckshot";
	p->iMaxAmmo1 = 105;
	p->pszAmmo2 = "sgmissilebcs";
	p->iMaxAmmo2 = 10;
	p->iMaxClip = 30;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_SGMISSILEM;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CSGMissilem::Deploy(void)
{
	ResetChargeTimer();

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;
#ifndef CLIENT_DLL
	ItemPreFrame();
#endif

	if (m_iWeaponState > 0 && m_iWeaponState <= 10)
	{
		return DefaultDeploy("models/v_sgmissilem.mdl", "models/p_sgmissilem.mdl", SGMISSILEM_DRAW_B, "m249", UseDecrement() != FALSE);
	}
	else
	{
		return DefaultDeploy("models/v_sgmissilem.mdl", "models/p_sgmissilem.mdl", SGMISSILEM_DRAW, "m249", UseDecrement() != FALSE);
	}
}

int CSGMissilem::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CSGMissilem::Holster(int skiplocal)
{
	ResetChargeTimer();

	CBasePlayerWeapon::Holster();
}


void CSGMissilem::InitChargeState(void)
{
	m_iWeaponState = 0;
	m_flTimeNextCharge = invalid_time_point;

	pev->skin = 0;
}


#ifndef CLIENT_DLL
void CSGMissilem::ItemPreFrame(void)
{
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

	if (!m_iWeaponState)
		SendChargeState();

	if (m_iWeaponState == 10)
		return;

	if (m_fInReload > 0)
		return;

	if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / 2.0s + 1.0);

		if (m_iWeaponState >= 10)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 10;

		SendChargeState();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(15);
		WRITE_BYTE(44);
		MESSAGE_END();

		m_flTimeNextCharge = gpGlobals->time + 2.0s;

		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
		WRITE_BYTE(CBasePlayer::GetAmmoIndex("sgmissilebcs"));
		WRITE_BYTE(1);
		MESSAGE_END();

		if (m_iWeaponState == 10)
			m_flTimeNextCharge = invalid_time_point;

		if (m_iWeaponState == 1)
		{
			if (m_pPlayer->pev->weaponanim == SGMISSILEM_IDLE)
			{
				SendWeaponAnim(SGMISSILEM_MISSILEM_ON, UseDecrement() != FALSE);

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.13s;
			}

			if (!m_pPlayer->m_bIsZombie)
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissilem.mdl");
		}
	}
}
#endif

void CSGMissilem::PrimaryAttack(void)
{
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

	ResetChargeTimer();

	if (m_pPlayer->pev->waterlevel == 3)
	{
		PlayEmptySound();
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		return;
	}

	if (m_iClip <= 0)
	{
		Reload();

		if (m_iClip == 0)
			PlayEmptySound();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
		return;
	}

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
	m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.04, 0.04, 0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sgmissile-1.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSGMissilem, 0, g_vecZero, g_vecZero, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 0.04, 7, m_vVecAiming.x * 100, FALSE, 8);

	if (m_iWeaponState > 0)
		SendWeaponAnim(SGMISSILEM_SHOOT_B + RANDOM_LONG(0, 1), UseDecrement() != FALSE);
	else
		SendWeaponAnim(SGMISSILEM_SHOOT, UseDecrement() != FALSE);
	
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif


	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.27s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.27s;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 4);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
}

void CSGMissilem::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	m_flTimeNextCharge = invalid_time_point;

	int iAnim;

	if (m_iWeaponState > 0 && m_iWeaponState <= 10)
		iAnim = SGMISSILEM_RELOAD_B;
	else
		iAnim = SGMISSILEM_RELOAD;

	if (DefaultReload(30, iAnim, 2.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}


void CSGMissilem::SecondaryAttack(void)
{
	if (m_iWeaponState <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = --m_iWeaponState;

	if (m_iWeaponState < 0)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 0;

	SendChargeState();

	m_flTimeNextCharge = invalid_time_point;

	if (m_iWeaponState <= 0)
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissilem.mdl");

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	m_fInSpecialReload = 0;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	CUtlVector<CBaseEntity*>* pList = new CUtlVector<CBaseEntity*>();
	CUtlVector<CBaseEntity*>* pGroupList = new CUtlVector<CBaseEntity*>();

	SGMissilemBcsFire(0, pList, pGroupList);

	for (int i = 1; i <= 2; i++)
	{
		SGMissilemBcsFire(i * 6.0, pList, pGroupList);
		SGMissilemBcsFire(-i * 6.0, pList, pGroupList);
	}
#endif

	int flags;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireSGMissilem, 0, g_vecZero, g_vecZero, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 0.04, 7, int(m_vVecAiming.x * 100), TRUE, 0);

	SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? SGMISSILEM_SHOOT_MISSILEM : SGMISSILEM_SHOOT_MISSILEM_LAST, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sgmissilem-2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
	}
}

void CSGMissilem::ResetChargeTimer(void)
{
	if (m_iWeaponState != 10 && m_flTimeNextCharge == invalid_time_point)
		m_flTimeNextCharge = gpGlobals->time + 2.0s;
}

void CSGMissilem::SendChargeState(void)
{
	if (!m_pPlayer)
		return;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireSGMissilem, 0, g_vecZero, g_vecZero, 1.0 - m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] / 10.0, 0, 0, 0, TRUE, 2);
}

void CSGMissilem::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

	ResetChargeTimer();

	if (m_iWeaponState > 0 && m_iWeaponState <= 10)
	{
		if (m_pPlayer->pev->weaponanim != SGMISSILEM_IDLE_B)
			SendWeaponAnim(SGMISSILEM_IDLE_B, UseDecrement() != FALSE);
	}
	else
	{
		if (m_pPlayer->pev->weaponanim != SGMISSILEM_IDLE)
			SendWeaponAnim(SGMISSILEM_IDLE, UseDecrement() != FALSE);
	}
}

#ifndef CLIENT_DLL
void CSGMissilem::SGMissilemBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList)
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 25 - gpGlobals->v_up * 2;

	float sinA = sin(fYaw * M_PI / 180);
	float cosA = cos(fYaw * M_PI / 180);
	Vector vecDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;
	Vector vecAngle;
	VectorAngles(vecDir, vecAngle);
	CSGMissileCannon* pEnt = static_cast<CSGMissileCannon*>(CSGMissileCannon::Create("sgmissile_cannon", vecSrc, vecAngle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(vecDir, 1000, 0.75s, GetDamageBcs(), pList, pGroupList, GetExplodeDamage(), GetExplodeRadius(), GetDirectDamageForce(), 3);
	}
}
#endif

float CSGMissilem::GetDamage() const
{
	float flDamage = 10.0;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 54.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 70.0f;
#endif
	return flDamage;
}

float CSGMissilem::GetDamageBcs() const
{
	float flDamage = 25.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 790.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 2000.0f;
#endif
	return flDamage;
}

float CSGMissilem::GetExplodeDamage(void)
{
	float flDamage = 100.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 330.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 167.0f;
#endif
	return flDamage;
}

float CSGMissilem::GetDirectDamageForce(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 750;
#endif
	return 30;
}

float CSGMissilem::GetExplodeRadius(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 125;
#endif
	return 65;
}

}
