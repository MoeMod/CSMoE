/*
wpn_vulcanus11.cpp - Port of Zombie IV Mod
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

    class CVulcanus11 : public LinkWeaponTemplate< CVulcanus11,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_vulcanus11.mdl";
    static constexpr const char *P_Model = "models/p_vulcanus11a.mdl";
    static constexpr const char *W_Model = "models/w_vulcanus11a.mdl";
    static constexpr const char *EventFile = "events/vulcanus11.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_vulcanus11";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 7;

#define WPNSTATE_VULCANUS11_MODEB	1

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
		ANIM_FUNC_CHANGE1,
		ANIM_FUNC_CHANGE2,
		ANIM_IDLE2,
		ANIM_INSERT2,
		ANIM_AFTER_RELOAD2,
		ANIM_START_RELOAD2,
		ANIM_DRAW2,
	};

    static constexpr float PlayerMaxSpeed = 240;
    KnockbackData KnockBack = { 1500.0f, 550.0f, 600.0f, 550.0f, 0.7f };

	KnockbackData GetKnockBackData() override
	{
		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
			return { 750.0f, 450.0f, 600.0f, 450.0f, 0.4f };
		else
			return { 1500.0f, 550.0f, 600.0f, 550.0f, 0.7f };
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		return { ACCURACY_AIR | ACCURACY_SPEED, 9, 4, 0 };
	}

	const char* GetCSModelName() override 
	{
		if(m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
			return "models/w_vulcanus11b.mdl";
		return "models/w_vulcanus11.mdl";		
	}

	bool m_bCheckReload;
	time_point_t m_flTimeCheckState;
	duration_t m_flPumpTime;

	void Spawn()
	{
		m_bCheckReload = false;
		m_flTimeCheckState = invalid_time_point;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/vulcanus11-1.wav");
		PRECACHE_SOUND("weapons/vulcanus11-2.wav");

		PRECACHE_MODEL("models/p_vulcanus11b.mdl");
		PRECACHE_MODEL("models/w_vulcanus11b.mdl");

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
		p->iMaxClip = 7;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_VULCANUS11;
		p->iFlags = 0;
		p->iWeight = XM1014_WEIGHT;

		return 1;
	}

	BOOL Deploy(void)
	{
		m_bCheckReload = false;
		m_flTimeCheckState = invalid_time_point;

		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
			return DefaultDeploy("models/v_vulcanus11.mdl", "models/p_vulcanus11b.mdl", ANIM_DRAW2, "shotgun", UseDecrement() != FALSE);
		else
			return DefaultDeploy("models/v_vulcanus11.mdl", "models/p_vulcanus11a.mdl", ANIM_DRAW, "shotgun", UseDecrement() != FALSE);
	}


	void SecondaryAttack(void)
	{
		m_fInSpecialReload = 0;

		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
		{
			m_flTimeCheckState = gpGlobals->time + 1.0s;
			SendWeaponAnim(ANIM_FUNC_CHANGE2, UseDecrement() != FALSE);
		}
		else
		{
			m_flTimeCheckState = gpGlobals->time + 1.0s;
			SendWeaponAnim(ANIM_FUNC_CHANGE1, UseDecrement() != FALSE);
		}

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1s;
	}

	float GetMaxSpeed(void) override
	{
		return 235;
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

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, flSpread, flSpread, 0, 0, m_iWeaponState & WPNSTATE_VULCANUS11_MODEB, cShots);

		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
			m_iClip -= m_iClip;
		else
			m_iClip--;


#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		else
			m_flTimeWeaponIdle = 0.7s;

		m_fInSpecialReload = 0;

		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
		{
			if (m_pPlayer->pev->flags & FL_ONGROUND)
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 6);
			else
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);
		}
		else
		{
			if (m_pPlayer->pev->flags & FL_ONGROUND)
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 6);
			else
				m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 9);
		}
	}

	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
		flFireDistance = 3000;
		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
		{
			flFireSpread = 0.0635;
			cShots = m_iClip;
			flCycleTime = 0.9s;
			flDamage = 22;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 260;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 395;
#endif
		}
		else
		{
			flFireSpread = 0.07;
			cShots = 7;
			flCycleTime = 0.27s;
			flDamage = 19;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 22;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 40;
#endif
		}
		
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
		{
			m_bCheckReload = true;
			SecondaryAttack();
			m_pPlayer->m_flNextAttack = 1s;
			return;
		}

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(m_iWeaponState & WPNSTATE_VULCANUS11_MODEB ? ANIM_START_RELOAD2 : ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.35s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.35s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.35s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.35s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(m_iWeaponState & WPNSTATE_VULCANUS11_MODEB ? ANIM_INSERT2 : ANIM_INSERT, UseDecrement() != FALSE);

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


	void ItemPostFrame(void) override
	{
		if (m_flTimeCheckState != invalid_time_point && gpGlobals->time >= m_flTimeCheckState)
		{
			m_flTimeCheckState = invalid_time_point;
			if (m_iWeaponState & WPNSTATE_VULCANUS11_MODEB)
			{
				m_iWeaponState &= ~WPNSTATE_VULCANUS11_MODEB;

				m_pPlayer->pev->weaponmodel = MAKE_STRING(P_Model);

				if (m_bCheckReload)
				{
					Reload();
					m_bCheckReload = false;
					return;
				}
			}
			else
			{
				m_iWeaponState |= WPNSTATE_VULCANUS11_MODEB;

				m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_vulcanus11b.mdl");
			}
		}

		CBasePlayerWeapon::ItemPostFrame();
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
					SendWeaponAnim(m_iWeaponState & WPNSTATE_VULCANUS11_MODEB ? ANIM_AFTER_RELOAD2 : ANIM_AFTER_RELOAD, UseDecrement() != FALSE);


					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
			{			
				SendWeaponAnim(m_iWeaponState & WPNSTATE_VULCANUS11_MODEB ? ANIM_IDLE2 : ANIM_IDLE, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;			
			}
				
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_vulcanus11, CVulcanus11)
}