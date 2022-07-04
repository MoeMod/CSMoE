/*
z4b_as50g.cpp - Port of Zombie IV Mod
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

    class CAS50G : public LinkWeaponTemplate< CAS50G,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
			TSecondaryAttackSniperZoom2,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.0s;
    static constexpr int ZoomFOV1 = 40;
    static constexpr int ZoomFOV2 = 10;
    static constexpr const char *V_Model = "models/v_as50g.mdl";
    static constexpr const char *P_Model = "models/p_as50g.mdl";
    static constexpr const char *W_Model = "models/w_as50g.mdl";
    static constexpr const char *EventFile = "events/as50.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_as50g";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 5;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr float RangeModifier = 0.973;
    static constexpr auto BulletType = BULLET_PLAYER_338MAG;
    KnockbackData KnockBack = { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f };

	float GetArmorRatioModifier() override { return 1.7; }

	void Precache() override
	{
		m_iShellId = PRECACHE_MODEL("models/rshell_big.mdl");
		return Base::Precache();
	}
	
	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50BMG";
		p->iMaxAmmo1 = MAX_AMMO_50BMG;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 5;
		p->iSlot = 0;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_AS50G;
		p->iFlags = 0;
		p->iWeight = AWP_WEIGHT;

		return 1;
	}

	void PrimaryAttack(void)
	{
		duration_t flCycleTime;

		if (m_pPlayer->m_iFOV == 90)
			flCycleTime = 0.4s;
		else
			flCycleTime = 0.45s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			Fire(0.85, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 140)
			Fire(0.3, flCycleTime, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 10)
			Fire(0.1, flCycleTime, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			Fire(0, flCycleTime, FALSE);
		else
			Fire(0.001, flCycleTime, FALSE);
	}

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 210;
		return 165;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.1;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.375 + 0.6;

			float flMaxAccuracy = m_pPlayer->m_iFOV == 90 ? 0.9 : 1.0;

			if (m_pPlayer->m_iFOV == 90)
				flMaxAccuracy = 0.9;
			else
				flMaxAccuracy = 1.0;

			if (m_flAccuracy > flMaxAccuracy)
				m_flAccuracy = flMaxAccuracy;
		}

		m_flLastFire = gpGlobals->time;
		flSpread *= (1.0 - m_flAccuracy);

		if (m_iClip <= 0)
		{
			if (m_fFireOnEmpty)
			{
				PlayEmptySound();
				m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
			}

			return;
		}

		m_iClip--;
		m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

		m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 5, BULLET_PLAYER_338MAG, GetDamage(), 0.982, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), FALSE, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

		if (m_pPlayer->pev->fov != 90)
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 10.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 2.0);
		}
		else
		{
			m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 8.0, 9.0) + m_pPlayer->pev->punchangle.x * 0.25;
			m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -3.0, 3.0);
		}
	}

	float GetDamage() const
	{
		float flDamage = 115.0f;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 250.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 550.0f;
#endif
		return flDamage;
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

};
LINK_ENTITY_TO_CLASS(weapon_as50g, CAS50G)
}