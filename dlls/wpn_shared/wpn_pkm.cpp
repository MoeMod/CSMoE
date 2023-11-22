/*
pkm.cpp
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
    class CPKM : public LinkWeaponTemplate< CPKM,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_pkm.mdl";
    static constexpr const char *P_Model = "models/p_pkm.mdl";
    static constexpr const char *W_Model = "models/w_pkm.mdl";
    static constexpr const char *EventFile = "events/pkm.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_pkm";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};

    static constexpr const auto& CycleTime = 0.09s;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    static constexpr int Penetration = 2;

    static constexpr float PlayerMaxSpeed = 221;
    static constexpr float ArmorRatioModifier = 1.5f;

    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 35;
    static constexpr int DamageZBS = 38;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_PKM;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }


    void PrimaryAttack(void)
    {
        BOOL bBurstMode = FBitSet(m_iWeaponState, WPNSTATE_FAMAS_BURST_MODE) ? TRUE : FALSE;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.055 + (0.5) * m_flAccuracy, 0.1s, FALSE, bBurstMode);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(0.035 + (0.075) * m_flAccuracy, 0.1s, FALSE, bBurstMode);
        else
            Fire((0.03) * m_flAccuracy + 0.01, 0.1s, FALSE, bBurstMode);
    }

    void SecondaryAttack(void)
    {
        if (m_iWeaponState & WPNSTATE_FAMAS_BURST_MODE)
        {
            ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_FullAuto");
            m_iWeaponState &= ~WPNSTATE_FAMAS_BURST_MODE;
        }
        else
        {
            ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "#Switch_To_BurstFire");
            m_iWeaponState |= WPNSTATE_FAMAS_BURST_MODE;
        }

        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.3s;
    }


    void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
    {
        m_iClip--;

        if (m_iClip < 0)
        {
            m_iClip = 0;
            shotsFired = 3;
            shootTime = invalid_time_point;
            return;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, Penetration, BulletType, GetDamage(), RangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        shotsFired++;

        if (shotsFired != 3)
        {
            shootTime = gpGlobals->time + 0.022s;
        }
        else
            shootTime = invalid_time_point;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim, BOOL bFireBurst)
    {
        if (bFireBurst != FALSE)
        {
            m_iFamasShotsFired = 0;
            flCycleTime = 0.26s;
        }
        else
            flSpread += 0.01;

        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0) + 0.375;

        if (m_flAccuracy > AccuracyMax)
            m_flAccuracy = AccuracyMax;

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
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BulletType, GetDamage(), RangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.25, 0.5, 0.375, 0.065, 4.0, 3.0, 8);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.65, 0.26, 0.225, 0.02, 3.0, 2.0, 7);
        else
            KickBack(0.8, 0.35, 0.34, 0.035, 4.0, 3.5, 9);

        if (bFireBurst != FALSE)
        {
            m_fBurstSpread = flSpread;
            m_iFamasShotsFired++;
            m_flFamasShoot = gpGlobals->time + 0.04s;
        }

    }


    		 	
};
LINK_ENTITY_TO_CLASS(weapon_pkm, CPKM)
}