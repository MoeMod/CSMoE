/*
dbarrel.cpp
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
    class CDBarrel : public LinkWeaponTemplate< CDBarrel,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_XM1014>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_dbarrel.mdl";
    static constexpr const char *P_Model = "models/p_dbarrel.mdl";
    static constexpr const char *W_Model = "models/w_dbarrel.mdl";
    static constexpr const char *EventFile = "events/dbarrel.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_dbarrel";
    static constexpr const char *AnimExtension = "shotgun";
    static constexpr int MaxClip = 2;

    static constexpr float Spread = 0.055;
    static constexpr int NumShot = 7;
    static constexpr float Distance = 3048.0;
    static constexpr duration_t CycleTime = 0.15s;
    static constexpr duration_t DefaultDeployTime = 1.1s;
    static constexpr duration_t ReloadTime = 1.6s;


	enum
	{
        ANIM_IDLE1 = 0,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT3,
        ANIM_RELOAD,
        ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 240;

    static constexpr int DamageDefault = 28;
    static constexpr int DamageZB = 32;
    static constexpr int DamageZBS = 32;

    KnockbackData KnockBack = { 5000.0f, 800.0f, 1400.0f, 800.0f, 0.75f };

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "12gauge";
        p->iMaxAmmo1 = 40;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_DBARREL;
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
        m_iShellId = PRECACHE_MODEL("models/shotgunshell.mdl");

        Base::Precache();
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
            PlayEmptySound();

            m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.0s;

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif
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
        m_pPlayer->FireBullets(NumShot, m_pPlayer->GetGunPosition(), gpGlobals->v_forward, Vector(Spread, Spread, 0.0), Distance, BULLET_PLAYER_BUCKSHOT, 0, GetDamage());
#endif
        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, m_vVecAiming.x, Spread, NumShot, m_vVecAiming.x * 100, m_iClip != 0, FALSE);

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + CycleTime;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + CycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
      
        if (m_pPlayer->pev->flags & FL_ONGROUND)
            m_pPlayer->pev->punchangle.x -= 11.0;
        else
            m_pPlayer->pev->punchangle.x -= 16.0;
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, ANIM_RELOAD, ReloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_dbarrel, CDBarrel)
}