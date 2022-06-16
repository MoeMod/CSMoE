/*
m950se.cpp
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
    class CM950SE : public LinkWeaponTemplate< CM950SE,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
			TSecondaryAttackZoom,
            TFirePistol,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

	static constexpr int ZoomFOV = 65;
    static constexpr const auto &DefaultReloadTime = 3.75s;
    static constexpr const char *V_Model = "models/v_m950se.mdl";
    static constexpr const char *P_Model = "models/p_m950se.mdl";
    static constexpr const char *W_Model = "models/w_m950se.mdl";
    static constexpr const char *EventFile = "events/m950.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_m950se";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 58;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.0f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.1*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.04*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.03*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.04*(1-A);

	static constexpr const auto& CycleTime = 0.15s;
    static constexpr int DamageDefault = 22;
    static constexpr int DamageZB = 49;
    static constexpr int DamageZBS = 57;

    static constexpr float AccuracyDefault = 0.9;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 450.0f, 350.0f, 400.0f, 300.0f, 0.3f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_GLOCK18); }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "9mm";
		p->iMaxAmmo1 = MAX_AMMO_9MM;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 58;
		p->iSlot = 1;
		p->iPosition = 2;
		p->iId = m_iId = WEAPON_M950SE;
		p->iFlags = 0;
		p->iWeight = GLOCK18_WEIGHT;

		return 1;
	}

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.29 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.9;
			else if (m_flAccuracy < 0.55)
				m_flAccuracy = 0.55;
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
		m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, FALSE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.2f, 0.8f, 0.5f, 0.5f, 7.0f, 5.0f, 4);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.7f, 0.5f, 0.325f, 0.065f, 4.0f, 3.0f, 5);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.55f, 0.3f, 0.25f, 0.035f, 3.0f, 2.0f, 6);
		else
			KickBack(0.6f, 0.35f, 0.3f, 0.05f, 3.25f, 2.0f, 5);
    }
    		 
	float GetDamage() const
	{
		float flDamage = 22.0;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 49.0f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 57.0f;
#endif
		return flDamage;
	}
};
LINK_ENTITY_TO_CLASS(weapon_m950se, CM950SE)
}