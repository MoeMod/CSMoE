/*
railgun.cpp
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
    class CRailGun : public LinkWeaponTemplate< CRailGun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GALIL>::template type,
            TPrecacheEvent,
            TDeployDefault
    >
    {
    public:

    static constexpr const auto &DefaultReloadTime = 3.1s;
    static constexpr const char *V_Model = "models/v_railgun.mdl";
    static constexpr const char *P_Model = "models/p_railgun.mdl";
    static constexpr const char *W_Model = "models/w_railgun.mdl";
    static constexpr const char *EventFile = "events/railgun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_railgun";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1,
        ANIM_IDLE_CHARGED,
        ANIM_SHOOT,
        ANIM_SHOOT_CHARGING,
        ANIM_SHOOT_CHARGED,
        ANIM_CHARGE,
        ANIM_CHARGE_FINISH,
        ANIM_DRAW,	
	};
    static constexpr float PlayerMaxSpeed = 200;
    static constexpr float ArmorRatioModifier = 1.55f;

    KnockbackData KnockBack = { 380.0f, 280.0f, 330.0f, 130.0f, 0.6f };

    int m_iChargeState;
    time_point_t m_flTimeCheckCharge;

    public:
    void Precache() override
    {
        Base::Precache();
        PRECACHE_MODEL("sprites/muzzleflash19.spr");
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "RailBeam";
        p->iMaxAmmo1 = 35;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 17;
        p->iId = m_iId = WEAPON_RAILGUN;
        p->iFlags = 0;
        p->iWeight = GALIL_WEIGHT;

        return 1;
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        m_iClip = WEAPON_NOCLIP;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = 35;
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    float GetDamage(int iState) const
    {
        float flDamage;

        if (iState >= 2)
        {
            flDamage = 110;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 850;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 3810;
#endif
        }
        else
        {
            flDamage = 100;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 620;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 2550;
#endif
        }

        return flDamage;
    }

    BOOL Deploy(void)
    {
        m_iChargeState = 0;
        m_flTimeCheckCharge = gpGlobals->time + 999999s;
        return Base::Deploy();
    }

    void PrimaryAttack(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.55, 2.55s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 120)
            Fire(0.1, 2.55s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 10)
            Fire(0.05, 2.55s, FALSE);
        else
            Fire(0.0, 2.55s, FALSE);
    }

    void OnAttack2Released(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.55, 2.55s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 120)
            Fire(0.1, 2.55s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 10)
            Fire(0.05, 2.55s, FALSE);
        else
            Fire(0.0, 2.55s, FALSE);
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
    
        if (m_iChargeState && !(m_pPlayer->pev->button & IN_ATTACK2))
        {
            OnAttack2Released();
        }

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        SendWeaponAnim(ANIM_IDLE1);
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20.0s;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        {
            PlayEmptySound();

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

            return;
        }

        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->punchangle + m_pPlayer->pev->v_angle);
        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

        float flDamage = GetDamage(m_iChargeState);

        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 10, BULLET_PLAYER_RAILBEAM, flDamage, 1.0, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), m_iChargeState, m_iClip > 0, TRUE);

        m_iChargeState = 0;

#ifndef CLIENT_DLL
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.6s;
        m_pPlayer->pev->punchangle.x -= 10.0;
    }

    void SecondaryAttack(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;

#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif
            return;
        }

        if (m_iChargeState)
        {
            if (gpGlobals->time > m_flTimeCheckCharge)
            {
                if (m_iChargeState > 1)
                {
                    SendWeaponAnim(ANIM_IDLE_CHARGED);

                    m_iChargeState = 3;
                    m_flTimeCheckCharge = gpGlobals->time + 999999s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3s;
                }
                else
                {
                    SendWeaponAnim(ANIM_CHARGE_FINISH);

                    m_iChargeState = 2;
                    m_flTimeCheckCharge = gpGlobals->time + 0.7s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.7s;
                }
            }
        }
        else
        {
            SendWeaponAnim(ANIM_CHARGE);

            m_flTimeCheckCharge = gpGlobals->time + 1.43s;
            m_iChargeState = 1;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.43s;
        }
    }

    void Reload(void) override
    {
    }

    float GetArmorRatioModifier() override { return 2.0; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_railbeam" , 180 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { 0, 6, 4, 0 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_railgun, CRailGun)
}