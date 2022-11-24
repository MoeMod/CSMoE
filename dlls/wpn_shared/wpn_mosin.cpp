/*
wpn_mosin.cpp - Port of Zombie IV Mod
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
#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

    class CMosin : public LinkWeaponTemplate< CMosin,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SCOUT>::template type,
            TPrecacheEvent,
			TDeployDefault,
			TSecondaryAttackSniperZoom2,
			TGetDamageDefault
    >
    {
    public:
	static constexpr int ZoomFOV1 = 40;
	static constexpr int ZoomFOV2 = 15;
    static constexpr const char *V_Model = "models/v_mosin.mdl";
    static constexpr const char *P_Model = "models/p_mosin.mdl";
    static constexpr const char *W_Model = "models/w_mosin.mdl";
    static constexpr const char *EventFile = "events/mosin.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_mosin";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 5;

	static constexpr const auto& DefaultDeployTime = 1.25s;
	static constexpr int DamageDefault = 105;
	static constexpr int DamageZB = 120;
	static constexpr int DamageZBS = 120;

	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_INSERT,
		ANIM_AFTER_RELOAD,
		ANIM_START_RELOAD,
		ANIM_DRAW,
	};

    static constexpr float PlayerMaxSpeed = 220;
    KnockbackData KnockBack = { 3000.0f, 500.0f, 1200.0f, 800.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_SCOUT); }
#endif
	float GetArmorRatioModifier() override { return 1.95; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 };}

	duration_t m_flPumpTime;

	void Spawn() override
	{

		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_SOUND("weapons/mosin-1.wav");

		m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "762Nato";
		p->iMaxAmmo1 = MAX_AMMO_762NATO;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 5;
		p->iSlot = 0;
		p->iPosition = 9;
		p->iId = m_iId = WEAPON_MOSIN;
		p->iFlags = 0;
		p->iWeight = SCOUT_WEIGHT;

		return 1;
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 220;

		return 175;
	}

	void PrimaryAttack(void)
	{
		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			MosinFire(0.9, 1.45s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 170)
			MosinFire(0.75, 1.45s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			MosinFire(0.0, 1.45s, FALSE);
		else
			MosinFire(0.007, 1.45s, FALSE);
	}

	void MosinFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->m_bResumeZoom = true;
			m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
			m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
		}
		else // not zoomed, so bullets diverts more
			flSpread += 0.086;

		if (m_iClip <= 0)
		{
			Reload();

			if (m_iClip == 0)
				PlayEmptySound();

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
			return;
		}
		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.95s;
		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x * 1000, vecDir.y * 1000, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
		
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.2s;

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

		m_fInSpecialReload = 0;

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
		m_pPlayer->pev->punchangle.x -= 3;
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
			if (m_pPlayer->pev->fov != 90)
				m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.8s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.8s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(ANIM_INSERT, UseDecrement() != FALSE);

			m_flNextReload = UTIL_WeaponTimeBase() + 0.5s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
		}
		else
		{
			m_iClip++;
			m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
			m_fInSpecialReload = 1;
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
				SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
			}
			
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_mosin, CMosin)
}