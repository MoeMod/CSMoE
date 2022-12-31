/*
thanatos3.cpp
Copyright (C) 2022 JustANoobQAQ

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
#ifndef CLIENT_DLL
#include "gamemode/mod_zb1.h"
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CThanatos3 : public LinkWeaponTemplate< CThanatos3,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_MP5N>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:

    static constexpr const auto& DefaultReloadTime = 3.2s;
    static constexpr const char *V_Model = "models/v_thanatos3.mdl";
    static constexpr const char *P_Model = "models/p_thanatos3.mdl";
    static constexpr const char *W_Model = "models/w_thanatos3.mdl";
    static constexpr const char *EventFile = "events/thanatos3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_thanatos3";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 60;

    static constexpr const auto& SpreadCalcNotOnGround = 0.375 * A;
    static constexpr const auto& SpreadCalcDefault = 0.035 * A;

    static constexpr const auto& CycleTime = 0.07s;

#define WPNSTATE_THANATOS3_MODEA 0
#define WPNSTATE_THANATOS3_MODEB 1
#define WPNSTATE_THANATOS3_MODEC 2
#define WPNSTATE_THANATOS3_MODED 3

	enum
	{
        ANIM_IDLE,
        ANIM_IDLE_W1,
        ANIM_IDLE_W2,
        ANIM_IDLE_W3,
        ANIM_SHOOT,
        ANIM_SHOOT_W1,
        ANIM_SHOOT_W2,
        ANIM_SHOOT_W3,
        ANIM_FLY_W1,
        ANIM_FLY_W2,
        ANIM_FLY_W3,
        ANIM_RELOAD,
        ANIM_RELOAD_W1,
        ANIM_RELOAD_W2,
        ANIM_RELOAD_W3,
        ANIM_SPREAD_W1,
        ANIM_SPREAD_W2,
        ANIM_SPREAD_W3,
        ANIM_DRAW,
        ANIM_DRAW_W1,
        ANIM_DRAW_W2,
        ANIM_DRAW_W3,
	};
    static constexpr float PlayerMaxSpeed = 255;
    static constexpr float ArmorRatioModifier = 1.2f;

    static constexpr int DamageDefault = 29;
    static constexpr int DamageZB = 47;
    static constexpr int DamageZBS = 130;

    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 1.5;

    KnockbackData KnockBack = { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };

    int m_iDefaultAmmo2;
    int m_iChargeShotsFired;

    void Spawn()
    {
        m_iDefaultAmmo2 = 0;
        m_iChargeShotsFired = 0;
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "46mm";
        p->iMaxAmmo1 = 80;
        p->pszAmmo2 = "thanatos3";
        p->iMaxAmmo2 = 12;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 15;
        p->iId = m_iId = WEAPON_THANATOS3;
        p->iFlags = 0;
        p->iWeight = UMP45_WEIGHT;

        return 1;
    }


    int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
    {
        if (m_iDefaultAmmo2)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
            m_iDefaultAmmo2 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }


#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_46mm" , AMMO_46MM_PRICE }; }
#endif
    float GetArmorRatioModifier() override { return 1.2; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 3, 0 }; }

    void Precache()
    {
        PRECACHE_MODEL("models/thanatos3_knife.mdl");
        PRECACHE_MODEL("models/thanatos3_wind.mdl");
        PRECACHE_MODEL("models/w_thanatos3b.mdl");
        
        PRECACHE_SOUND("weapons/thanatos3_wood1.wav");
        PRECACHE_SOUND("weapons/thanatos3_wood2.wav");
        PRECACHE_SOUND("weapons/thanatos3_stone1.wav");
        PRECACHE_SOUND("weapons/thanatos3_stone2.wav");
        PRECACHE_SOUND("weapons/thanatos3_metal1.wav");
        PRECACHE_SOUND("weapons/thanatos3_metal2.wav");
        PRECACHE_SOUND("weapons/thanatos3_knife_hit1.wav");
        PRECACHE_SOUND("weapons/thanatos3_knife_hit2.wav");
        PRECACHE_SOUND("weapons/thanatos3_knife_swish.wav");
        PRECACHE_SOUND("weapons/thanatos3_fly_shoot.wav");

        PRECACHE_SOUND("weapons/thanatos3_spread_w1.wav");
        PRECACHE_SOUND("weapons/thanatos3_spread_w2.wav");
        PRECACHE_SOUND("weapons/thanatos3_spread_w3.wav");
        PRECACHE_SOUND("weapons/thanatos3_fly_w1.wav");
        PRECACHE_SOUND("weapons/thanatos3_fly_w2.wav");
        PRECACHE_SOUND("weapons/thanatos3_fly_w3.wav");


        Base::Precache();
    }

    BOOL Deploy() override
    {
        m_flAccuracy = AccuracyDefault;
        m_bDelayFire = false;

        int iAnim = ANIM_DRAW + m_iWeaponState;

        return DefaultDeploy(V_Model, P_Model, iAnim, "carbine", UseDecrement() != FALSE);
    }

    void FireB()
    {

    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 190.0) + 0.6;
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

        if (m_iWeaponState < WPNSTATE_THANATOS3_MODED)
            m_iChargeShotsFired++;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 1, BULLET_PLAYER_45ACP, GetDamage(), 0.83, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        if (m_iChargeShotsFired >= 12 && m_iWeaponState < WPNSTATE_THANATOS3_MODED)
        {
            m_iWeaponState++;
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] += 2 * m_iWeaponState;
            m_iChargeShotsFired = 0;
        }

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState, m_iChargeShotsFired == 12);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 0.55, 0.4, 0.05, 5, 3.25, 9);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.6, 0.3, 0.25, 0.035, 3.5, 2.75, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.25, 0.175, 0.125, 0.02, 2.25, 1.2, 10);
        else
            KickBack(0.275, 0.2, 0.15, 0.02, 3, 1.75, 9);
    }

    void SecondaryAttack() override
    {
        if (m_iWeaponState < WPNSTATE_THANATOS3_MODEB)
            return;

        SendWeaponAnim(m_iWeaponState + ANIM_SHOOT_W3, UseDecrement() != FALSE);

        EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM, "weapons/thanatos3_fly_shoot.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.85s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.85s;

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        m_iWeaponState = WPNSTATE_THANATOS3_MODEA;
    }

    void WeaponIdle() override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        int iAnim = m_iWeaponState + ANIM_IDLE;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }

    void Reload() override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        int iAnim = m_iWeaponState + ANIM_RELOAD;

        if (DefaultReload(MaxClip, iAnim, DefaultReloadTime))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.0;
        }
    }
};
LINK_ENTITY_TO_CLASS(weapon_thanatos3, CThanatos3)
}