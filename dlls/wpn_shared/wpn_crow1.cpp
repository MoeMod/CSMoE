/*
crow1.cpp
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
    class CCrow1 : public LinkWeaponTemplate< CCrow1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFirePistol,
            TRecoilPunch,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_crow1.mdl";
    static constexpr const char *P_Model = "models/p_crow1.mdl";
    static constexpr const char *W_Model = "models/w_crow1.mdl";
    static constexpr const char *EventFile = "events/crow1.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crow1";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 50;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT_EMPTY,
        ANIM_RELOAD_START,
        ANIM_RELOAD_FAST,
        ANIM_RELOAD_SLOW,
        ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.0f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 0;
    static constexpr const auto& SpreadCalcNotOnGround = 1.0 * (1 - A);
    static constexpr const auto& SpreadCalcWalking = 0.17 * (1 - A);
    static constexpr const auto& SpreadCalcDucking = 0.075 * (1 - A);
    static constexpr const auto& SpreadCalcDefault = 0.1 * (1 - A);
    static constexpr const auto& CycleTime = 0.2s;
    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 75;
    static constexpr int DamageZBS = 75;
    static constexpr float AccuracyDefault = 0.525f;
    static constexpr const auto& AccuracyCalc = A - (0.325 - T) * 0.285;
    static constexpr float AccuracyMin = 0.525;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.76;
    static constexpr auto BulletType = BULLET_PLAYER_9MM;
    static constexpr int Penetration = 1;
    KnockbackData KnockBack = { 350, 300, 300, 150, 0.4f };
    std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
    
    bool bCheckingReload;
    bool bCheckingCharge;
    bool bCanCharge;
    time_point_t m_flNextCheckReload;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "9MMBox";
        p->iMaxAmmo1 = 120;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 50;
        p->iSlot = 1;
        p->iPosition = 2;
        p->iId = m_iId = WEAPON_CROW1;
        p->iFlags = 0;
        p->iWeight = GLOCK18_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_9mmbox" , 20 }; }
#endif
    float GetArmorRatioModifier() override { return 1.0; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 0 }; }

    void Precache()
    {
        //crow1_shoot_empty
        //PRECACHE_MODEL("sprites/muzzleflash3.spr");
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
                m_flAccuracy = AccuracyDefault;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }
        }
        else
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            if (DefaultReload(MaxClip, ANIM_RELOAD_SLOW, 2.5s))
            {
                m_flAccuracy = AccuracyDefault;
                m_bDelayFire = false;
                m_iShotsFired = 0;
            }       
        }
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
    }

    void SecondaryAttack()
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            FireB(0.08 * (1 - m_flAccuracy), 0.08s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            FireB(0.04 * (1 - m_flAccuracy), 0.08s, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            FireB(0.02 * (1 - m_flAccuracy), 0.08s, FALSE);
        else
            FireB(0.03 * (1 - m_flAccuracy), 0.08s, FALSE);
    }

    void FireB(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.0) + 0.5;

        if (m_flAccuracy > 2.0)
            m_flAccuracy = 2.0;
        else if (m_flAccuracy < 0.9)
            m_flAccuracy = 0.9;

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
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.76, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, TRUE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.4, 0.5, 0.45, 0.15, 6, 4, 4);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.3, 0.45, 0.5, 0.045, 5, 2.75, 5);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.675, 0.3, 0.35, 0.0125, 4, 2.25, 5);
        else
            KickBack(0.725, 0.35, 0.4, 0.015, 4.25, 2, 5);
    }

};
LINK_ENTITY_TO_CLASS(weapon_crow1, CCrow1)
}