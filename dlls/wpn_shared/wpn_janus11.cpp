/*
wpn_janus11.cpp - Port of Zombie IV Mod
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

    class CJANUS11 : public LinkWeaponTemplate< CJANUS11,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M3>::template type,
            TPrecacheEvent
    >
    {
    public:
    static constexpr const char *V_Model = "models/v_janus11.mdl";
    static constexpr const char *P_Model = "models/p_janus11.mdl";
    static constexpr const char *W_Model = "models/w_janus11.mdl";
    static constexpr const char *EventFile = "events/janus11.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_janus11";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 15;

#define WPNSTATE_JANUS11_SIGNAL 1
#define WPNSTATE_JANUS11_USING 2

	enum
	{
		ANIM_IDLE,
		ANIM_CHANGE,
		ANIM_SHOOT1,
		ANIM_INSERT,
		ANIM_AFTER_RELOAD,
		ANIM_START_RELOAD,
		ANIM_DRAW,
		ANIM_IDLE2,
		ANIM_SHOOT2,
		ANIM_DRAW2,
		ANIM_CHANGE2,
		ANIM_IDLE_SIGNAL,
		ANIM_INSERT_SIGNAL,
		ANIM_AFTER_RELOAD_SIGNAL,
		ANIM_START_RELOAD_SIGNAL,
		ANIM_SHOOT_SIGNAL,
		ANIM_DRAW_SIGNAL,
	};

    static constexpr float PlayerMaxSpeed = 205;
    KnockbackData KnockBack = { 750.0f, 500.0f, 600.0f, 500.0f, 0.4f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.3; }
	WeaponCrosshairData GetCrosshairData() override
	{ 
		if(m_iWeaponState)
			return { CROSSHAIR_COLOR_CUSTOM, 9, 4, 0 };
		else
			return { 0, 9, 4, 0 };
	}

	time_point_t m_flNextReset;
	duration_t m_flPumpTime;

	void Spawn() override
	{
		m_flNextReset = invalid_time_point;
		Base::Spawn();
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/muzzleflash12.spr");
		PRECACHE_MODEL("sprites/muzzleflash32.spr");
		PRECACHE_SOUND("weapons/janus11_1.wav");

		m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

		return Base::Precache();
	}
    	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "buckshot";
		p->iMaxAmmo1 = 32;
		p->pszAmmo2 = "InfiniteAmmo";
		p->iMaxAmmo2 = 255;
		p->iMaxClip = 15;
		p->iSlot = 0;
		p->iPosition = 5;
		p->iId = m_iId = WEAPON_JANUS11;
		p->iFlags = 0;
		p->iWeight = M3_WEIGHT;

		return 1;
	}

	int ExtractAmmo(CBasePlayerWeapon* pWeapon)
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	BOOL Deploy(void)
	{
		int iAnim;
		if (m_iWeaponState == WPNSTATE_JANUS11_USING)
			iAnim = ANIM_DRAW2;
		else if (m_iWeaponState == WPNSTATE_JANUS11_SIGNAL)
			iAnim = ANIM_DRAW_SIGNAL;
		else
			iAnim = ANIM_DRAW;
		return DefaultDeploy("models/v_janus11.mdl", "models/p_janus11.mdl", iAnim, "shotgun", UseDecrement() != FALSE);
	}

	void SecondaryAttack(void)
	{
		if (m_iWeaponState == WPNSTATE_JANUS11_SIGNAL)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
			m_iWeaponState = WPNSTATE_JANUS11_USING;
			SendWeaponAnim(ANIM_CHANGE, UseDecrement() != FALSE);
			m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;

			m_flNextReset = gpGlobals->time + 7.0s;
		}
	}

	void ItemPostFrame(void)
	{
		if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
		{
			if (m_iWeaponState == WPNSTATE_JANUS11_SIGNAL)
			{
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
				m_flNextReset = invalid_time_point;
			}
			if (m_iWeaponState == WPNSTATE_JANUS11_USING)
			{
				m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
				SendWeaponAnim(ANIM_CHANGE2, UseDecrement() != FALSE);
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.67s;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.67s;
				m_flNextReset = invalid_time_point;
			}
			pev->iuser1 = 0;
			m_iWeaponState = 0;
		}
		return CBasePlayerWeapon::ItemPostFrame();
	}

	void PrimaryAttack(void)
	{
		if (m_pPlayer->pev->waterlevel == 3)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
			return;
		}

		if (m_iClip <= 0 && m_iWeaponState != WPNSTATE_JANUS11_USING)
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

		PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0, 0, 0, 0, m_iWeaponState, FALSE);
#endif

		float flDamage, flDistance, flSpread;
		duration_t flCycleTime;
		int cShots;

		GetFireInfo(flDamage, flDistance, flSpread, cShots, flCycleTime);

		if (m_iWeaponState == WPNSTATE_JANUS11_USING)
		{
			FireBulletsEx(8, flDamage, flDistance, flSpread, m_pPlayer->GetGunPosition(), Vector(flSpread, flSpread, 0.0));
		}
		else
		{
			m_iClip--;
			UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
			m_pPlayer->FireBullets(cShots, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(flSpread, flSpread, 0.0), flDistance, BULLET_PLAYER_BUCKSHOT, 0, flDamage);

			if (CBaseEntity* pEntity = gMultiDamage.pEntity)
			{
				if (!m_iWeaponState)
				{
					if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
					{
						pev->iuser1++;
						if (pev->iuser1 >= 15)
						{
							m_iWeaponState = WPNSTATE_JANUS11_SIGNAL;
							m_flNextReset = gpGlobals->time + 9.0s;
						}

					}
				}
			}
#endif
		}

		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		if (m_iClip)
			m_flPumpTime = UTIL_WeaponTimeBase() + 0.5s;

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

		if (m_iClip)
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
		else
			m_flTimeWeaponIdle = 0.875s;

		m_fInSpecialReload = 0;

		if (m_pPlayer->pev->flags & FL_ONGROUND)
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 3, 5);
		else
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 10);

		m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.35s;
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

	void FireBulletsEx(int cShots, float flDamage, float flFireDistance, float flFireSpread, Vector vecSrc, Vector vecSpread)
	{
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		for (int iShot = 1; iShot <= cShots; iShot++)
		{
			// get circular gaussian spread
			float x, y, z;

			do {
				x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
				z = x * x + y * y;
			} while (z > 1);

			Vector vecDir;

			vecDir = gpGlobals->v_forward + x * vecSpread.x * gpGlobals->v_right + y * vecSpread.y * gpGlobals->v_up;

			m_pPlayer->FireBullets3(vecSrc, vecDir, 0, flFireDistance, 7, BULLET_PLAYER_JANUS11_BEAMSHOT, flDamage, 1.0, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
		}
	}

	void GetFireInfo(float& flDamage, float& flFireDistance, float& flFireSpread, int& cShots, duration_t& flCycleTime)
	{
		if (m_iWeaponState == WPNSTATE_JANUS11_USING)
		{
			flFireDistance = 8192;
			flFireSpread = 0.07;
			cShots = 8;
			flCycleTime = 0.45s;
			flDamage = 25;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 60;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 120;
#endif
		}
		else
		{
			flFireDistance = 4096;
			flFireSpread = 0.07;
			cShots = 8;
			flCycleTime = 0.71s;
			flDamage = 0;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 0;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 40;
#endif
		}
	}

	void Reload(void)
	{
		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == MaxClip || m_iWeaponState == WPNSTATE_JANUS11_USING)
			return;

		if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
			return;

		if (!m_fInSpecialReload)
		{
#ifndef CLIENT_DLL
			m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
			SendWeaponAnim(m_iWeaponState == WPNSTATE_JANUS11_SIGNAL ? ANIM_START_RELOAD_SIGNAL : ANIM_START_RELOAD, UseDecrement() != FALSE);

			m_fInSpecialReload = 1;
			m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.53s;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.53s;
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.53s;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.53s;
		}
		else if (m_fInSpecialReload == 1)
		{
			if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
				return;

			m_fInSpecialReload = 2;
			SendWeaponAnim(m_iWeaponState == WPNSTATE_JANUS11_SIGNAL ? ANIM_INSERT_SIGNAL : ANIM_INSERT, UseDecrement() != FALSE);

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
					SendWeaponAnim(m_iWeaponState == WPNSTATE_JANUS11_SIGNAL ? ANIM_AFTER_RELOAD_SIGNAL : ANIM_AFTER_RELOAD, UseDecrement() != FALSE);

					m_fInSpecialReload = 0;
					m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
				}
			}
			else
			{
				int iAnim;
				if (m_iWeaponState == WPNSTATE_JANUS11_USING)
					iAnim = ANIM_IDLE2;
				else if (m_iWeaponState == WPNSTATE_JANUS11_SIGNAL)
					iAnim = ANIM_IDLE_SIGNAL;
				else
					iAnim = ANIM_IDLE;
				SendWeaponAnim(iAnim, UseDecrement() != FALSE);
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
			}
			
		}
	}

};
LINK_ENTITY_TO_CLASS(weapon_janus11, CJANUS11)
}