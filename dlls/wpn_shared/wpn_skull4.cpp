/*
skull4.cpp
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
    class CSKULL4 : public LinkWeaponTemplate< CSKULL4,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TSecondaryAttackZoom,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.4s;
    static constexpr int ZoomFOV = 55;
    static constexpr const char *V_Model = "models/v_skull4.mdl";
    static constexpr const char *P_Model = "models/p_skull4.mdl";
    static constexpr const char *W_Model = "models/w_skull4.mdl";
    static constexpr const char *EventFile = "events/skull4_left.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull4";
    static constexpr const char *AnimExtension = "dualpistols";
    static constexpr int MaxClip = 48;
	enum
	{
        ANIM_IDLE1,
        ANIM_IDLE_LEFTEMPTY,
        ANIM_SHOOTLEFT1,
        ANIM_SHOOTLEFT2,
        ANIM_SHOOTLEFTLAST,
        ANIM_SHOOTRIGHT1,
        ANIM_SHOOTRIGHT2,
        ANIM_SHOOTRIGHTLAST,
        ANIM_RELOAD,
        ANIM_DRAW
	};
    static constexpr float PlayerMaxSpeed = 223;
    static constexpr const auto &SpreadCalcNotOnGround = 0.2*A;
    static constexpr const auto &SpreadCalcDefault = 0.04*A;

	static constexpr const auto& CycleTime = 0.204s;
    static constexpr int DamageDefault = 45;
    static constexpr int DamageZB = 67;
    static constexpr int DamageZBS = 160;
    static constexpr const auto &AccuracyCalc = (N * N * N / 215.0) + 0.25;
    static constexpr float AccuracyDefault = 0.15;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 3;
    KnockbackData KnockBack = { 250.0f, 650.0f, 750.0f, 300.0f, 0.4f };

	KickBackData KickBackWalking = { 0.4, 0.15, 0.21, 0.025, 3.0, 1.2, 5 };
	KickBackData KickBackNotOnGround = { 0.52, 0.5, 0.4, 0.02, 3.0, 2.0, 7 };
	KickBackData KickBackDucking = { 0.3, 0.12, 0.13, 0.02, 1.8, 0.8, 7 };
	KickBackData KickBackDefault = { 0.4, 0.13, 0.2, 0.02, 2.5, 1.0, 7 };
    
    int m_usFireSKULL4Left;
    int m_usFireSKULL4Right;

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556az";
        p->iMaxAmmo1 = 240;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 48;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_SKULL4;
        p->iFlags = 0;
        p->iWeight = SG552_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556az" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/skull4-1.wav");
        m_usFireSKULL4Left = PRECACHE_EVENT(1, "events/skull4_left.sc");
        m_usFireSKULL4Right = PRECACHE_EVENT(1, "events/skull4_right.sc");
    }

    BOOL Deploy(void)
    {
        m_flAccuracy = 0;
        m_iShotsFired = 0;
        m_bDelayFire = false;

        if (!(m_iClip & 1))
            m_iWeaponState |= WPNSTATE_ELITE_LEFT;

        BOOL fResult = DefaultDeploy(V_Model, P_Model, ANIM_DRAW, m_pPlayer->m_bIsFemale ? "dualpistols" : "dualpistols_2", UseDecrement() != FALSE);

        m_pPlayer->ResetMaxSpeed();

        return fResult;
    }


    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215) + 0.25;

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

        if (m_iWeaponState & WPNSTATE_ELITE_LEFT)
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
            m_iWeaponState &= ~WPNSTATE_ELITE_LEFT;
            eventindex = m_usFireSKULL4Left;
        }
        else
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
            m_iWeaponState |= WPNSTATE_ELITE_LEFT;
            eventindex = m_usFireSKULL4Right;
        }

        const float flDistance = 8192;
        const int iPenetration = Penetration;
        const Bullet iBulletType = BulletType;
        const int iDamage = GetDamage();
        const float flRangeModifier = RangeModifier;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, BulletType, iDamage, flRangeModifier, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), eventindex, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip, m_iWeaponState & WPNSTATE_ELITE_LEFT);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        Recoil();
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

        if (m_iClip)
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 60s;

            SendWeaponAnim(m_iClip == 1 ? ANIM_IDLE_LEFTEMPTY : ANIM_IDLE1, UseDecrement() != FALSE);
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_skull4, CSKULL4)
}