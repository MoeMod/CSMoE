/*
kronos5.cpp
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
#include "shake.h"
#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CKronos5 : public LinkWeaponTemplate< CKronos5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TGetDamageDefault
    >
    {
    public:
    static constexpr const auto& DefaultReloadTime = 2.6s;
    static constexpr const char *V_Model = "models/v_kronos5.mdl";
    static constexpr const char *P_Model = "models/p_kronos5.mdl";
    static constexpr const char *W_Model = "models/w_kronos5.mdl";
    static constexpr const char *EventFile = "events/kronos5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_kronos5";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 30;
	enum
	{
        ANIM_IDLE,
        ANIM_IDLE2,
        ANIM_IDLE3,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_RELOAD,
        ANIM_RELOAD2,
        ANIM_DRAW,
        ANIM_DRAW2,
        ANIM_SCAN_ACTIVATE,
        ANIM_ZOOM,
        ANIM_SCAN_DEACTIVATE,
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 150;
    static constexpr int DamageZBS = 204;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;

#define WPNSTATE_KRONOS5_SCAN_ACTIVATE	1
#define WPNSTATE_KRONOS5_ZOOM_ACTIVATE	2
#define KRONOS5_CHARGE_AMMO 30

    KnockbackData KnockBack = { 550.0f, 350.0f, 450.0f, 350.0f, 0.65f };

    bool m_bHit;
    time_point_t m_flNextThink;

    void Spawn()
    {
        m_flNextThink = invalid_time_point;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556k";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_KRONOS5;
        p->iFlags = 0;
        p->iWeight = AUG_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556k" , 120 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 5, 3, 140 }; }

    float GetMaxSpeed(void)
    {
        if (m_pPlayer->m_iFOV == 90)
            return 235;

        return 210;
    }

    float GetDamageB()
    {
        float flDamage = 110;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 967;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1849;
#endif
        return flDamage;
    }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash3.spr");
        //PRECACHE_SOUND("weapons/kronos5_takeaim.wav");
        Base::Precache();
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

    BOOL Deploy(void)
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        m_flNextThink = invalid_time_point;
        return DefaultDeploy("models/v_kronos5.mdl", "models/p_kronos5.mdl", m_iWeaponState & WPNSTATE_KRONOS5_SCAN_ACTIVATE ? ANIM_DRAW2 : ANIM_DRAW, "mp5", UseDecrement() != FALSE);
    }

    void PrimaryAttack() override
    {
        if (m_pPlayer->pev->fov != 90)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Kronos5Fire(0.35 * m_flAccuracy + 0.035, 0.8s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Kronos5Fire(0.035 * m_flAccuracy + 0.025, 0.8s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                Kronos5Fire(0.0175 * m_flAccuracy, 0.8s, FALSE);
            else
                Kronos5Fire(0.015 * m_flAccuracy, 0.8s, FALSE);
        }
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Kronos5Fire(0.35 * m_flAccuracy + 0.035, 0.08s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 140)
                Kronos5Fire(0.035 * m_flAccuracy + 0.025, 0.08s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                Kronos5Fire(0.0175 * m_flAccuracy, 0.08s, FALSE);
            else
                Kronos5Fire(0.02 * m_flAccuracy, 0.08s, FALSE);
        }
    }


    void Kronos5Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0) + 0.35;

        if (m_flAccuracy > 1)
            m_flAccuracy = 1;

        m_flLastFire = gpGlobals->time;

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

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        m_bHit = m_pPlayer->m_iFOV != 90 ? true : false;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecUp, vecRight, vecForward;
        vecForward = gpGlobals->v_forward;
        vecUp = gpGlobals->v_up;
        vecRight = gpGlobals->v_right;
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 2, BULLET_PLAYER_556MM, m_pPlayer->m_iFOV == 90 ? GetDamage() : GetDamageB(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

#ifndef CLIENT_DLL
        PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_KRONOS5_ZOOM_ACTIVATE, m_iWeaponState & WPNSTATE_KRONOS5_SCAN_ACTIVATE);
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

        if (!m_iWeaponState)
        {
            if (pev->iuser1 >= KRONOS5_CHARGE_AMMO)
            {
                SendWeaponAnim(ANIM_SCAN_ACTIVATE, UseDecrement() != FALSE);
                m_iWeaponState |= WPNSTATE_KRONOS5_SCAN_ACTIVATE;
                pev->iuser1 = 0;
            }
        }

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (m_pPlayer->m_iFOV == 90)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(1.0, 0.45, 0.25, 0.075, 5.0, 3.25, 6);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.7, 0.4, 0.21, 0.03, 4.0, 2.25, 7);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.5, 0.25, 0.125, 0.01, 3.0, 2.0, 8);
            else
                KickBack(0.55, 0.3, 0.2, 0.01, 3.25, 2.0, 7);
        }
        else
        {
            m_pPlayer->pev->punchangle.x -= (UTIL_SharedRandomFloat(m_pPlayer->random_seed + 4, 1.5, 3.0) + m_pPlayer->pev->punchangle.x * 0.25);
            m_pPlayer->pev->punchangle.y += UTIL_SharedRandomFloat(m_pPlayer->random_seed + 5, -0.8, 0.8);
        }
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_KRONOS5_ZOOM_ACTIVATE)
        {
            SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
            m_iWeaponState = 0;
            m_flNextThink = invalid_time_point;
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            pev->iuser1 = 0;
            return;
        }

        if (m_iWeaponState & WPNSTATE_KRONOS5_SCAN_ACTIVATE)
        {
            SendWeaponAnim(ANIM_ZOOM, UseDecrement() != FALSE);
            m_flNextThink = gpGlobals->time + 0.87s;
            m_pPlayer->m_flNextAttack = 0.87s;
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.875s;
            m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.875s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.87s;
        }

    }

    void ItemPostFrame(void)
    {
        if (m_flNextThink != invalid_time_point && m_flNextThink < gpGlobals->time)
        {
            if (m_pPlayer->m_iFOV != 90)
            {
                SendWeaponAnim(ANIM_SCAN_DEACTIVATE, UseDecrement() != FALSE);
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.875s;
                m_iWeaponState = 0;
                m_flNextThink = invalid_time_point;
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
                pev->iuser1 = 0;
            }
            else
            {
#ifndef CLIENT_DLL
                MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->edict());
                WRITE_BYTE(20);
                WRITE_SHORT(5);
                MESSAGE_END();

                if (!m_pPlayer->m_bNightVisionOn)
                    UTIL_ScreenFade(m_pPlayer, Vector(80, 80, 0), 5s, 5s, 80, FFADE_MODULATE);
#endif
                m_iWeaponState |= WPNSTATE_KRONOS5_ZOOM_ACTIVATE;
                m_flNextThink = gpGlobals->time + 5.0s;
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 65;


            }
        }

        return CBasePlayerWeapon::ItemPostFrame();
    }


    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iClipSize = 30;
        int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

        if (!j)
            return;

        if (m_pPlayer->m_iFOV != 90)
        {
            m_iWeaponState = 0;
            m_flNextThink = invalid_time_point;
            m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;
            pev->iuser1 = 0;
        }

        if (DefaultReload(iClipSize, m_iWeaponState & WPNSTATE_KRONOS5_SCAN_ACTIVATE ? ANIM_RELOAD2 : ANIM_RELOAD, 2.6s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }


    void Holster(int skiplocal)
    {
        if (m_iWeaponState & WPNSTATE_KRONOS5_ZOOM_ACTIVATE)
            m_iWeaponState = 0;
        CBasePlayerWeapon::Holster(skiplocal);
    }


    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;

            if (m_iWeaponState & WPNSTATE_KRONOS5_SCAN_ACTIVATE)
            {
                if (m_iWeaponState & WPNSTATE_KRONOS5_ZOOM_ACTIVATE)
                    SendWeaponAnim(ANIM_IDLE3, UseDecrement() != FALSE);
                else
                    SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
            }
            else
            {
                SendWeaponAnim(ANIM_IDLE, UseDecrement() != FALSE);
            }

        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_kronos5, CKronos5)
}