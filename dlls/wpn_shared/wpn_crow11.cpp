/*
crow11.cpp
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
    class CCrow11 : public LinkWeaponTemplate< CCrow11,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_XM1014>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_crow11.mdl";
    static constexpr const char *P_Model = "models/p_crow11.mdl";
    static constexpr const char *W_Model = "models/w_crow11.mdl";
    static constexpr const char *EventFile = "events/crow11.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_crow11";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 20;
	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_RELOAD_START,
        ANIM_RELOAD_FAST,
        ANIM_RELOAD_SLOW,
        ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 250;

    static constexpr int DamageDefault = 26;
    static constexpr int DamageZB = 68;
    static constexpr int DamageZBS = 110;

    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };
    
    bool bCheckingReload;
    bool bCheckingCharge;
    bool bCanCharge;
    time_point_t m_flNextCheckReload;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "12gauge";
        p->iMaxAmmo1 = 80;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 20;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CROW11;
        p->iFlags = 0;
        p->iWeight = XM1014_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_12gauge" , 65 }; }
#endif
    float GetArmorRatioModifier() override { return 1.0; }
    WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash1.spr");
        PRECACHE_MODEL("sprites/muzzleflash2.spr");
        Base::Precache();
    }

    BOOL Deploy() override
    {
        bCheckingReload = false;
        bCheckingCharge = false;
        bCanCharge = false;
        m_flNextCheckReload = invalid_time_point;
        return Base::Deploy();
    }


    void PrimaryAttack(void) override
    {
        if (m_pPlayer->pev->waterlevel == 3)
        {
            PlayEmptySound();
            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.15s;
            return;
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

        m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
#ifndef CLIENT_DLL
        m_pPlayer->FireBullets(6, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(0.069, 0.069, 0.0), 3048, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif
        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, m_vVecAiming.x, m_vVecAiming.y, 7, m_vVecAiming.x * 100, m_iClip != 0, FALSE);

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.3s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
      
        if (m_pPlayer->pev->flags & FL_ONGROUND)
            m_pPlayer->pev->punchangle.x -= 3;
        else
            m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 7, 11);
    }

    void ItemPostFrame() override
    {
        if (m_pPlayer->pev->button & IN_RELOAD)
        {
            if (bCheckingReload)
            {
                if(bCheckingCharge)
                    bCanCharge = true;
                else
                    bCanCharge = false;
            }
        }

        if (m_flNextCheckReload != invalid_time_point && m_flNextCheckReload < gpGlobals->time)
        {
            if (bCheckingReload)
            {
                if (!bCheckingCharge && bCanCharge)
                {
                    bCheckingCharge = true;
                    bCanCharge = false;
                    m_flNextCheckReload = gpGlobals->time + 0.2s;
                    m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
                }
                else
                {
                    Reload();
                    bCheckingCharge = false;
                    bCheckingReload = false;
                }
            }
        }
        
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iClipSize = MaxClip;
        int j = Q_min(iClipSize - m_iClip, m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]);

        if (!j)
            return;

        if (!bCheckingReload)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            SendWeaponAnim(ANIM_RELOAD_START, UseDecrement() != FALSE);
            m_flNextCheckReload = gpGlobals->time + 0.8s;
            m_pPlayer->m_flNextAttack = 0.5s;   //oldbutton is useless now need to avoid double time IN_RELOAD
            m_flNextSecondaryAttack = m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
            bCheckingReload = true;
            bCanCharge = true;
            return;
        }

        if (bCanCharge)
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            DefaultReload(MaxClip, ANIM_RELOAD_FAST, 1.33s);
        }
        else
        {
            bCheckingReload = false;
            m_flNextCheckReload = invalid_time_point;
            DefaultReload(MaxClip, ANIM_RELOAD_SLOW, 2.33s);
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_crow11, CCrow11)
}