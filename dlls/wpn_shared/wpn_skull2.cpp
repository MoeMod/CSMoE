/*
skull2.cpp
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
    class CSKULL2 : public LinkWeaponTemplate< CSKULL2,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_ELITE>::template type,
            TPrecacheEvent
    >
    {
    public:

#define WPNSTATE_SKULL2_LEFT	(1<<0)

    static constexpr const char *V_Model = "models/v_skull2.mdl";
    static constexpr const char *P_Model = "models/p_skull2.mdl";
    static constexpr const char *W_Model = "models/w_skull2.mdl";
    static constexpr const char *EventFile = "events/skull2.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_skull2";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 14;

	enum
	{
        SKULL2_IDLE1,
        SKULL2_RELOAD1,
        SKULL2_DRAW1,
        SKULL2_SHOOT1,
        SKULL2_SHOOT1_EMPTY,
        SKULL2_IDLE2,
        SKULL2_IDLE2_LEFTEMPTY,
        SKULL2_SHOOT_LEFT1,
        SKULL2_SHOOT_LEFT2,
        SKULL2_SHOOT_LEFTLAST,
        SKULL2_SHOOT_RIGHT1,
        SKULL2_SHOOT_RIGHT2,
        SKULL2_SHOOT_RIGHTLAST,
        SKULL2_RELOAD2,
        SKULL2_DRAW2,
        SKULL2_DRAW3,
        SKULL2_DRAW4,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.5f;
    KnockbackData KnockBack = { 650.0f, 550.0f, 500.0f, 300.0f, 0.5f };

    KnockbackData GetKnockBackData() override
    {
        if (m_iState)
            return { 650.0f, 550.0f, 500.0f, 300.0f, 0.5f };
        else
            return { 800.0f, 650.0f, 500.0f, 400.0f, 0.5f };
    }

    int m_usFireSKULL2Left;
    int m_usFireSKULL2Right;
    int m_iState;
    time_point_t m_flTimeWeaponIdle2;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "50AEaz";
        p->iMaxAmmo1 = 35;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = m_iState ? 7 : 14;
        p->iSlot = 1;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_SKULL2;
        p->iFlags = 0;
        p->iWeight = DEAGLE_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50aeaz" , 70 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");
        PRECACHE_MODEL("models/p_skull2_2.mdl");


        m_usFireSKULL2Left = PRECACHE_EVENT(1, "events/skull2_left.sc");
        m_usFireSKULL2Right = PRECACHE_EVENT(1, "events/skull2_right.sc");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.5;
        m_flTimeWeaponIdle2 = invalid_time_point;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_SKULL2_LEFT;

        if(!m_iState)
            return DefaultDeploy("models/v_skull2.mdl", "models/p_skull2_2.mdl", SKULL2_DRAW2, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
        else
            return DefaultDeploy("models/v_skull2.mdl", "models/p_skull2.mdl", SKULL2_DRAW1, AnimExtension, UseDecrement() != FALSE);
    }

    void SecondaryAttack()
    {
        if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
            return;

        if (!m_iState)
            SendWeaponAnim(SKULL2_DRAW4, UseDecrement() != FALSE);       
        else
            SendWeaponAnim(SKULL2_DRAW3, UseDecrement() != FALSE);    

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.0s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.0s;
        m_flTimeWeaponIdle2 = gpGlobals->time + 3.0s;
    }

    void ItemPostFrame() override
    {      
        CBasePlayerWeapon::ItemPostFrame();

        if (m_flTimeWeaponIdle2 != invalid_time_point && gpGlobals->time > m_flTimeWeaponIdle2)
        {
            m_flTimeWeaponIdle2 = invalid_time_point;
            if (!m_iState)
            {
                m_iState = 1;
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_skull2.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, AnimExtension);

                
                int MaxClip = 7;

                if (m_iClip >= MaxClip)
                {
                    int delta = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

                    // Add them to the clip
                    m_iClip += delta;
                    m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= delta;
#ifndef CLIENT_DLL
                    m_pPlayer->TabulateAmmo();
#endif
                    m_fInReload = FALSE;
                }            
            }
            else
            {
                m_iState = 0;
                m_pPlayer->pev->weaponmodel = MAKE_STRING("models/p_skull2_2.mdl");

                strcpy(m_pPlayer->m_szAnimExtention, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2");

                int MaxClip = 14;

      
                int delta = Q_min(MaxClip - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

                // Add them to the clip
                if (abs(delta) >= 7)
                    delta = delta < 0 ? -7 : 7;

                m_iClip += delta;
                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= delta;
#ifndef CLIENT_DLL
                m_pPlayer->TabulateAmmo();
#endif
                m_fInReload = FALSE;
                
            }
        }
    }

    void PrimaryAttack(void)
    {
        if (!m_iState)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                SKULL2FireA(1.5 * m_flAccuracy, 0.16s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                SKULL2FireA(0.03 * m_flAccuracy, 0.16s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                SKULL2FireA(0.01 * m_flAccuracy, 0.16s, FALSE);
            else
                SKULL2FireA(0.02 * m_flAccuracy, 0.16s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                SKULL2FireB(1.5 * m_flAccuracy, 0.24s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                SKULL2FireB(0.02 * m_flAccuracy, 0.24s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                SKULL2FireB(0.01 * m_flAccuracy, 0.24s, FALSE);
            else
                SKULL2FireB(0.015 * m_flAccuracy, 0.24s, FALSE);
        }
     
    }

    void SKULL2FireA(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iShotsFired++;

        if (m_iShotsFired > 1)
            return;

        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.275 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

            if (m_flAccuracy > 0.9)
                m_flAccuracy = 0.9;
            else if (m_flAccuracy < 0.5)
                m_flAccuracy = 0.5;
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

        unsigned short eventindex;
        if (m_iWeaponState & WPNSTATE_SKULL2_LEFT)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_iWeaponState &= ~WPNSTATE_SKULL2_LEFT;
            eventindex = m_usFireSKULL2Left;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_SKULL2_LEFT;
            eventindex = m_usFireSKULL2Right;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.975, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), TRUE, m_iClip);
        
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        m_pPlayer->pev->punchangle.x -= 2;
    }

    void SKULL2FireB(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iShotsFired++;

        if (m_iShotsFired > 1)
            return;

        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.275 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

            if (m_flAccuracy > 0.95)
                m_flAccuracy = 0.95;
            else if (m_flAccuracy < 0.5)
                m_flAccuracy = 0.5;
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
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.975, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);

        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        m_pPlayer->pev->punchangle.x -= 2;
    }


    float GetDamage(void)
    {
        float flDamage;
        if (!m_iState)
        {
            flDamage = 54.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 84.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 195.0f;
#endif
            return flDamage;
        }
        else
        {
            flDamage = 45.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 68.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 185.0f;
#endif
            return flDamage;
        }
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iMaxClip, iAnim;
        duration_t flDeloadTime;

        if (!m_iState)
        {
            iMaxClip = 14;
            iAnim = SKULL2_RELOAD2;
            flDeloadTime = 3.3s;
        }
        else
        {
            iMaxClip = 7;
            iAnim = SKULL2_RELOAD1;
            flDeloadTime = 2.5s;
        }

        if (DefaultReload(iMaxClip, iAnim, flDeloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.5;
            m_iShotsFired = 0;
        }
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

        if (!m_iState)
        {
            if (m_iClip)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0s;

                SendWeaponAnim(m_iClip == 1 ? SKULL2_IDLE2_LEFTEMPTY : SKULL2_IDLE2, UseDecrement() != FALSE);
            }
        }   
        else
            SendWeaponAnim(SKULL2_IDLE1, UseDecrement() != FALSE);
    }

};
LINK_ENTITY_TO_CLASS(weapon_skull2, CSKULL2)
}