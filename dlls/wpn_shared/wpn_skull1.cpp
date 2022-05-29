/*
skull1.cpp
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
    class CSKULL1 : public LinkWeaponTemplate< CSKULL1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TRecoilPunch,
            TWeaponIdleDefault,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.62s;
    static constexpr const char *V_Model = "models/v_skull1.mdl";
    static constexpr const char *P_Model = "models/p_skull1.mdl";
    static constexpr const char *W_Model = "models/w_skull1.mdl";
    static constexpr const char *EventFile = "events/skull1.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_skull1";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 7;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_IDLE_LEFTEMPTY,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SP_SHOOT1,
		ANIM_SP_SHOOT2,
        ANIM_RELOAD,
        ANIM_DRAW,
	};
	static constexpr const auto& SpreadCalcNotOnGround = 1.5*A;
	static constexpr const auto& SpreadCalcWalking = 0.02*A;
	static constexpr const auto& SpreadCalcDucking = 0.01*A;
	static constexpr const auto& SpreadCalcDefault = 0.015*A;
	static constexpr const auto& CycleTime = 0.24s;
	static constexpr int DamageDefault = 57;
	static constexpr int DamageZB = 80;
	static constexpr int DamageZBS = 80;
	static constexpr float AccuracyDefault = 0.95f;
	static constexpr const auto& AccuracyCalc = A - (0.275 - T) * 0.3;
	static constexpr float AccuracyMin = 0.5;
	static constexpr float AccuracyMax = 0.95;
	static constexpr float RangeModifier = 0.8;
	static constexpr auto BulletType = BULLET_PLAYER_50AE;
	static constexpr int Penetration = 2;
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
	KnockbackData KnockBack = { 750.0f, 500.0f, 600.0f, 500.0f, 0.4f };
	KnockbackData GetKnockBackData() override
	{
		if (m_iWeaponState)
			return { 650.0f, 450.0f, 300.0f, 300.0f, 0.4f };
		else
			return { 750.0f, 550.0f, 450.0f, 350.0f, 0.5f };
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50aeaz" , 70 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	int GetItemInfo(ItemInfo* p)
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "50AEaz";
		p->iMaxAmmo1 = 35;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 7;
		p->iSlot = 1;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_SKULL1;
		p->iFlags = 0;
		p->iWeight = DEAGLE_WEIGHT;

		return 1;
	}

	void PrimaryAttack()
	{
		m_iWeaponState = 0;
		Base::PrimaryAttack();
	}

	void SecondaryAttack()
	{
		m_iWeaponState = 1;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			FireB(0.17 * m_flAccuracy, 0.15s, FALSE);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			FireB(0.09 * m_flAccuracy, 0.15s, FALSE);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			FireB(0.01 * m_flAccuracy, 0.15s, FALSE);
		else
			FireB(0.02 * m_flAccuracy, 0.15s, FALSE);
	}

    void FireB(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
		if (m_flLastFire != invalid_time_point)
		{
			m_flAccuracy -= (0.275 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.325;

			if (m_flAccuracy > 0.9)
				m_flAccuracy = 0.9;
			else if (m_flAccuracy < 0.3)
				m_flAccuracy = 0.3;
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
		Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_50AE, GetDamageB(), 0.8, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

		int flags;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif

		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), 5, TRUE);
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
		if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
			m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

		if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(2.0, 1.0, 0.5, 0.25, 8.0, 5.0, 3);
		else if (m_pPlayer->pev->velocity.Length2D() > 0)
			KickBack(0.8, 0.3, 0.1, 0.15, 5.5, 3.2, 3);
		else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
			KickBack(0.2, 0.1, 0.07, 0.01, 3.0, 1.5, 7);
		else
			KickBack(0.35, 0.25, 0.15, 0.03, 3.5, 2.0, 7);
    }
    		 
	float GetDamageB() const
	{
		float flDamage = 34;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 72;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage = 130;
#endif
		return flDamage;
	}
};
LINK_ENTITY_TO_CLASS(weapon_skull1, CSKULL1)
}