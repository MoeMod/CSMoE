/*
mechanicxm214.cpp
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
    class CMechanicXM214 : public LinkWeaponTemplate< CMechanicXM214,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.9s;
    static constexpr const char *V_Model = "models/z4b/v_mechanicxm214.mdl";
    static constexpr const char *P_Model = "models/z4b/p_mechanicxm214.mdl";
    static constexpr const char *W_Model = "models/z4b/w_mechanicxm214.mdl";
    static constexpr const char *EventFile = "events/mechanicxm214.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_mechanicxm214";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = -1;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_CHAGEMODE,
        ANIM_DRAW,
		ANIM_RUN,
	};
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.05;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 31;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 40;
    static constexpr const auto &AccuracyCalc = (N * N * N / 180.0) + 0.4;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1;
    static constexpr float RangeModifier = 0.96;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

#define MECHANICXM214_MAXAMMO   200
    
    int m_iHolsterAmmo;
    time_point_t m_flTimeHolster;
    time_point_t m_flTimeNextCharge;
    duration_t m_flNextRunIdle;
    duration_t m_flNextIdle;

    void Spawn()
    {
        Base::Spawn();
        m_iDefaultAmmo = MECHANICXM214_MAXAMMO;
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "M214Ammo";
        p->iMaxAmmo1 = MECHANICXM214_MAXAMMO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = -1;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_Z4B_MECHANICXM214;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

    void Precache() override
    {
        Base::Precache();

        PRECACHE_SOUND("CK_WPN/WEAPON/Mechanic/XM214_S.WAV");

    }

    BOOL Deploy() override
    {
        EMIT_SOUND_DYN(this->edict(), CHAN_ITEM, "CK_WPN/WEAPON/Mechanic/XM214_S.WAV", 0.8, ATTN_NORM, 0, PITCH_NORM);

        ResetRunIdle();

        if (m_flTimeHolster != invalid_time_point)
        {
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] += (gpGlobals->time - m_flTimeHolster) / 0.125s;
        }
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] >= MECHANICXM214_MAXAMMO)
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = MECHANICXM214_MAXAMMO;

        return Base::Deploy();
    }

    void ResetRunIdle()
    {
        m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
    }

    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        if (m_iDefaultAmmo)
        {
            m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] = m_iDefaultAmmo;
            m_iDefaultAmmo = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }

    void Holster(int skiplocal) override
    {
        SetThink(nullptr);

        m_iHolsterAmmo = m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType];
        m_flTimeHolster = gpGlobals->time;
        return CBasePlayerWeapon::Holster(skiplocal);
    }

    void ItemPostFrame() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < MECHANICXM214_MAXAMMO)
        {
            if (m_flTimeNextCharge <= gpGlobals->time)
            {
                m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]++;
                m_flTimeNextCharge = gpGlobals->time + 0.125s;
            }
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 190.0 + 0.3;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();

                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

            return;
        }

        ResetRunIdle();
        m_flTimeNextCharge = gpGlobals->time + 1s;
        m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_762MM, GetDamage(), RangeModifier, m_pPlayer->pev, false, m_pPlayer->random_seed);

        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.15, 0.36, 0.09, 0.165, 4.2, 4.25, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.95, 0.31, 0.14, 0.06, 2.45, 3.75, 1);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.55, 0.16, 0.06, 0.0275, 1.95, 2.75, 6);
        else
            KickBack(0.6, 0.21, 0.11, 0.03, 2.2, 3.0, 6);
    }

    void Reload()
    {
        return;
    }

    BOOL CanDrop() override { return FALSE; }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
        {
            if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
            {
                if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
                {
                    SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

                    m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.8s;
                }

                m_flNextIdle = UTIL_WeaponTimeBase();
            }
            else
            {
                if (m_flNextIdle <= UTIL_WeaponTimeBase())
                {
                    SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

                    m_flNextIdle = UTIL_WeaponTimeBase() + 20s;
                }

                m_flNextRunIdle = UTIL_WeaponTimeBase();
            }

            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.1s;
        }

    }
};

LINK_ENTITY_TO_CLASS(z4b_mechanicxm214, CMechanicXM214)
}