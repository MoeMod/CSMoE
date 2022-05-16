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
#include "wpn_sgmissileex.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_sgmissileex, CSGMissileEX)

void CSGMissileEX::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_sgmissileex.mdl");
	m_iDefaultAmmo2 = 0;
	m_iDefaultAmmo = 45;

	pev->skin = 0;
	pev->body = 0;

	SetToAMode();
	InitChargeState();
	SendChargeState();

	FallInit();
}

void CSGMissileEX::Precache(void)
{
	PRECACHE_MODEL("sprites/ef_sgmissileex_line.spr");
	PRECACHE_MODEL("models/p_sgmissileex_a.mdl");
	PRECACHE_MODEL("models/p_sgmissileex_b.mdl");

	m_iModelIndex = PRECACHE_MODEL("models/v_sgmissileex.mdl");
	PRECACHE_MODEL("models/w_sgmissileex.mdl");
	m_iShellId = PRECACHE_MODEL("models/shell_sgdrill.mdl");

	m_usFireSGMissileEX = PRECACHE_EVENT(1, "events/sgmissileex.sc");
}

int CSGMissileEX::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "sgdrill_buckshot";
	p->iMaxAmmo1 = 105;
	p->pszAmmo2 = "sgmissilebcs";
	p->iMaxAmmo2 = 12;
	p->iMaxClip = 45;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CSGMissileEX::Deploy(void)
{
	ResetChargeTimer();

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

	if (m_iWeaponState > 0 && m_iWeaponState <= 12)
	{
		SetToBMode();

		return DefaultDeploy("models/v_sgmissileex.mdl", "models/p_sgmissileex_b.mdl", SGMISSILEEX_DRAW_B, "m249", UseDecrement() != FALSE);
	}
	else
	{
		SetToAMode();

		return DefaultDeploy("models/v_sgmissileex.mdl", "models/p_sgmissileex_a.mdl", SGMISSILEEX_DRAW, "m249", UseDecrement() != FALSE);
	}
}

int CSGMissileEX::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}

void CSGMissileEX::Holster(int skiplocal)
{
	ResetChargeTimer();

	CBasePlayerWeapon::Holster();
}

void CSGMissileEX::SetToAMode(void)
{
	pev->modelindex = m_iModelIndex;

	if (GetBodygroup(GROUP_SKINGROUP) == 0)
	{
		SetBodygroup(GROUP_SKINGROUP, 1);
		SetBodygroup(GROUP_SKINGROUP2, 1);
	}

	int hands = 0;

	if (m_pPlayer && m_pPlayer->m_bIsFemale)
		hands = 1;

	SetBodygroup(GROUP_HANDS, hands);

	pev->modelindex = 0;

	if (m_pPlayer && !m_pPlayer->m_bIsZombie)
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissileex_a.mdl");
}

void CSGMissileEX::SetToBMode(void)
{
	pev->modelindex = m_iModelIndex;

	if (GetBodygroup(GROUP_SKINGROUP) == 1)
	{
		SetBodygroup(GROUP_SKINGROUP, 0);
		SetBodygroup(GROUP_SKINGROUP2, 0);
	}

	int hands = 0;

	if (m_pPlayer && m_pPlayer->m_bIsFemale)
		hands = 1;

	SetBodygroup(GROUP_HANDS, hands);

	pev->modelindex = 0;

	if (m_pPlayer && !m_pPlayer->m_bIsZombie)
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissileex_b.mdl");
}

void CSGMissileEX::InitChargeState(void)
{
	m_iWeaponState = 0;
	m_flTimeNextCharge = invalid_time_point;

	pev->skin = 0;
}


#ifndef CLIENT_DLL
void CSGMissileEX::ItemPreFrame(void)
{
	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState;

	if (m_iWeaponState)
	{
		SetToBMode();
	}
	else
	{
		SetToAMode();
		SendChargeState();
	}

	if (m_iWeaponState == 12)
		return;

	if (m_fInReload > 0)
		return;

	if (m_flTimeNextCharge != invalid_time_point && m_flTimeNextCharge <= gpGlobals->time)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState += int((gpGlobals->time - m_flTimeNextCharge) / 2.0s + 1.0);

		if (m_iWeaponState >= 12)
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 12;

		SetChargeSkin();
		SendChargeState();

		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
		WRITE_BYTE(15);
		WRITE_BYTE(44);
		MESSAGE_END();

		m_flTimeNextCharge = gpGlobals->time + 1.5s;

		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
		WRITE_BYTE(CBasePlayer::GetAmmoIndex("sgmissilebcs"));
		WRITE_BYTE(1);
		MESSAGE_END();

		if (m_iWeaponState == 12)
			m_flTimeNextCharge = invalid_time_point;

		if (m_iWeaponState == 1)
		{
			if (m_pPlayer->pev->weaponanim == SGMISSILEEX_IDLE)
			{
				SendWeaponAnim(SGMISSILEEX_MISSILEEX_ON, UseDecrement() != FALSE);

				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1.13s;
			}

			SetToBMode();

			if (!m_pPlayer->m_bIsZombie)
				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissileex_b.mdl");
		}
	}
}
#endif

void CSGMissileEX::PrimaryAttack(void)
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
	m_pPlayer->FireBullets(7, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.04, 0.04, 0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sgmissile-1.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireSGMissileEX, 0, g_vecZero, g_vecZero, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 0.04, 7, m_vVecAiming.x * 100, FALSE, 8);

	if (m_iWeaponState > 0)
		SendWeaponAnim(SGMISSILEEX_SHOOT_B + RANDOM_LONG(0, 1), UseDecrement() != FALSE);
	else
		SendWeaponAnim(SGMISSILEEX_SHOOT, UseDecrement() != FALSE);
	
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

void CSGMissileEX::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	m_flTimeNextCharge = invalid_time_point;

	int iAnim;

	if (m_iWeaponState > 0 && m_iWeaponState <= 12)
		iAnim = SGMISSILEEX_RELOAD_B;
	else
		iAnim = SGMISSILEEX_RELOAD;

	if (DefaultReload(30, iAnim, 2.0s))
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
	}
}


void CSGMissileEX::SecondaryAttack(void)
{
	if (m_iWeaponState <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = --m_iWeaponState;

	if (m_iWeaponState < 0)
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iWeaponState = 0;

	SetChargeSkin();
	SendChargeState();

	m_flTimeNextCharge = invalid_time_point;

	if (m_iWeaponState <= 0)
		m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_sgmissileex_a.mdl");

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	m_fInSpecialReload = 0;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	CUtlVector<CBaseEntity*>* pList = new CUtlVector<CBaseEntity*>();
	CUtlVector<CBaseEntity*>* pGroupList = new CUtlVector<CBaseEntity*>();

	SGMissileEXBcsFire(0, pList, pGroupList);

	for (int i = 1; i <= 4; i++)
	{
		SGMissileEXBcsFire(i * 6.0, pList, pGroupList);
		SGMissileEXBcsFire(-i * 6.0, pList, pGroupList);
	}
#endif

	int flags;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireSGMissileEX, 0, g_vecZero, g_vecZero, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType], 0.04, 7, int(m_vVecAiming.x * 100), TRUE, 0);

	SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? SGMISSILEEX_SHOOT_MISSILEEX : SGMISSILEEX_SHOOT_MISSILEEX_LAST, UseDecrement() != FALSE);

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/sgmissile-2.wav", VOL_NORM, 0.52, 0, 94 + RANDOM_LONG(0, 15));

	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType])
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
	}
	else
	{
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;
	}
}

void CSGMissileEX::ResetChargeTimer(void)
{
	if (m_iWeaponState != 12 && m_flTimeNextCharge == invalid_time_point)
		m_flTimeNextCharge = gpGlobals->time + 2.0s;
}

void CSGMissileEX::SendChargeState(void)
{
	if (!m_pPlayer)
		return;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireSGMissileEX, 0, g_vecZero, g_vecZero, 1.0 - m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] / 12.0, 0, 0, 0, TRUE, 2);
}

void CSGMissileEX::SetChargeSkin(void)
{
	if (m_iWeaponState >= 10)
	{
		pev->skin = 10;
		return;
	}
	pev->skin = m_iWeaponState;
}

void CSGMissileEX::WeaponIdle(void)
{
	ResetEmptySound();

	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

	ResetChargeTimer();

	if (m_iWeaponState > 0 && m_iWeaponState <= 12)
	{
		SetToBMode();

		if (m_pPlayer->pev->weaponanim != SGMISSILEEX_IDLE_B)
			SendWeaponAnim(SGMISSILEEX_IDLE_B, UseDecrement() != FALSE);
	}
	else
	{
		SetToAMode();

		if (m_pPlayer->pev->weaponanim != SGMISSILEEX_IDLE)
			SendWeaponAnim(SGMISSILEEX_IDLE, UseDecrement() != FALSE);
	}
}

#ifndef CLIENT_DLL
void CSGMissileEX::SGMissileEXBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList)
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
		pEnt->Init(vecDir, 1000, 0.59s, GetDamageBcs(), pList, pGroupList, GetExplodeDamage(), GetExplodeRadius(), GetDirectDamageForce(), 4);
	}
}
#endif

float CSGMissileEX::GetDamage() const
{
	float flDamage = 16.0;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 120.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 120.0f;
#endif
	return flDamage;
}

float CSGMissileEX::GetDamageBcs() const
{
	float flDamage = 35.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 1680.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 2350.0f;
#endif
	return flDamage;
}

float CSGMissileEX::GetExplodeDamage(void)
{
	float flDamage = 105.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 600.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 300.0f;
#endif
	return flDamage;
}

float CSGMissileEX::GetDirectDamageForce(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 750;
#endif
	return 30;
}

float CSGMissileEX::GetExplodeRadius(void)
{
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		return 125;
#endif
	return 65;
}

}
