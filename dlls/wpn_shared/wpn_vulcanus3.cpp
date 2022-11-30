/*
vulcanus3.cpp
Copyright (C) 2022 JustANoobQAQ

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
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CVulcanus3 : public LinkWeaponTemplate< CVulcanus3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TPrimaryAttackRifle
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 3.2s;
    static constexpr const char *V_Model = "models/v_vulcanus3.mdl";
    static constexpr const char *P_Model = "models/p_vulcanus3.mdl";
    static constexpr const char *W_Model = "models/w_vulcanus3.mdl";
    static constexpr const char *EventFile = "events/vulcanus3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_vulcanus3";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 45;

    static constexpr const auto& SpreadCalcNotOnGround = 0.2 * A;
    static constexpr const auto& SpreadCalcDefault = 0.03 * A;

    static constexpr const auto& CycleTime = 0.09s;

#define WPNSTATE_VULCANUS3_MODEA 1
#define WPNSTATE_VULCANUS3_MODEB 2
#define WPNSTATE_VULCANUS3_MODEC 3
#define WPNSTATE_VULCANUS3_MODED 4

	enum
	{
            ANIM_IDLE,
            ANIM_IDLEA,
            ANIM_IDLEB,
            ANIM_IDLEC,
            ANIM_IDLED,
            ANIM_SHOOT,
            ANIM_SHOOTA,
            ANIM_SHOOTB,
            ANIM_SHOOTC,
            ANIM_SHOOTD,
            ANIM_CHANGEA,
            ANIM_CHANGEB,
            ANIM_CHANGEC,
            ANIM_CHANGED,
            ANIM_CHANGE_EMPTYA,
            ANIM_CHANGE_EMPTYB,
            ANIM_CHANGE_EMPTYC,
            ANIM_CHANGE_EMPTYD,
            ANIM_RELOAD,
            ANIM_RELOADA,
            ANIM_RELOADB,
            ANIM_RELOADC,
            ANIM_RELOADD,
            ANIM_DRAW,
            ANIM_DRAWA,
            ANIM_DRAWB,
            ANIM_DRAWC,
            ANIM_DRAWD,
	};
    static constexpr float PlayerMaxSpeed = 245;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 27;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 105;

    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 1.0;

    KnockbackData KnockBack = { 400.0f, 300.0f, 300.0f, 100.0f, 0.35f };

    
    int m_iChargeShotsFired;
    time_point_t m_flCheckCharge;
    time_point_t m_flNextReload;

    bool HasSecondaryAttack() override { return false; }

    void Spawn()
    {
        m_iChargeShotsFired = 0;
        m_flCheckCharge = invalid_time_point;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "45acp";
        p->iMaxAmmo1 = MAX_AMMO_45ACP;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 15;
        p->iId = m_iId = WEAPON_VULCANUS3;
        p->iFlags = 0;
        p->iWeight = UMP45_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_UMP45); }
#endif
    float GetArmorRatioModifier() override { return 1.2; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash2.spr");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = AccuracyDefault;
        m_bDelayFire = false;

        int iAnim = ANIM_DRAW + m_iWeaponState;

        return DefaultDeploy(V_Model, P_Model, iAnim, "carbine", UseDecrement() != FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 240.0) + 0.3;
        if (m_flAccuracy > AccuracyMax)
            m_flAccuracy = AccuracyMax;
       
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

        if (m_iWeaponState < WPNSTATE_VULCANUS3_MODED)
            m_iChargeShotsFired++;

        if(m_iWeaponState)
            m_flCheckCharge = gpGlobals->time + GetModeTime();

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

        float flDamage = GetDamage();
        flDamage += GetRatio() * m_iWeaponState;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, flDamage, 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        if (m_iChargeShotsFired >= 9 && m_iWeaponState < WPNSTATE_VULCANUS3_MODED)
        {
            m_iWeaponState++;
            m_iChargeShotsFired = 0;
        }

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, m_iChargeShotsFired == 9);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.8, 0.45, 0.4, 0.03, 6.0, 3.5, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.45, 0.25, 0.2, 0.0275, 3.0, 2.0, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.225, 0.15, 0.125, 0.015, 2.0, 1.0, 10);
        else
            KickBack(0.275, 0.175, 0.15, 0.02, 2.5, 1.25, 9);
    }

    void ItemPostFrame() override
    {
        if (m_fInReload && m_flNextReload != invalid_time_point && m_flNextReload < gpGlobals->time)
        {
            // complete the reload.
            int j = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);
#ifndef CLIENT_DLL
            j = m_pPlayer->m_pModStrategy->AdjustDefaultReloadClip(this, j);
#endif

            // Add them to the clip
            m_iClip += j;
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= j;
#ifndef CLIENT_DLL
            m_pPlayer->TabulateAmmo();
#endif
            m_fInReload = FALSE;
            m_flNextReload = invalid_time_point;
        }

        if (m_fInReload)
            return;

        if (m_flCheckCharge != invalid_time_point && m_flCheckCharge < gpGlobals->time)
        {
            if (m_iChargeShotsFired < 9)
            {
                m_flCheckCharge = invalid_time_point;
                m_iWeaponState = 0;
                m_iChargeShotsFired = 0;

                int iAnim = m_iWeaponState + ANIM_CHANGE_EMPTYA;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
                SendWeaponAnim(iAnim, UseDecrement() != FALSE);
            }
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }

    void WeaponIdle() override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        int iAnim = m_iWeaponState + ANIM_IDLE;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim = m_iWeaponState + ANIM_RELOAD;

        if (DefaultReload(MaxClip, iAnim, DefaultReloadTime, 1))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flNextReload = gpGlobals->time + DefaultReloadTime;
            m_flAccuracy = 0.9;
        }
    }

    float GetDamage() const
    {
        float flDamage = 28.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 34.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 70.0f;
#endif
        return flDamage;
    }

    float GetRatio() const
    {
        float flDamage = 1.25f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 17.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 55.0f;
#endif
        return flDamage;
    }

    duration_t GetModeTime()
    {
        duration_t flTime;
        switch (m_iWeaponState)
        {
        case WPNSTATE_VULCANUS3_MODEA: flTime = 6.0s; break;
        case WPNSTATE_VULCANUS3_MODEB: flTime = 5.0s; break;
        case WPNSTATE_VULCANUS3_MODEC: flTime = 4.4s; break;
        case WPNSTATE_VULCANUS3_MODED: flTime = 3.7s; break;
        default:
            flTime = 0.0s;
            break;
        }

        return flTime;
    }

};
LINK_ENTITY_TO_CLASS(weapon_vulcanus3, CVulcanus3)
}