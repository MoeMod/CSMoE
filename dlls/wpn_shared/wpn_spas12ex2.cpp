/*
wpn_spas12ex2.cpp - Port of Zombie IV Mod
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

    class CSPAS12EX2 : public LinkWeaponTemplate< CSPAS12EX2,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_spas12ex2.mdl";
    static constexpr const char *P_Model = "models/p_spas12ex2.mdl";
    static constexpr const char *W_Model = "models/w_spas12ex2.mdl";
    static constexpr const char *EventFile = "events/spas12ex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_spas12ex2";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 9;

#define WPNSTATE_SPAS12EX2_MODEB	1

	enum
	{
		ANIM_IDLE,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_INSERT,
		ANIM_AFTER_RELOAD,
		ANIM_START_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT3,
	};

    static constexpr float PlayerMaxSpeed = 235;
    KnockbackData KnockBack = { 1600.0f, 600.0f, 625.0f, 575.0f, 0.7f };

	KnockbackData GetKnockBackData() override
	{
		if (m_iWeaponState & WPNSTATE_SPAS12EX2_MODEB)
			return { 750.0f, 450.0f, 600.0f, 450.0f, 0.4f };
		else
			return { 1600.0f, 600.0f, 625.0f, 575.0f, 0.7f };
	}


#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	duration_t m_flPumpTime;

	void Precache() override
	{
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
		p->iId = m_iId = WEAPON_SPAS12EX2;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		return DefaultDeploy("models/v_spas12ex2.mdl", "models/p_spas12ex2.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
	}


	void SecondaryAttack(void)
	{
		if (m_iWeaponState & WPNSTATE_SPAS12EX2_MODEB)
		{
#ifndef CLIENT_DLL
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_SPAS12EX2_SWITCH_TO_MODE1");
#endif

			m_iWeaponState &= ~WPNSTATE_SPAS12EX2_MODEB;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
		}
		else
		{
#ifndef CLIENT_DLL
			ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#CSO_SPAS12EX2_SWITCH_TO_MODE2");
#endif

			m_iWeaponState |= WPNSTATE_SPAS12EX2_MODEB;
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

		float flDamage, flDistance, flSpread;
		duration_t flCycleTime;
		int cShots;

		GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

#ifndef CLIENT_DLL
		m_pPlayer->FireBullets(cShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0), flDistance, BULLET_PLAYER_BUCKSHOT, 0, flDamage);
#endif

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, flSpread, flSpread, 0, 0, m_iWeaponState & WPNSTATE_SPAS12EX2_MODEB, cShots);

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
		else
			m_flTimeWeaponIdle = 0.7s;

		m_fInSpecialReload = 0;

		if (m_iWeaponState & WPNSTATE_SPAS12EX2_MODEB)
		{
			if (m_pPlayer->pev->flags & FL_ONGROUND)
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 5);
			else
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 11);
		}
		else
		{
			if (m_pPlayer->pev->flags & FL_ONGROUND)
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 6);
			else
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 8, 12);

			m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.65s;
		}
	}

	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
		flFireDistance = 3048;
		if (m_iWeaponState & WPNSTATE_SPAS12EX2_MODEB)
		{
			flFireSpread = 0.0695;
			cShots = 7;
			flCycleTime = 0.28s;
			flDamage = 20;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 23;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 23;
#endif
		}
		else
		{
			flFireSpread = 0.0635;
			cShots = 9;
			flCycleTime = 0.88s;
			flDamage = 21;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 23;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 23;
#endif
		}
		
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.45s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.45s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.45s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.45s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_INSERT, UseDecrement() != FALSE);

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
					SendWeaponAnim(ANIM_AFTER_RELOAD, UseDecrement() != FALSE);


					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
			{
				SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
			}
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_spas12ex2, CSPAS12EX2)
}