/*
vulcanus1.cpp
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
    class CVulcanus1 : public LinkWeaponTemplate< CVulcanus1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_ELITE>::template type,
            TPrecacheEvent
    >
    {
    public:

#define WPNSTATE_VULCANUS1_LEFT	(1<<0)

    static constexpr const char *V_Model = "models/v_vulcanus1.mdl";
    static constexpr const char *P_Model = "models/p_vulcanus1.mdl";
    static constexpr const char *W_Model = "models/w_vulcanus1.mdl";
    static constexpr const char *EventFile = "events/vulcanus1.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_vulcanus1";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 24;
    KnockbackData KnockBack = { 350.0f, 300.0f, 350.0f, 200.0f, 0.5f };

	enum
	{
        ANIM_IDLEB,
        ANIM_RELOADB,
        ANIM_DRAWB,
        ANIM_SHOOTB,
        ANIM_SHOOT_EMPTY,
        ANIM_IDLE,
        ANIM_IDLE_LEFTEMPTY,
        ANIM_SHOOT_LEFT1,
        ANIM_SHOOT_LEFT2,
        ANIM_SHOOT_LEFTLAST,
        ANIM_SHOOT_RIGHT1,
        ANIM_SHOOT_RIGHT2,
        ANIM_SHOOT_RIGHTLAST,
        ANIM_RELOAD,
        ANIM_DRAW,
        ANIM_CHANGEB,
        ANIM_CHANGEA,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.0f;

    int m_usFireVulcanus1Left;
    int m_usFireVulcanus1Right;
    int m_iState;

    void Spawn()
    {
        m_iState = 0;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "45acp";
        p->iMaxAmmo1 = MAX_AMMO_45ACP;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 24;
        p->iSlot = 1;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_VULCANUS1;
        p->iFlags = 0;
        p->iWeight = ELITE_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_USP); }
#endif
    float GetArmorRatioModifier() override { return 1.0; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");

        m_usFireVulcanus1Left = PRECACHE_EVENT(1, "events/vulcanus1_left.sc");
        m_usFireVulcanus1Right = PRECACHE_EVENT(1, "events/vulcanus1_right.sc");
        
        PRECACHE_SOUND("weapons/vulcanus1-2.wav");

        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = m_iState ? 0.9 : 1.0;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_VULCANUS1_LEFT;

        BOOL bResult = DefaultDeploy("models/v_vulcanus1.mdl", "models/p_vulcanus1.mdl", m_iState ? ANIM_DRAWB : ANIM_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);

        return bResult;
    }

    void SecondaryAttack(void)
    {
        if (m_iState)
        {
            m_iState = 0;
            SendWeaponAnim(ANIM_CHANGEB, UseDecrement() != FALSE);
        }
        else
        {
            m_iState = 1;
            SendWeaponAnim(ANIM_CHANGEA, UseDecrement() != FALSE);
        }
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2s;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 2s;
    }

    void PrimaryAttack(void)
    {
        if (m_iState)
            Vulcanus1Fire((0.09) * (1 - m_flAccuracy), 1.0s, FALSE);
        else
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                Vulcanus1Fire((0.14) * (1 - m_flAccuracy), 0.25s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                Vulcanus1Fire((0.055) * (1 - m_flAccuracy), 0.25s, FALSE);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                Vulcanus1Fire((0.03) * (1 - m_flAccuracy), 0.25s, FALSE);
            else
                Vulcanus1Fire((0.035) * (1 - m_flAccuracy), 0.25s, FALSE);
        }

    }

    void Vulcanus1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iShotsFired++;

        if (m_iShotsFired > 1)
            return;

        if (m_flLastFire != invalid_time_point)
        {
            m_flAccuracy -= (0.52 - ((gpGlobals->time - m_flLastFire) / 1s)) * 0.3;

            float flAccuracyMax = m_iState ? 0.9 : 1.0;
            if (m_flAccuracy > flAccuracyMax)
                m_flAccuracy = flAccuracyMax;
            else if (m_flAccuracy < 0.6)
                m_flAccuracy = 0.6;
        }

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

        Vector vecDir;
        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        if (m_iState)
        {
            for (int i = 0; i < 4; i++)
            {
                UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

                m_iClip--;

                if (m_iWeaponState & WPNSTATE_VULCANUS1_LEFT)
                {
#ifndef CLIENT_DLL
                    m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
                    m_iWeaponState &= ~WPNSTATE_VULCANUS1_LEFT;
                   
                    vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition() - gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 4096, BULLET_PLAYER_45ACP, 1, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
                    PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_VULCANUS1_LEFT, m_iClip);
                }
                else
                {
#ifndef CLIENT_DLL
                    m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
                    m_iWeaponState |= WPNSTATE_VULCANUS1_LEFT;

                    vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition() + gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 4096, BULLET_PLAYER_45ACP, 1, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
                    PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState & WPNSTATE_VULCANUS1_LEFT, m_iClip);
                }

                if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                    flSpread += 0.03;
                else if (m_pPlayer->pev->velocity.Length2D() > 0)
                    flSpread += 0.02;
                else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                    flSpread += 0.01;
                else
                    flSpread += 0.015;

                if (m_iClip <= 0)
                    break;
            }

            SendWeaponAnim(m_iClip > 0 ? ANIM_SHOOTB : ANIM_SHOOT_EMPTY, UseDecrement() != FALSE);

            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/vulcanus1-2.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        }
        else
        {
            UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

            m_iClip--;

            if (m_iWeaponState & WPNSTATE_VULCANUS1_LEFT)
            {
#ifndef CLIENT_DLL
                m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
                m_iWeaponState &= ~WPNSTATE_VULCANUS1_LEFT;

                vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition() - gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 4096, BULLET_PLAYER_45ACP, 1, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
                PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireVulcanus1Left, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iClip);

            }
            else
            {
#ifndef CLIENT_DLL
                m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
                m_iWeaponState |= WPNSTATE_VULCANUS1_LEFT;

                vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition() + gpGlobals->v_right * 5, gpGlobals->v_forward, flSpread, 4096, BULLET_PLAYER_45ACP, 1, GetDamage(), 0.8, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);
                PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFireVulcanus1Right, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iClip);
            }
        }

        
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 1.0, 0.8, 0.8, 5.0, 5.0, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.4, 0.5, 0.1, 0.15, 2.5, 3.5, 4);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.3, 0.45, 0.12, 0.12, 2.0, 3.0, 2);
        else
            KickBack(0.4, 0.55, 0.2, 0.3, 2.0, 2.5, 3);
    }


    float GetDamage(void)
    {
        float flDamage;
        if (m_iState)
        {
            flDamage = 30.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 69.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 85.0f;
#endif
            return flDamage;
        }
        else
        {
            flDamage = 37.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 63.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 75.0f;
#endif
            return flDamage;
        }
    }

    void Reload(void)
    {
        if (m_pPlayer->ammo_45acp <= 0)
            return;

        if (DefaultReload(MaxClip, m_iState ? ANIM_RELOADB : ANIM_RELOAD, 3.7s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = m_iState ? 0.9 : 1.0;
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

        if (m_iState)
            SendWeaponAnim(ANIM_IDLEB, UseDecrement() != FALSE);
        else
        {
            if (m_iClip)
            {
                SendWeaponAnim(m_iClip == 1 ? ANIM_IDLE_LEFTEMPTY : ANIM_IDLE, UseDecrement() != FALSE);
            }
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_vulcanus1, CVulcanus1)
}