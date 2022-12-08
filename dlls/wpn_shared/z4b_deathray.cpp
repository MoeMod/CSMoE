/*
deathray.cpp
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
    class CDeathRay : public LinkWeaponTemplate< CDeathRay,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AWP>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/z4b/v_deathray.mdl";
    static constexpr const char *P_Model = "models/p_deathray.mdl";
    static constexpr const char *W_Model = "models/w_deathray.mdl";
    static constexpr const char *EventFile = "events/deathray.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_deathray";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = -1;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_DRAW,
        ANIM_DRAW1,
       
	};
    static constexpr float PlayerMaxSpeed = 230;

    static constexpr int DamageDefault = 130;
    static constexpr int DamageZB = 760;
    static constexpr int DamageZBS = 1100;

    KnockbackData KnockBack = { 5000, 900, 1000, 800, 0.2f };

    time_point_t m_flNextReset;
    int m_iShotsRequired;
    int m_bHit;

#define WPNSTATE_DEATHRAY_SIGNAL 1
#define WPNSTATE_DEATHRAY_USING 2

    void Spawn() override
    {  
        m_flNextReset = invalid_time_point;
        m_iShotsRequired = GetShotsRequired();
        Base::Spawn();
        m_iDefaultAmmo = 35;
    }

    float GetMaxSpeed(void)
    {
        if (m_pPlayer->m_iFOV == 90)
            return 195;

        return 140;
    }

    int GetShotsRequired()
    {
        int iShotsRequired = 3;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            iShotsRequired = 3;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            iShotsRequired = 3;
#endif
        return iShotsRequired;
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash19.spr");

        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "RailBeam";
        p->iMaxAmmo1 = 35;
        p->pszAmmo2 = "InfiniteAmmo";
        p->iMaxAmmo2 = 255;
        p->iMaxClip = -1;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_Z4B_DEATHRAY;
        p->iFlags = 0;
        p->iWeight = AWP_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        if (m_iDefaultAmmo)
        {
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
            m_iClip = WEAPON_NOCLIP;
            m_iDefaultAmmo = 0;
            return TRUE;
        }

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_railbeam" , 180 }; }
#endif
    float GetArmorRatioModifier() override 
    {
        return 1.9;
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

        return DefaultDeploy(V_Model, P_Model, ANIM_DRAW, "carbine", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        duration_t flCycleTime = 2.6s;
        if (m_iWeaponState == WPNSTATE_DEATHRAY_USING)
            flCycleTime = 0.15s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.5, flCycleTime, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 120)
            Fire(0.07, flCycleTime, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 10)
            Fire(0.03, flCycleTime, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            Fire(0.0, flCycleTime, FALSE);
        else
            Fire(0.0, flCycleTime, FALSE);
    }


#ifndef CLIENT_DLL
    void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace)
    {
        if (!m_bHit)
        {
            if (CBaseEntity* pEntity = gMultiDamage.pEntity)
            {
                if (pEntity->IsPlayer() || pEntity->Classify() == CLASS_PLAYER_ALLY)
                {
                    if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) != GR_TEAMMATE)
                    {
                        pev->iuser1++;

                        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
                        WRITE_BYTE(1);
                        WRITE_BYTE(pev->iuser1);
                        MESSAGE_END();

                        m_bHit = true;
                    }
                }
            }
        }

    }
#endif

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_pPlayer->pev->fov != 90)
        {    
            if (m_iWeaponState != WPNSTATE_DEATHRAY_USING)
            {
                m_pPlayer->m_bResumeZoom = true;
                m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
                m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
                m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
            }
        }
        else // not zoomed, so bullets diverts more
            flSpread += 0.03;


        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

            return;
        }

        if (m_iWeaponState != WPNSTATE_DEATHRAY_USING)
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

        m_bHit = m_iWeaponState == WPNSTATE_DEATHRAY_USING ? true : false;

        float flDamage = m_iWeaponState == WPNSTATE_DEATHRAY_USING ? GetDamage() : GetDamageB();

        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 5, BULLET_PLAYER_RAILBEAM, flDamage, 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
      
#ifndef CLIENT_DLL 
        if (!m_iWeaponState)
        {
            if (pev->iuser1 >= m_iShotsRequired)
            {
                m_iWeaponState = WPNSTATE_DEATHRAY_SIGNAL;
                m_flNextReset = gpGlobals->time + 9.0s;
                EMIT_SOUND_DYN(edict(), CHAN_ITEM, "weapons/stunrifle_lowbattery.wav", 1.0, ATTN_NORM, 0, PITCH_NORM);
                pev->iuser1 = 0;
                
                m_pPlayer->ResetMaxSpeed();
                m_pPlayer->m_bResumeZoom = false;
                m_pPlayer->m_iLastZoom = m_pPlayer->m_iFOV;
            }
        }

        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, FALSE);
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.6s;

        m_pPlayer->pev->punchangle.x -= 2;
    }

    void ItemPostFrame(void) override
    {
        if (m_flNextReset != invalid_time_point && m_flNextReset < gpGlobals->time)
        {
            if (m_iWeaponState == WPNSTATE_DEATHRAY_SIGNAL)
            {
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0s;
                m_flNextReset = invalid_time_point;
            }
            if (m_iWeaponState == WPNSTATE_DEATHRAY_USING)
            {
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
                SendWeaponAnim(ANIM_DRAW, UseDecrement() != FALSE);
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
                m_flNextReset = invalid_time_point;

                if (m_pPlayer->pev->fov != 90)
                {
                    m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90;
                    m_pPlayer->ResetMaxSpeed();
                }
            }
            pev->iuser1 = 0;
            m_iWeaponState = 0;
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState == WPNSTATE_DEATHRAY_SIGNAL)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 255;
            m_iWeaponState = WPNSTATE_DEATHRAY_USING;
            SendWeaponAnim(ANIM_DRAW1, UseDecrement() != FALSE);
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.7s;
            m_flNextReset = gpGlobals->time + 8.0s;

        }
        else
        {
            switch (m_pPlayer->m_iFOV)
            {
            case 90: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 40; break;
            case 40: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 10; break;
            default: m_pPlayer->m_iFOV = m_pPlayer->pev->fov = 90; break;
            }

            m_pPlayer->ResetMaxSpeed();
            EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/zoom.wav", 0.2, 2.4);
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
        }
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
    }

    float GetDamageB()
    {
        float flDamage = 29;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 690;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 900;
#endif
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(z4b_deathray, CDeathRay)
}