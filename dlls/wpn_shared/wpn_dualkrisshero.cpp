/*
dualkrisshero.cpp
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
    class CDualKrissex : public LinkWeaponTemplate< CDualKrissex,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AUG>::template type,
            TPrecacheEvent,
            TSecondaryAttackZoom
    >
    {
    public:

#define WPNSTATE_DUALKRISSHERO_LEFT	(1<<0)

    static constexpr int ZoomFOV = 65;
    static constexpr const char *V_Model = "models/v_dualkrisshero.mdl";
    static constexpr const char *P_Model = "models/p_dualkrisshero.mdl";
    static constexpr const char *W_Model = "models/w_dualkrisshero.mdl";
    static constexpr const char *EventFile = "events/dualkrisshero.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_dualkrisshero";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 70;
    KnockbackData KnockBack = { 350.0f, 300.0f, 350.0f, 200.0f, 0.5f };

	enum
	{
        DUALKRISSHERO_IDLE,
        DUALKRISSHERO_IDLE_LEFTEMPTY,
        DUALKRISSHERO_SHOOT_LEFT1,
        DUALKRISSHERO_SHOOT_LEFT2,
        DUALKRISSHERO_SHOOT_LEFTLAST,
        DUALKRISSHERO_SHOOT_RIGHT1,
        DUALKRISSHERO_SHOOT_RIGHT2,
        DUALKRISSHERO_SHOOT_RIGHTLAST,
        DUALKRISSHERO_RELOAD,
        DUALKRISSHERO_DRAW
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.45f;

    int m_usFireDualKrissexLeft;
    int m_usFireDualKrissexRight;

    void Spawn()
    {
        Base::Spawn();
        m_bDelayFire = false;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762Nato";
        p->iMaxAmmo1 = MAX_AMMO_762NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 70;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_DUALKRISSHERO;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
    float GetArmorRatioModifier() override { return 1.45; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");

        m_usFireDualKrissexLeft = PRECACHE_EVENT(1, "events/dualkriss_left.sc");
        m_usFireDualKrissexRight = PRECACHE_EVENT(1, "events/dualkriss_right.sc");
        
        Base::Precache();
    }

    float GetMaxSpeed(void)
    {
        if (m_pPlayer->m_iFOV == 90)
            return 240.0;

        return 220.0;
    }


    BOOL Deploy() override
    {
        m_flAccuracy = 0.1;
        m_iShotsFired = 0;
        m_bDelayFire = false;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_DUALKRISSHERO_LEFT;

        BOOL bResult = DefaultDeploy("models/v_dualkrisshero.mdl", "models/p_dualkrisshero.mdl", DUALKRISSHERO_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
        m_pPlayer->ResetMaxSpeed();

        return bResult;
    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
        {
            if (m_pPlayer->m_iFOV == 90)
                DualKrissexFire((0.05) * m_flAccuracy, 0.15s, FALSE);
            else
                DualKrissexFire((0.03) * m_flAccuracy, 0.19s, FALSE);
        }
        else if (m_pPlayer->m_iFOV == 90)
            DualKrissexFire((0.0) * m_flAccuracy, 0.15s, FALSE);
        else
            DualKrissexFire((0.0) * m_flAccuracy, 0.19s, FALSE);

    }

    void DualKrissexFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.25;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

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
        if (m_iWeaponState & WPNSTATE_DUALKRISSHERO_LEFT)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_iWeaponState &= ~WPNSTATE_DUALKRISSHERO_LEFT;
            eventindex = m_usFireDualKrissexLeft;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_DUALKRISSHERO_LEFT;
            eventindex = m_usFireDualKrissexRight;
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

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (m_pPlayer->m_iFOV == 90)
        {
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBack(1.0, 0.45, 0.25, 0.075, 4.5, 3.0, 6);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBack(0.5, 0.25, 0.0825, 0.0225, 3.5, 2.25, 7);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBack(0.3, 0.15, 0.05, 0.015, 2.5, 0.8, 8);
            else
                KickBack(0.4, 0.2, 0.0625, 0.0175, 3.0, 1.0, 7);
        }
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.8, 0.45, 0.25, 0.075, 4.5, 3.0, 6);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.4, 0.25, 0.08, 0.0225, 3.5, 2.15, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.2, 0.15, 0.05, 0.015, 2.5, 0.8, 8);
        else
            KickBack(0.3, 0.2, 0.06, 0.0175, 3.0, 1.0, 7);
    }


    float GetDamage(void)
    {
        float flDamage;
        if (m_pPlayer->m_iFOV == 90)
        {
            flDamage = 187.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 187.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 187.0f;
#endif
            return flDamage;
        }
        else
        {
            flDamage = 185.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 185.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 185.0f;
#endif
            return flDamage;
        }
    }

    void Reload(void)
    {
        if (m_pPlayer->ammo_45acp <= 0)
            return;

        if (DefaultReload(MaxClip, DUALKRISSHERO_RELOAD, 3.1s))
        {
            if (m_pPlayer->m_iFOV != 90)
                SecondaryAttack();
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.1;
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

        if (m_iClip)
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60.0s;

            SendWeaponAnim(m_iClip == 1 ? DUALKRISSHERO_IDLE_LEFTEMPTY : DUALKRISSHERO_IDLE, UseDecrement() != FALSE);
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_dualkrisshero, CDualKrissex)
}