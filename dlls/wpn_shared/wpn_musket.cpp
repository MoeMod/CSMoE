/*
musket.cpp
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
    class CMusket : public LinkWeaponTemplate< CMusket,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
            TPrecacheEvent,
            TGetDamageDefault

    >
    {
    public:

    static constexpr const char *V_Model = "models/v_musket.mdl";
    static constexpr const char *P_Model = "models/p_musket.mdl";
    static constexpr const char *W_Model = "models/w_musket.mdl";
    static constexpr const char *EventFile = "events/musket.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_musket";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = -1;

    static constexpr int DamageDefault = 110;
    static constexpr int DamageZB = 290;
    static constexpr int DamageZBS = 500;

	enum
	{
        ANIM_IDLE1,
        ANIM_IDLE2,
        ANIM_SHOOTSTART,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.7f;
    KnockbackData KnockBack = { 2500.0f, 600.0f, 900.0f, 600.0f, 0.3f };

    time_point_t m_flTimeWeaponIdle2;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "MusketAmmo";
        p->iMaxAmmo1 = -1;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 1;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_MUSKET;
        p->iFlags = 0;
        p->iWeight = DEAGLE_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_musket" , 1000 }; }
#endif
    float GetArmorRatioModifier() override { return 1.7; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

    void Precache(void)
    {
        PRECACHE_MODEL("sprites/muzzleflash14.spr");
        Base::Precache();
    }

    void Spawn() override
    {
        m_flTimeWeaponIdle2 = invalid_time_point;
        Base::Spawn();
    }

    BOOL Deploy() override
    {
        m_flTimeWeaponIdle2 = invalid_time_point;

        return DefaultDeploy("models/v_musket.mdl", "models/p_musket.mdl", ANIM_DRAW, AnimExtension, UseDecrement() != FALSE);
    }

    void ItemPostFrame() override
    {      
        if (m_pPlayer->pev->button & IN_RELOAD)
        {
            Reload();
            m_pPlayer->pev->button &= ~IN_RELOAD;
        }

        if (m_flTimeWeaponIdle2 != invalid_time_point && m_flTimeWeaponIdle2 < gpGlobals->time)
        {
            m_flTimeWeaponIdle2 = invalid_time_point;

            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                MusketFire(0.08, 2.3s, FALSE);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                MusketFire(0.06, 2.3s, FALSE);
            else
                MusketFire(0.02, 2.3s, FALSE);
        }

        CBasePlayerWeapon::ItemPostFrame();
    }

    void PrimaryAttack(void)
    {  
        if (m_flTimeWeaponIdle2 == invalid_time_point)
        {
            SendWeaponAnim(ANIM_SHOOTSTART, UseDecrement() != FALSE);

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 9999s;
            m_flTimeWeaponIdle2 = gpGlobals->time + 0.2s;
        }
    }

    void MusketFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
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

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192.0, 7, BULLET_PLAYER_50AE, GetDamage(), 0.98, m_pPlayer->pev, true, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0.0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), m_iClip, FALSE);
        
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (m_iClip <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
        m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 6, 7) * 0.75;
        m_pPlayer->pev->punchangle.y -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, -2, 2);
    }

    void Reload(void)
    {
        if (m_flTimeWeaponIdle2 != invalid_time_point && m_flTimeWeaponIdle2 < gpGlobals->time)
            m_flTimeWeaponIdle2 = gpGlobals->time + 0.01s;
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
 
        if (m_iClip)
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        else
            SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
        
    }

};
LINK_ENTITY_TO_CLASS(weapon_musket, CMusket)
}