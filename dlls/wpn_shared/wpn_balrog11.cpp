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
#include "wpn_balrog11.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

LINK_ENTITY_TO_CLASS(weapon_balrog11, CBalrog11)

void CBalrog11::Spawn(void)
{
	Precache();
	m_iId = WEAPON_XM1014;
	SET_MODEL(ENT(pev), "models/w_balrog11.mdl");
	m_iDefaultAmmo2 = m_iChargeShotsFired = 0;
	m_iDefaultAmmo = XM1014_DEFAULT_GIVE;

	FallInit();
}

void CBalrog11::Precache(void)
{
	PRECACHE_MODEL("models/v_balrog11.mdl");
	PRECACHE_MODEL("models/w_balrog11.mdl");
	PRECACHE_MODEL("models/p_balrog11.mdl");
	PRECACHE_MODEL("sprites/muzzleflash17.spr");

	m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

	PRECACHE_SOUND("weapons/balrog11-1.wav");
	PRECACHE_SOUND("weapons/balrog11-2.wav");
	PRECACHE_SOUND("weapons/balrog11_insert.wav");
	PRECACHE_SOUND("weapons/balrog9_charge_finish1.wav");

	m_usFireBalrog11 = PRECACHE_EVENT(1, "events/balrog11.sc");
}

int CBalrog11::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "buckshot";
	p->iMaxAmmo1 = MAX_AMMO_BUCKSHOT;
	p->pszAmmo2 = "buckbcs";
	p->iMaxAmmo2 = 7;
	p->iMaxClip = XM1014_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 12;
	p->iId = m_iId = WEAPON_XM1014;
	p->iFlags = 0;
	p->iWeight = XM1014_WEIGHT;

	return 1;
}

BOOL CBalrog11::Deploy(void)
{
	m_iChargeShotsFired = 0;

	return DefaultDeploy("models/v_balrog11.mdl", "models/p_balrog11.mdl", BALROG11_DRAW, "m249", UseDecrement() != FALSE);
}

int CBalrog11::ExtractAmmo(CBasePlayerWeapon* pWeapon)
{
	if (m_iDefaultAmmo2)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
		m_iDefaultAmmo2 = 0;
	}
	return CBasePlayerWeapon::ExtractAmmo(pWeapon);
}


void CBalrog11::PrimaryAttack(void)
{
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
	m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0725, 0.0725, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif
	float flVol = VOL_NORM;

	m_iChargeShotsFired++;

	if (m_iChargeShotsFired >= 4)
	{
		m_iChargeShotsFired = 0;

		if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] < 7)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
			flVol = 0.2;

			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/balrog9_charge_finish1.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
			WRITE_BYTE(CBasePlayer::GetAmmoIndex("buckbcs"));
			WRITE_BYTE(1);
			MESSAGE_END();
#endif
		}
	}

	EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/balrog11-1.wav", flVol, 0.52, 0, 94 + RANDOM_LONG(0, 15));

	int flags;
#ifdef CLIENT_WEAPONS
	flags = FEV_NOTHOST;
#else
	flags = 0;
#endif

	PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireBalrog11, 0, g_vecZero, g_vecZero, flVol, 0.0735, 7, m_vVecAiming.x * 100, FALSE, FALSE);
	
#ifndef CLIENT_DLL
	if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.27s;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.27s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
	else
		m_flTimeWeaponIdle = 0.75s;

	m_fInSpecialReload = 0;

	if (m_pPlayer->pev->flags & FL_ONGROUND)
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 4);
	else
		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
}

void CBalrog11::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == XM1014_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		SendWeaponAnim(BALROG11_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.475s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.475s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.475s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.475s;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/balrog11_insert.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

		SendWeaponAnim(BALROG11_INSERT, UseDecrement());

		m_flNextReload = UTIL_WeaponTimeBase() + 0.28s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.28s;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
		m_pPlayer->ammo_buckshot--;
	}
}


void CBalrog11::SecondaryAttack(void)
{
	if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
		return;

	m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	m_fInSpecialReload = 0;

#ifndef CLIENT_DLL
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	CUtlVector<CBaseEntity*>* pList = new CUtlVector<CBaseEntity*>();
	CUtlVector<CBaseEntity*>* pGroupList = new CUtlVector<CBaseEntity*>();

	BuckBcsFire(0, pList, pGroupList);

	for (int i = 1; i <= 2; i++)
	{
		BuckBcsFire(i * 7.0, pList, pGroupList);
		BuckBcsFire(-i * 7.0, pList, pGroupList);
	}
#endif

	int flags;

	PLAYBACK_EVENT_FULL(0, m_pPlayer->edict(), m_usFireBalrog11, 0, g_vecZero, g_vecZero, 1.0, 0.0735, 7, int(m_vVecAiming.x * 100), TRUE, FALSE);

	if (m_iClip)
		m_flPumpTime = UTIL_WeaponTimeBase() + 0.125s;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.35s;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75s;
}

void CBalrog11::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = zero_duration;

	m_iChargeShotsFired = 0;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload != 0)
		{
			if (m_iClip != XM1014_MAX_CLIP && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else
			{
				SendWeaponAnim(BALROG11_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.43s;
			}
		}
		else
		{
			SendWeaponAnim(BALROG11_IDLE, UseDecrement() != FALSE);

			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
		}
	}
}

#ifndef CLIENT_DLL
void CBalrog11::BuckBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList)
{
	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 25 - gpGlobals->v_up * 2;

	float sinA = sin(fYaw * M_PI / 180);
	float cosA = cos(fYaw * M_PI / 180);
	Vector vecDir = gpGlobals->v_forward * cosA + gpGlobals->v_right * sinA;
	Vector vecAngle;
	VectorAngles(vecDir, vecAngle);
	CBalrog11Cannon* pEnt = static_cast<CBalrog11Cannon*>(CBalrog11Cannon::Create("balrog11_cannon", vecSrc, vecAngle, ENT(m_pPlayer->pev)));
	if (pEnt)
	{
		pEnt->Init(vecDir, 1000, 0.75s, GetDamageBcs(), pList, pGroupList);
	}
}
#endif

float CBalrog11::GetDamage() const
{
	float flDamage = 20.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 20.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 42.0f;
#endif
	return flDamage;
}

float CBalrog11::GetDamageBcs() const
{
	float flDamage = 35.0f;
#ifndef CLIENT_DLL
	if (g_pModRunning->DamageTrack() == DT_ZB)
		flDamage = 625.0f;
	else if (g_pModRunning->DamageTrack() == DT_ZBS)
		flDamage = 2350.0f;
#endif
	return flDamage;
}

}
