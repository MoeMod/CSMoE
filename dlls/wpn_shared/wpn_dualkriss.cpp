/*
dualkriss.cpp
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
    class CDualKriss : public LinkWeaponTemplate< CDualKriss,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
            TPrecacheEvent
    >
    {
    public:

#define WPNSTATE_DUALKRISSHERO_LEFT	(1<<0)

    static constexpr const char *V_Model = "models/v_dualkriss.mdl";
    static constexpr const char *P_Model = "models/p_dualkriss.mdl";
    static constexpr const char *W_Model = "models/w_dualkriss.mdl";
    static constexpr const char *EventFile = "events/dualkrisshero.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_dualkriss";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 50;
    KnockbackData KnockBack = { 400.0f, 400.0f, 500.0f, 180.0f, 0.35f };

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
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.3f;

    int m_usFireDualKrissLeft;
    int m_usFireDualKrissRight;

    void Spawn()
    {
        Base::Spawn();
        m_bDelayFire = false;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "45acp";
        p->iMaxAmmo1 = MAX_AMMO_45ACP;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 50;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_DUALKRISS;
        p->iFlags = 0;
        p->iWeight = MP5NAVY_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_USP); }
#endif
    float GetArmorRatioModifier() override { return 1.3; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    void Precache()
    {
        m_iShellId = PRECACHE_MODEL("models/pshell.mdl");

        m_usFireDualKrissLeft = PRECACHE_EVENT(1, "events/dualkriss_left.sc");
        m_usFireDualKrissRight = PRECACHE_EVENT(1, "events/dualkriss_right.sc");
        
        Base::Precache();
    }

    float GetMaxSpeed(void)
    {
        return 235.0;
    }


    BOOL Deploy() override
    {
        m_flAccuracy = 0.1;
        m_iShotsFired = 0;
        m_bDelayFire = false;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_DUALKRISSHERO_LEFT;

        BOOL bResult = DefaultDeploy("models/v_dualkriss.mdl", "models/p_dualkriss.mdl", DUALKRISSHERO_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);
        m_pPlayer->ResetMaxSpeed();

        return bResult;
    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            DualKrissFire((0.2) * m_flAccuracy, 0.069s, FALSE);
        else
            DualKrissFire((0.04) * m_flAccuracy, 0.069s, FALSE);

    }

    void DualKrissFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired) / 210.0 + 0.25;

        if (m_flAccuracy > 2.0)
            m_flAccuracy = 2.0;

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
            eventindex = m_usFireDualKrissLeft;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_DUALKRISSHERO_LEFT;
            eventindex = m_usFireDualKrissRight;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.83, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iClip);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(0.62, 0.6, 0.5, 0.04, 5.0, 3.0, 10);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.5, 0.17, 0.21, 0.025, 3.5, 1.2, 5);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.4, 0.14, 0.13, 0.02, 1.8, 0.8, 10);
        else
            KickBack(0.5, 0.16, 0.2, 0.02, 3.5, 1.0, 10);
    }


    float GetDamage(void)
    {
        float flDamage;

        flDamage = 30.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 36.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 36.0f;
#endif
        return flDamage;
        
    }

    void Reload(void)
    {
        if (m_pPlayer->ammo_45acp <= 0)
            return;

        if (DefaultReload(MaxClip, DUALKRISSHERO_RELOAD, 3.36s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0;
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
LINK_ENTITY_TO_CLASS(weapon_dualkriss, CDualKriss)
}