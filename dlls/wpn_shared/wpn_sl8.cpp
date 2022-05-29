/*
sl8.cpp
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
    class CSL8 : public LinkWeaponTemplate< CSL8,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG550>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
			TSecondaryAttackSniperZoom2,
            TPrimaryAttackRifle,
            TFireRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.35s;
	static constexpr int ZoomFOV1 = 40;
	static constexpr int ZoomFOV2 = 15;
    static constexpr const char *V_Model = "models/v_sl8.mdl";
    static constexpr const char *P_Model = "models/p_sl8.mdl";
    static constexpr const char *W_Model = "models/w_sl8.mdl";
    static constexpr const char *EventFile = "events/sl8.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_sl8";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 20;
	enum
	{
		ANIM_IDLE1 = 0,	
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr const auto &SpreadCalcNotOnGround = (1 - A) * 0.45;
    static constexpr const auto &SpreadCalcWalking = 0.12 + 0 * A;
    static constexpr const auto &SpreadCalcDucking = 0.05 * (1 - A);
    static constexpr const auto &SpreadCalcDefault = 0.03 * (1 - A);
	static constexpr const auto& CycleTime = 0.225s;
    static constexpr int DamageDefault = 62;
    static constexpr int DamageZB = 73;
    static constexpr int DamageZBS = 73;
    static constexpr float AccuracyDefault = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "556Nato";
		p->iMaxAmmo1 = MAX_AMMO_556NATO;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 20;
		p->iSlot = 0;
		p->iPosition = 16;
		p->iId = m_iId = WEAPON_SL8;
		p->iFlags = 0;
		p->iWeight = SG550_WEIGHT;

		return 1;
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_SG550); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { 0, 6, 4, 0 }; }


	float GetArmorRatioModifier() override { return 1.5; }

	float GetMaxSpeed(void) override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 250;

		return 150;
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		if (m_pPlayer->pev->fov == 90)
			flSpread += 0.025;

		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy = ((gpGlobals->time - m_flLastFire) / 1s) * 0.35 + 0.55;

			if (m_flAccuracy > 0.98)
				m_flAccuracy = 0.98;
		}

		m_flLastFire = gpGlobals->time;

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

		Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.98, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.8s;

		m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 0.55, 1.0) + m_pPlayer->pev->punchangle.x * 0.25;
		m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.65, 0.95);
	}
};

LINK_ENTITY_TO_CLASS(weapon_sl8, CSL8)
}