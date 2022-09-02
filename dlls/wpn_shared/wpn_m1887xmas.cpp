/*
wpn_m1887xmas.cpp - Port of Zombie IV Mod
Copyright (C) 2019 Moemod Haoyuan

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

#include "weapons/KnifeAttack.h"
#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CM1887xmas : public LinkWeaponTemplate< CM1887xmas,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_m1887xmas.mdl";
    static constexpr const char *P_Model = "models/p_m1887xmas.mdl";
    static constexpr const char *W_Model = "models/w_m1887xmas.mdl";
    static constexpr const char *EventFile = "events/m1887xmas.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m1887xmas";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 8;
	static constexpr int DamageDefault = 20;
	static constexpr int DamageZB = 22;
	static constexpr int DamageZBS = 22;

	enum
	{
		M1887_IDLE,
		M1887_FIRE1,
		M1887_FIRE2,
		M1887_FIRE3,
		M1887_FIRE4,
		M1887_FIRE5,
		M1887_RELOAD,
		M1887_PUMP,
		M1887_START_RELOAD,
		M1887_DRAW
	};

    static constexpr float PlayerMaxSpeed = 230;
    KnockbackData KnockBack = { 1000.0f, 250.0f, 600.0f, 700.0f, 0.9f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.3; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	duration_t m_flPumpTime;

	void Precache() override
	{
		PRECACHE_SOUND("weapons/m1887xmas-1.wav");
		PRECACHE_SOUND("weapons/m1887xmas-2.wav");
		PRECACHE_SOUND("weapons/m1887xmas-3.wav");
		PRECACHE_SOUND("weapons/reload1.wav");
		PRECACHE_SOUND("weapons/reload3.wav");
		PRECACHE_MODEL("models/xmas_empty.mdl");

		m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "buckshot";
		p->iMaxAmmo1 = 32;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 8;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_M1887XMAS;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
		WRITE_BYTE(31);
		MESSAGE_END();
#endif
		return DefaultDeploy("models/v_m1887xmas.mdl", "models/p_m1887xmas.mdl", M1887_DRAW, "shotgun", UseDecrement() != FALSE);
	}

	void PrimaryAttack(void)
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
		m_pPlayer->FireBullets(8, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.0675, 0.0675, 0), 3000, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, FALSE, FALSE);

		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
		else
			m_flTimeWeaponIdle = 0.7s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 6, 10);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 10, 14);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
			SendWeaponAnim(M1887_START_RELOAD, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);

			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(31);
			MESSAGE_END();
#endif
			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.55s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.55s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.55s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.55s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(M1887_RELOAD, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
			MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
			WRITE_BYTE(31);
			MESSAGE_END();
#endif
			m_flNextReload = UTIL_WeaponTimeBase() + 0.45s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45s;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
			m_pPlayer->ammo_buckshot--;
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flPumpTime != zero_duration && m_flPumpTime < UTIL_WeaponTimeBase())
			m_flPumpTime = zero_duration;

		if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
		{
			if (m_iClip == 0 && m_fInSpecialReload == 0 && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				Reload();
			}
			else if (m_fInSpecialReload != 0)
			{
				if (m_iClip != MaxClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
				{
					Reload();
				}
				else
				{
					SendWeaponAnim(M1887_PUMP, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
					MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
					WRITE_BYTE(31);
					MESSAGE_END();
#endif

					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
			{
				SendWeaponAnim(M1887_IDLE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 13s;
#ifndef CLIENT_DLL
				MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
				WRITE_BYTE(31);
				MESSAGE_END();
#endif
			}
				
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_m1887xmas, CM1887xmas)
}