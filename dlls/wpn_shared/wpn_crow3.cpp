/*
crow3.cpp
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
    class CCrow3 : public LinkWeaponTemplate< CCrow3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
            TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_crow3.mdl";
    static constexpr const char *P_Model = "models/p_crow3.mdl";
    static constexpr const char *W_Model = "models/w_crow3.mdl";
    static constexpr const char *EventFile = "events/crow3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crow3";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 64;
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
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr const auto &SpreadCalcNotOnGround = 0.2*A;
    static constexpr const auto &SpreadCalcDefault = 0.05*A;

    static constexpr const auto& CycleTime = 0.07s;
 
    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 68;
    static constexpr int DamageZBS = 110;
    static constexpr const auto &AccuracyCalc = (N * N / 200.0) + 0.49;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 0.8;
    static constexpr float RangeModifier = 0.8;
    static constexpr auto BulletType = BULLET_PLAYER_9MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };

    KickBackData KickBackWalking = { 0.5, 0.275, 0.2, 0.03, 3.0, 2.0, 10 };
    KickBackData KickBackNotOnGround = { 0.9, 0.475, 0.35, 0.0425, 5.0, 3.0, 6 };
    KickBackData KickBackDucking = { 0.225, 0.15, 0.1, 0.015, 2.0, 1.0, 10 };
    KickBackData KickBackDefault = { 0.22, 0.172, 0.122, 0.02, 2.22, 1.22, 10 };
    
    bool bCheckingReload;
    bool bCheckingCharge;
    bool bCanCharge;
    time_point_t m_flNextCheckReload;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "9mm";
        p->iMaxAmmo1 = MAX_AMMO_9MM;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 64;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CROW3;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_MP5N); }
#endif
    float GetArmorRatioModifier() override { return 1.1; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash2.spr");
        PRECACHE_MODEL("sprites/muzzleflash17.spr");
        Base::Precache();
    }

    BOOL Deploy() override
    {
        bCheckingReload = false;
        bCheckingCharge = false;
        bCanCharge = false;
        m_flNextCheckReload = invalid_time_point;
        return Base::Deploy();
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
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            SendWeaponAnim(ANIM_RELOAD_START, UseDecrement() != FALSE);
            m_flNextCheckReload = gpGlobals->time + 0.8s;
            m_pPlayer->m_flNextAttack = 0.5s;   //oldbutton is useless now need to avoid double time IN_RELOAD
            m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
            bCheckingReload = true;
            bCanCharge = true;
            return;
        }

        if (bCanCharge)
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            if (DefaultReload(MaxClip, ANIM_RELOAD_FAST, 1.0s))
            {
                m_flAccuracy = 0;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }
        }
        else
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            if (DefaultReload(MaxClip, ANIM_RELOAD_SLOW, 2.4s))
            {
                m_flAccuracy = 0;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }       
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_crow3, CCrow3)
}