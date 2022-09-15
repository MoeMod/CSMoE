/*
wpn_spsg.cpp - Port of Zombie IV Mod
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

#include "weapons/WeaponTemplate.hpp"
#include "weapons/RadiusDamage.hpp"
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CSPSG : public LinkWeaponTemplate< CSPSG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_spsg.mdl";
    static constexpr const char *P_Model = "models/p_spsg.mdl";
    static constexpr const char *W_Model = "models/w_spsg.mdl";
    static constexpr const char *EventFile = "events/spsg.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_spsg";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 2;
	static constexpr int DamageDefault = 14;
	static constexpr int DamageZB = 30;
	static constexpr int DamageZBS = 125;

	enum
	{
		ANIM_IDLE1,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,		
	};

    static constexpr float PlayerMaxSpeed = 250;
    KnockbackData KnockBack = { 3500.0f, 600.0f, 1000.0f, 600.0f, 0.65f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	int m_iInstantShots;

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash25.spr");

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
		p->iMaxClip = 2;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_SPSG;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		m_iInstantShots = 1;
		return DefaultDeploy("models/v_spsg.mdl", "models/p_spsg.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
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
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

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
		m_iInstantShots = 1;
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(7 * m_iClip, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.035, 0.035, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif
		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.035, 0.0, m_iInstantShots, 7, FALSE, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		{
			m_pPlayer->pev->punchangle.x -= 9;
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= 5.5;
		}
	}


	void SecondaryAttack(void)
	{
		if (m_iClip == 1)
		{
			PrimaryAttack();

			return;
		}

		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
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

		m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
		m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;


		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_iInstantShots = m_iClip;
		m_iClip = 0;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(7 * m_iInstantShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.035, 0.035, 0.0), 4800, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.035, 0.0, m_iInstantShots, 7 * m_iInstantShots, FALSE, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
		{
			m_pPlayer->pev->punchangle.x -= 9;
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= 5.5;
		}
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(MaxClip, ANIM_RELOAD, 1.03s))
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
		}
	}

	void WeaponIdle(void)
	{
		ResetEmptySound();
		m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
		SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

	}

};
LINK_ENTITY_TO_CLASS(weapon_spsg, CSPSG)
}