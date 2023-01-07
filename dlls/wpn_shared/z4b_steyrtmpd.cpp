/*
steyrtmpd.cpp
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
    class CSteyrTMPD : public LinkWeaponTemplate< CSteyrTMPD,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/z4b/v_steyrtmpd.mdl";
    static constexpr const char *P_Model = "models/z4b/p_steyrtmpd.mdl";
    static constexpr const char *W_Model = "models/z4b/w_steyrtmpd.mdl";
    static constexpr const char *EventFile = "events/steyrtmpd.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_steyrtmpd";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 80;

    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 60;
    static constexpr int DamageZBS = 135;

    KnockbackData KnockBack = { 650.0f, 500.0f, 550.0f, 400.0f, 0.9f };

	enum
	{
        STEYRTMPD_IDLE,
        STEYRTMPD_RELOAD,
        STEYRTMPD_DRAW,
        STEYRTMPD_SHOOT_PRE,
        STEYRTMPD_SHOOT,
        STEYRTMPD_SHOOT_POST,
        STEYRTMPD_RUN,
        STEYRTMPD_ON_SHOULDER,
        STEYRTMPD_DRAW2,
        STEYRTMPD_ON_SHOULDER_IDLE,
        STEYRTMPD_ON_SHOULDER_RELOAD,
        STEYRTMPD_ON_SHOULDER_DRAW,
        STEYRTMPD_ON_SHOULDER_SHOOT_PRE,
        STEYRTMPD_ON_SHOULDER_SHOOT_LEFTLAST,
        STEYRTMPD_ON_SHOULDER_SHOOT_POST,
        STEYRTMPD_ON_SHOULDER_RUN,
        STEYRTMPD_OFF_SHOULDER,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.45f;

    time_point_t m_tNextPlayEvent;

    void Spawn()
    {
        Base::Spawn();
        m_tNextPlayEvent = invalid_time_point;
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
        p->iId = m_iId = WEAPON_Z4B_STEYRTMPD;
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
        PRECACHE_MODEL("sprites/muzzleflash25.spr");

        PRECACHE_MODEL("sprites/ef_dualsword_left.spr");
        PRECACHE_MODEL("sprites/ef_dualsword_right.spr");

        m_usFire = PRECACHE_EVENT(1, "events/steyrtmpd.sc");
        
        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.1;
        m_iShotsFired = 0;
        m_bDelayFire = false;

        int iAnim;

        if (m_iWeaponState)
            iAnim = STEYRTMPD_ON_SHOULDER_DRAW;
        else
            iAnim = RANDOM_LONG(0, 1) ? STEYRTMPD_DRAW : STEYRTMPD_DRAW2;

        BOOL bResult = DefaultDeploy("models/z4b/v_steyrtmpd.mdl", "models/z4b/p_steyrtmpd.mdl", iAnim, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);

        m_tNextPlayEvent = gpGlobals->time + 0.75s;
        pev->body = m_pPlayer->m_bIsFemale ? 1 : 2;

        return bResult;
    }

    float GetMaxSpeed(void)
    {
        if (m_iWeaponState == 1)
            return 220.0;

        return 240.0;
    }

    void SecondaryAttack()
    {   
        if (m_flNextSecondaryAttack > UTIL_WeaponTimeBase())
            return;

        if (!m_iWeaponState)
        {      
            m_iWeaponState = 1;
            SendWeaponAnim(STEYRTMPD_ON_SHOULDER, UseDecrement() != FALSE);

            m_pPlayer->m_flNextAttack = 2.0s;
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
        }
        else
        {
            m_iWeaponState = 0;
            SendWeaponAnim(STEYRTMPD_OFF_SHOULDER, UseDecrement() != FALSE);

            m_pPlayer->m_flNextAttack = 2.0s;
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.5s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;
        }    
    }

    void PrimaryAttack(void)
    {
        if (m_iWeaponState)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                DualTMPFire((0.1) * m_flAccuracy, 0.04s, FALSE);
            else
                DualTMPFire((0.015) * m_flAccuracy, 0.04s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                DualTMPFire((0.2) * m_flAccuracy, 0.06s, FALSE);
            else
                DualTMPFire((0.025) * m_flAccuracy, 0.06s, FALSE);
        }
       

    }

    void DualTMPFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (m_iWeaponState)
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

#ifndef CLIENT_DLL
        if(RANDOM_LONG(0,1))
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
        else
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_9MM, GetDamage(), 0.88, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        m_iClip--;
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip == 0, m_iWeaponState == 1);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        if (m_iWeaponState)
        {
            flSpread += 0.01;
            vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_9MM, GetDamage(), 0.88, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
            PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip == 0, m_iWeaponState == 1);
        }

       
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (m_iWeaponState)
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
        if (m_iWeaponState)
        {
            flDamage = 33.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 265.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 265.0f;
#endif
            return flDamage;
        }
        else
        {
            flDamage = 30.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 287.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 287.0f;
#endif
            return flDamage;
        }
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim;
        if (!m_iWeaponState)
        {
            iAnim = STEYRTMPD_RELOAD;
        }
        else
        {
            iAnim = STEYRTMPD_ON_SHOULDER_RELOAD;
        }

        if (DefaultReload(80, iAnim, 2.6s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.1;
            m_iShotsFired = 0;
        }
    }

    void ItemPostFrame()
    {
        if (gpGlobals->time > m_tNextPlayEvent && m_tNextPlayEvent != invalid_time_point)
        {
            PLAYBACK_EVENT_FULL(FEV_HOSTONLY, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, 2.0, 0.0, m_iWeaponState, 0, FALSE, FALSE);
            m_tNextPlayEvent = gpGlobals->time + 0.15s;
        }

        CBasePlayerWeapon::ItemPostFrame();
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

       

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
        SendWeaponAnim(m_iWeaponState ? STEYRTMPD_ON_SHOULDER_IDLE : STEYRTMPD_IDLE, UseDecrement() != FALSE);
    }

};
LINK_ENTITY_TO_CLASS(z4b_steyrtmpd, CSteyrTMPD)
}