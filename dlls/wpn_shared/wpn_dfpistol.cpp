/*
wpn_dfpistol.cpp - Port of Zombie IV Mod
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

    class CDFPISTOL : public LinkWeaponTemplate< CDFPISTOL,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_dfpistol.mdl";
    static constexpr const char *P_Model = "models/p_dfpistol.mdl";
    static constexpr const char *W_Model = "models/w_dfpistol.mdl";
    static constexpr const char *EventFile = "events/dfpistol.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_dfpistol";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 4;
	static constexpr int DamageDefault = 7;
	static constexpr int DamageZB = 35;
	static constexpr int DamageZBS = 70;

	enum
	{
		ANIM_IDLE,
		ANIM_SHOOT,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr float PlayerMaxSpeed = 250;
    KnockbackData KnockBack = { 2000.0f, 2000.0f, 1700.0f, 1700.0f, 1.0f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 8, 3, 0 };
	}

	int m_iInstantShots;

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash58.spr");

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
		p->iMaxClip = 4;
		p->iSlot = 1;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_DFPISTOL;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		m_iInstantShots = 1;
		return DefaultDeploy("models/v_dfpistol.mdl", "models/p_dfpistol.mdl", ANIM_DRAW, "onehanded", UseDecrement() != FALSE);
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


		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		m_iInstantShots = m_iClip;
		m_iClip = 0;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(7 * m_iInstantShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.085, 0.085, 0.0), 3200, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0.0, g_vecZero, g_vecZero, 0.085, 0.0, m_iInstantShots, 7 * m_iInstantShots, FALSE, FALSE);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 12);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 12, 16);
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			return;

		if (DefaultReload(MaxClip, ANIM_RELOAD, 3.03s))
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
		SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);

	}

};
LINK_ENTITY_TO_CLASS(weapon_dfpistol, CDFPISTOL)
}