/*
dualuzi.cpp
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
    class CDualUZI : public LinkWeaponTemplate< CDualUZI,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

#define WPNSTATE_DUALUZI_LEFT	(1<<0)

    static constexpr int ZoomFOV = 65;
    static constexpr const char *V_Model = "models/v_dualuzi.mdl";
    static constexpr const char *P_Model = "models/p_dualuzi.mdl";
    static constexpr const char *W_Model = "models/w_dualuzi.mdl";
    static constexpr const char *EventFile = "events/dualuzi.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_dualuzi";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 80;

    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 60;
    static constexpr int DamageZBS = 135;

    KnockbackData KnockBack = { 350.0f, 300.0f, 350.0f, 200.0f, 0.5f };

	enum
	{
        DUALUZI_IDLE,
        DUALUZI_IDLE2,
        DUALUZI_IDLE_LEFTEMPTY,
        DUALUZI_SHOOT_LEFT1,
        DUALUZI_SHOOT_LEFT2,
        DUALUZI_SHOOT_LEFT3,
        DUALUZI_SHOOT_LEFT4,
        DUALUZI_SHOOT_LEFT5,
        DUALUZI_SHOOT_LEFTLAST,
        DUALUZI_SHOOT_RIGHT1,
        DUALUZI_SHOOT_RIGHT2,
        DUALUZI_SHOOT_RIGHT3,
        DUALUZI_SHOOT_RIGHT4,
        DUALUZI_SHOOT_RIGHT5,
        DUALUZI_SHOOT_RIGHTLAST,
        DUALUZI_RELOAD,
        DUALUZI_DRAW,
        DUALUZI_ON_SHOULDER,
        DUALUZI_ON_SHOULDER_IDLE,
        DUALUZI_ON_SHOULDER_SHOOT_LEFT,
        DUALUZI_ON_SHOULDER_SHOOT_LEFTLAST,
        DUALUZI_ON_SHOULDER_LEFTEMPTY,
        DUALUZI_ON_SHOULDER_SHOOT_RIGHT,
        DUALUZI_ON_SHOULDER_SHOOT_RIGHTLAST,
        DUALUZI_ON_SHOULDER_RELOAD,
        DUALUZI_OFF_SHOULDER,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.45f;

    int m_usFireDualUZILeft;
    int m_usFireDualUZIRight;
    int m_iState;
    time_point_t m_flTimeWeaponIdle2;

    void Spawn()
    {
        Base::Spawn();
        m_bDelayFire = false;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "9mm";
        p->iMaxAmmo1 = MAX_AMMO_9MM;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 80;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_DUALUZI;
        p->iFlags = 0;
        p->iWeight = TMP_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_TMP); }
#endif
    float GetArmorRatioModifier() override { return 1.45; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");

        m_usFireDualUZILeft = PRECACHE_EVENT(1, "events/dualuzi_left.sc");
        m_usFireDualUZIRight = PRECACHE_EVENT(1, "events/dualuzi_right.sc");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.1;
        m_iShotsFired = 0;
        m_bDelayFire = false;
        m_iState = 0;
        m_flTimeWeaponIdle2 = invalid_time_point;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_DUALUZI_LEFT;

        BOOL bResult = DefaultDeploy("models/v_dualuzi.mdl", "models/p_dualuzi.mdl", DUALUZI_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
        m_pPlayer->ResetMaxSpeed();

        return bResult;
    }

    void SecondaryAttack()
    {   
        if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
            return;

        if (!m_iState)
        {       
            SendWeaponAnim(DUALUZI_ON_SHOULDER, UseDecrement() != FALSE);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle2 = gpGlobals->time + 2.0s;
        }
        else
        {
            SendWeaponAnim(DUALUZI_OFF_SHOULDER, UseDecrement() != FALSE);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
            m_flTimeWeaponIdle2 = gpGlobals->time + 2.0s;
        }    
    }

    void ItemPostFrame(void)
    {
        CBasePlayerWeapon::ItemPostFrame();

        if (m_flTimeWeaponIdle2 != invalid_time_point && gpGlobals->time > m_flTimeWeaponIdle2)
        {
            if (!m_iState)
                m_iState = 1;
            else
                m_iState = 0;

            m_flTimeWeaponIdle2 = invalid_time_point;
        }
    }

    void PrimaryAttack(void)
    {
        if (m_iState)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                DualUZIFire((0.1) * m_flAccuracy, 0.06s, FALSE);
            else
                DualUZIFire((0.015) * m_flAccuracy, 0.06s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                DualUZIFire((0.2) * m_flAccuracy, 0.06s, FALSE);
            else
                DualUZIFire((0.025) * m_flAccuracy, 0.06s, FALSE);
        }
       

    }

    void DualUZIFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (m_iState)
        {
            m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 250.0 + 0.2;

            if (m_flAccuracy > 0.6)
                m_flAccuracy = 0.6;
        }
        else
        {
            m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.3;

            if (m_flAccuracy > 1.0)
                m_flAccuracy = 1.0;
        }

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

        unsigned short eventindex;
        if (m_iWeaponState & WPNSTATE_DUALUZI_LEFT)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_iWeaponState &= ~WPNSTATE_DUALUZI_LEFT;
            eventindex = m_usFireDualUZILeft;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_DUALUZI_LEFT;
            eventindex = m_usFireDualUZIRight;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.88, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iState == 1, m_iClip);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (m_iState)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(0.85, 0.4, 0.3, 0.04, 4.5, 3.0, 8);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.3, 0.2, 0.15, 0.03, 3.0, 2.25, 7);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.19, 0.09, 0.09, 0.009, 2.0, 1.0, 5);
            else
                KickBack(0.21, 0.11, 0.11, 0.011, 2.1, 1.1, 6);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(0.9, 0.5, 0.4, 0.04, 4.75, 3.25, 8);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.35, 0.25, 0.2, 0.03, 3.25, 2.5, 7);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.23, 0.13, 0.13, 0.013, 2.2, 1.2, 5);
            else
                KickBack(0.25, 0.15, 0.15, 0.015, 2.3, 1.3, 6);
        }    
    }


    float GetDamage(void)
    {
        float flDamage; 
        flDamage = 30.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 60.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 135.0f;
#endif
        return flDamage;
        
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim;
        duration_t flDeloadTime;

        if (!m_iState)
        {
            iAnim = DUALUZI_RELOAD;
            flDeloadTime = 3.33s;
        }
        else
        {
            iAnim = DUALUZI_ON_SHOULDER_RELOAD;
            flDeloadTime = 4.16s;
        }

        if (DefaultReload(80, iAnim, flDeloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.0;
            m_iShotsFired = 0;
        }
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 5.8s;

        if (m_iClip)
        {          
            if (m_iState)
            {
                SendWeaponAnim(m_iClip == 1 ? DUALUZI_ON_SHOULDER_LEFTEMPTY : DUALUZI_ON_SHOULDER_IDLE, UseDecrement() != FALSE);              
            }            
            else
                SendWeaponAnim(m_iClip == 1 ? DUALUZI_IDLE_LEFTEMPTY : DUALUZI_IDLE + RANDOM_LONG(0, 1), UseDecrement() != FALSE);
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_dualuzi, CDualUZI)
}