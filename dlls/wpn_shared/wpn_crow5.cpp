/*
crow5.cpp
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
    class CCrow5 : public LinkWeaponTemplate< CCrow5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TSecondaryAttackZoom,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr int ZoomFOV = 65;
    static constexpr const char *V_Model = "models/v_crow5.mdl";
    static constexpr const char *P_Model = "models/p_crow5.mdl";
    static constexpr const char *W_Model = "models/w_crow5.mdl";
    static constexpr const char *EventFile = "events/crow5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crow5";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_RELOAD_START,
        ANIM_RELOAD_FAST,
        ANIM_RELOAD_SLOW,
        ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.05*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.042*A + 0.02;
    static constexpr const auto &SpreadCalcDefault = 0.037*A + 0.017;

    static constexpr const auto& CycleTime1 = 0.095s;
    static constexpr const auto& CycleTime2 = 0.18s;
    duration_t CycleTime = CycleTime1;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 68;
    static constexpr int DamageZBS = 95;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.2;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.6;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 450.0f, 350.0f, 400.0f, 200.0f, 0.6f };

	KickBackData KickBackWalking = { 1.4, 0.3, 0.065, 0.04, 3.0, 2.0, 6 };
	KickBackData KickBackNotOnGround = { 1.6, 0.6, 0.4, 0.125, 5.0, 3.0, 8 };
	KickBackData KickBackDucking = { 0.5, 0.13, 0.15, 0.001, 1.5, 0.3, 15 };
	KickBackData KickBackDefault = { 0.9, 0.2, 0.04, 0.022, 2.0, 1.5, 8 };
    
    bool bCheckingReload;
    bool bCheckingCharge;
    bool bCanCharge;
    time_point_t m_flNextCheckReload;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = M249_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CROW5;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

    BOOL Deploy() override
    {
        bCheckingReload = false;
        bCheckingCharge = false;
        bCanCharge = false;
        m_flNextCheckReload = invalid_time_point;
        return Base::Deploy();
    }

    void PrimaryAttack() override
    {
        if (m_pPlayer->pev->fov != 90)
            CycleTime = CycleTime2;
        else
            CycleTime = CycleTime1;
        return Base::PrimaryAttack();
    }

    void ItemPostFrame() override
    {
        if (m_pPlayer->pev->button & IN_RELOAD)
        {
            if (bCheckingReload)
            {
                if(bCheckingCharge)
                    bCanCharge = true;
                else
                    bCanCharge = false;
            }
        }

        if (m_flNextCheckReload != invalid_time_point && m_flNextCheckReload < gpGlobals->time)
        {
            if (bCheckingReload)
            {
                if (!bCheckingCharge && bCanCharge)
                {
                    bCheckingCharge = true;
                    bCanCharge = false;
                    m_flNextCheckReload = gpGlobals->time + 0.2s;
                    m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
                }
                else
                {
                    Reload();
                    bCheckingCharge = false;
                    bCheckingReload = false;
                    
                }
            }
        }
        
        return CBasePlayerWeapon::ItemPostFrame();
    }


    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iClipSize = MaxClip;
        int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

        if (!j)
            return;

        if (!bCheckingReload)
        {
            if (m_pPlayer->m_iFOV != 90)
                SecondaryAttack();
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            SendWeaponAnim(ANIM_RELOAD_START, UseDecrement() != FALSE);
            m_flNextCheckReload = gpGlobals->time + 1.36s;
            m_pPlayer->m_flNextAttack = 0.5s;   //oldbutton is useless now need to avoid double time IN_RELOAD
            m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.36s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.36s;
            bCheckingReload = true;
            bCanCharge = true;
            return;
        }

        if (bCanCharge)
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            if (DefaultReload(100, ANIM_RELOAD_FAST, 1.6s))
            {
                m_flAccuracy = 0.2;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }
        }
        else
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            if (DefaultReload(100, ANIM_RELOAD_SLOW, 3.0s))
            {
                m_flAccuracy = 0.2;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }       
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_crow5, CCrow5)
}