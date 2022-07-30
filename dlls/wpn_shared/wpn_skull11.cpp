/*
wpn_skull11.cpp - Port of Zombie IV Mod
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

    class CSKULL11 : public LinkWeaponTemplate< CSKULL11,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent,
            TDeployDefault,
			TWeaponIdleDefault,
			TReloadDefault
    >
    {
    public:
	static constexpr const auto& DefaultReloadTime = 3.9s;
    static constexpr const char *V_Model = "models/v_skull11.mdl";
    static constexpr const char *P_Model = "models/p_skull11.mdl";
    static constexpr const char *W_Model = "models/w_skull11.mdl";
    static constexpr const char *EventFile = "events/skull11.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull11";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 28;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 205;
    static constexpr auto BulletType = BULLET_PLAYER_BUCKSHOT;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 750.0f, 500.0f, 600.0f, 500.0f, 0.4f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_buckshotaz" , 175 }; }
#endif
	float GetArmorRatioModifier() override { return 1.3; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

	void Precache() override
	{
		PRECACHE_SOUND("weapons/skull11_1.wav");
		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "buckshotaz";
		p->iMaxAmmo1 = 40;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 28;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_SKULL11;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
		{
#ifndef CLIENT_DLL
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_Skull11_Switch_To_Buckshot");
#endif

			m_iWeaponState &= ~WPNSTATE_GLOCK18_BURST_MODE;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		}
		else
		{
#ifndef CLIENT_DLL
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_Skull11_Switch_To_Slug");
#endif

			m_iWeaponState |= WPNSTATE_GLOCK18_BURST_MODE;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		}

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
		float flDamage, flDistance, flSpread;
		duration_t flCycleTime;
		int cShots;

		GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
		
#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(cShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0.0), flDistance, BULLET_PLAYER_BUCKSHOT, 0, flDamage);
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.0, flSpread, 7, m_vVecAiming.x * 100, m_iClip == 0, cShots);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.25s;
		else
			m_flTimeWeaponIdle = 0.75s;
		
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 3);
	}

	float GetDamageShotGun()
	{
		float flDamage = 20;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
		{
			flDamage = 80;	//600
		}
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
		{
			flDamage = 120;
		}
#endif
		return flDamage;
	}


	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
		if (!(m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE))
		{
			flFireDistance = 3048;
			flFireSpread = 0.0775;
			cShots = 7;
			flCycleTime = 0.29s;
			flDamage = 0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 22;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 38;
#endif
		}
		else
		{
			flFireDistance = 6048;
			flFireSpread = 0.01;
			cShots = 3;
			flCycleTime = 0.5s;
			flDamage = 23;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 30;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 74;
#endif
		}
	}


};
LINK_ENTITY_TO_CLASS(weapon_skull11, CSKULL11)
}