/*
janus3.cpp
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
    class CJANUS3 : public LinkWeaponTemplate< CJANUS3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MAC10>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_janus3.mdl";
    static constexpr const char *P_Model = "models/p_janus3.mdl";
    static constexpr const char *W_Model = "models/w_janus3.mdl";
    static constexpr const char *EventFile = "events/janus3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_janus3";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = 50;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_RELOAD1,
        ANIM_DRAW1,
        ANIM_SHOOT1,
        ANIM_SHOOT1_SIGNAL,
        ANIM_CHANGE1,
        ANIM_IDLE2,
        ANIM_DRAW2,
        ANIM_SHOOT2_1,
        ANIM_SHOOT2_2,
        ANIM_SHOOT2_3,
        ANIM_CHANGE2,
        ANIM_SIGNAL,
        ANIM_RELOAD_SIGNAL,
        ANIM_DRAW_SIGNAL,
	};
    static constexpr float PlayerMaxSpeed = 230;

    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 35;
    static constexpr int DamageZBS = 52;

    KnockbackData KnockBack = { 375.0f, 270.0f, 300.0f, 120.0f, 0.6f };

    time_point_t m_flNextReset;
    int m_iShotsRequired;

#define WPNSTATE_JANUS3_SIGNAL 1
#define WPNSTATE_JANUS3_USING 2

    void Spawn() override
    {
        m_flNextReset = invalid_time_point;
        m_iShotsRequired = GetShotsRequired();
        Base::Spawn();
    }

    int GetShotsRequired()
    {
        int iShotsRequired = 25;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iShotsRequired = 20;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iShotsRequired = 20;
#endif
        return iShotsRequired;
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash32.spr");
        PRECACHE_SOUND("weapons/janus3-1.wav");
        PRECACHE_SOUND("weapons/change1_ready.wav");
        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "45acp";
        p->iMaxAmmo1 = MAX_AMMO_45ACP;
        p->pszAmmo2 = "InfiniteAmmo";
        p->iMaxAmmo2 = 255;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_JANUS3;
        p->iFlags = 0;
        p->iWeight = MAC10_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_MAC10); }
#endif
    float GetArmorRatioModifier() override 
    {
        if (m_iWeaponState == WPNSTATE_JANUS3_USING)
            return 1.2;
        return 1;
    }
    WeaponCrosshairData GetCrosshairData() override 
    {
        if (m_iWeaponState)
            return { CROSSHAIR_COLOR_CUSTOM | ACCURACY_AIR, 6, 3, 0 };
        else
            return { ACCURACY_AIR, 6, 3, 0 };
    }

    BOOL Deploy() override
    {
        m_flAccuracy = 0.05;
        m_iShotsFired = 0;

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUS3_USING)
            iAnim = ANIM_DRAW2;
        else if (m_iWeaponState == WPNSTATE_JANUS3_SIGNAL)
            iAnim = ANIM_DRAW_SIGNAL;
        else
            iAnim = ANIM_DRAW1;

        return DefaultDeploy("models/v_janus3.mdl", "models/p_janus3.mdl", iAnim, "carbine", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        duration_t flCycleTime = 0.09875s;
        if (m_iWeaponState == WPNSTATE_JANUS3_USING)
            flCycleTime = 0.072s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.07 + (0.08) * m_flAccuracy, flCycleTime, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(0.025 + (0.045) * m_flAccuracy, flCycleTime, FALSE);
        else
            Fire(0.025 + (0.03) * m_flAccuracy, flCycleTime, FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 210) + 0.05;

        if (m_flAccuracy > 0.8)
            m_flAccuracy = 0.8;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif
            return;
        }


        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir, vecUp, vecRight, vecForward;
        vecForward = gpGlobals->v_forward;
        vecUp = gpGlobals->v_up;
        vecRight = gpGlobals->v_right;

        if(m_iWeaponState != WPNSTATE_JANUS3_USING)
            m_iClip--;
        
        vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.85, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
   
#ifndef CLIENT_DLL 
        int HitResult = UTIL_GetLineTarget(vecSrc, vecForward, vecRight, vecUp, vecDir, 8192, m_pPlayer, true);
        if (HitResult == HIT_PLAYER)
        {
            if (!m_iWeaponState)
            {
                pev->iuser1++;
                if (pev->iuser1 >= m_iShotsRequired)
                {
                    m_iWeaponState = WPNSTATE_JANUS3_SIGNAL;
                    m_flNextReset = gpGlobals->time + 9.0s;

                    EMIT_SOUND_DYN(edict(), CHAN_ITEM, "weapons/change1_ready.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
                }
            }
        }

        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
#endif
 
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

        if (m_iWeaponState == WPNSTATE_JANUS3_USING)
        {
            if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.12, 0.635, 0.73, 0.04, 3.25, 4.0, 8);
            else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(0.45, 0.25, 0.115, 0.145, 2.7, 1.7, 10);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.05, 0.19, 0.12, 0.025, 2.25, 1.5, 9);
            else
                KickBack(0.175, 0.18, 0.16, 0.0125, 2.1, 1.6, 10);
        }
        else
        {
            if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.4, 0.2, 0.125, 0.13, 2.5, 1.5, 10);
            else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(0.115, 0.625, 0.75, 0.0435, 3.5, 3.8, 11);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.1, 0.175, 0.125, 0.02, 2.25, 1.5, 9);
            else
                KickBack(0.175, 0.18, 0.16, 0.0125, 2.1, 1.6, 10);
        }
    }

    void ItemPostFrame(void) override
    {
        if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
        {
            if (m_iWeaponState == WPNSTATE_JANUS3_SIGNAL)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
                m_flNextReset = invalid_time_point;
            }
            if (m_iWeaponState == WPNSTATE_JANUS3_USING)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
                SendWeaponAnim(ANIM_CHANGE2, UseDecrement() != FALSE);
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
                m_flNextReset = invalid_time_point;
            }
            pev->iuser1 = 0;
            m_iWeaponState = 0;
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState == WPNSTATE_JANUS3_SIGNAL)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
            m_iWeaponState = WPNSTATE_JANUS3_USING;
            SendWeaponAnim(ANIM_CHANGE1, UseDecrement() != FALSE);
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;

            m_flNextReset = gpGlobals->time + 8.0s;
        }
    }

    void Reload() override
    {
        if (m_pPlayer->ammo_45acp <= 0 || m_iWeaponState == WPNSTATE_JANUS3_USING)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState == WPNSTATE_JANUS3_SIGNAL ? ANIM_RELOAD_SIGNAL : ANIM_RELOAD1, 3.0s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.05;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        int iAnim;
        if (m_iWeaponState == WPNSTATE_JANUS3_USING)
            iAnim = ANIM_IDLE2;
        else if (m_iWeaponState == WPNSTATE_JANUS3_SIGNAL)
            iAnim = ANIM_SIGNAL;
        else
            iAnim = ANIM_IDLE1;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }

    int GetDamageB()
    {
        int iDamage = 29;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iDamage = 53;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iDamage = 68;
#endif
        return iDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_janus3, CJANUS3)
}