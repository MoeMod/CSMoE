/*
vulcanus7.cpp
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
    class CVulcanus7 : public LinkWeaponTemplate< CVulcanus7,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_vulcanus7.mdl";
    static constexpr const char *P_Model = "models/p_vulcanus7.mdl";
    static constexpr const char *W_Model = "models/w_vulcanus7.mdl";
    static constexpr const char *EventFile = "events/vulcanus7.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_vulcanus7";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 120;
	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEB,
        ANIM_IDLEA_EMPTY,
        ANIM_IDLEB_EMPTY,
        ANIM_CHANGE,
        ANIM_CHANGE_EMPTY,
		ANIM_SHOOTA1,
		ANIM_SHOOTB1,
        ANIM_SHOOTB2,
        ANIM_SHOOTC1_CHANGE,
        ANIM_SHOOTC1_CHANGE_EMPTY,
		ANIM_RELOADA,
        ANIM_RELOADB,
		ANIM_DRAWA,
        ANIM_DRAWB,
        ANIM_DRAWA_EMPTY,
        ANIM_DRAWB_EMPTY,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.3*A + 0.035;
    static constexpr const auto &SpreadCalcWalking = 0.02*A + 0.028;
    static constexpr const auto& SpreadCalcDucking = 0.02*A + 0.01;
    static constexpr const auto &SpreadCalcDefault = 0.0275*A;

	static constexpr const auto& CycleTime = 0.105s;

    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 35;
    static constexpr int DamageZBS = 51;

    static constexpr float AccuracyDefault = 0.25;
    static constexpr float AccuracyMax = 0.9;

    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

    time_point_t m_flTimeCheckState;
    int m_iDefaultAmmo2;
    int m_iChargeShotsFired;
    int m_iTargetShotsFired;

#define WPNSTATE_VULCANUS7_MODEB	2
#define VULCANUS7_CHANGE_TIME   5s

    void Spawn()
    {
        m_iDefaultAmmo2 = 5;
        Base::Spawn();
    }


    void Precache() override
    {
        PRECACHE_SOUND("weapons/vulcanus7-1.wav");
        PRECACHE_SOUND("weapons/vulcanus7-2.wav");
        PRECACHE_SOUND("weapons/vulcanus_exp.wav");
        PRECACHE_SOUND("weapons/vulcanus_shootc1_change.wav");


        PRECACHE_MODEL("models/p_vulcanus7b.mdl");
        PRECACHE_MODEL("models/w_vulcanus7b.mdl");

        return Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = "vulcanus7";
        p->iMaxAmmo2 = 5;
        p->iMaxClip = 120;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_VULCANUS7;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }


    int ExtractAmmo(CBasePlayerWeapon* pWeapon)
    {
        if (m_iDefaultAmmo2)
        {
            m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
            m_iDefaultAmmo2 = 0;
        }
        return CBasePlayerWeapon::ExtractAmmo(pWeapon);
    }


#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

    BOOL Deploy(void)
    {
        m_flAccuracy = AccuracyDefault;
        m_iShotsFired = 0;
  
        if (m_iWeaponState < 2)
        {
            m_iWeaponState = 0;
            m_flTimeCheckState = gpGlobals->time + VULCANUS7_CHANGE_TIME;
        }
           
        int iAnim;

        if (m_iWeaponState)
        {
            if (m_iClip)
                iAnim = ANIM_DRAWB;
            else
                iAnim = ANIM_DRAWB_EMPTY;

        }
        else
        {
            if (m_iClip)
                iAnim = ANIM_DRAWA;
            else
                iAnim = ANIM_DRAWA_EMPTY;
        }

        m_iChargeShotsFired = 0;
        m_iTargetShotsFired = RANDOM_LONG(8, 11);


        return DefaultDeploy(V_Model, P_Model, iAnim, "m249", UseDecrement() != FALSE);
    }

    void SecondaryAttack()
    {
        if (m_iWeaponState != WPNSTATE_VULCANUS7_MODEB)
            return;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Vulcanus7Missile(0.06 * m_flAccuracy, 2.0s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Vulcanus7Missile(0.04 * m_flAccuracy, 2.0s, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            Vulcanus7Missile(0.01 * m_flAccuracy, 2.0s, FALSE);
        else
            Vulcanus7Missile(0.02 * m_flAccuracy, 2.0s, FALSE);
    }

    void Vulcanus7Missile(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_flTimeCheckState = gpGlobals->time + VULCANUS7_CHANGE_TIME + 2.0s;

        m_iWeaponState = 0;

        SendWeaponAnim(m_iClip ? ANIM_SHOOTC1_CHANGE : ANIM_SHOOTC1_CHANGE_EMPTY, UseDecrement() != FALSE);

        EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/vulcanus_shootc1_change.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);

        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.0s;
    }

    void ItemPostFrame(void) override
    {
        if (m_iWeaponState < WPNSTATE_VULCANUS7_MODEB)
        {
            if (m_flTimeCheckState != invalid_time_point && gpGlobals->time >= m_flTimeCheckState)
            {
                switch (m_iWeaponState)
                {
                case 0:
                {
                    m_iWeaponState = 1;
                    SendWeaponAnim(m_iClip ? ANIM_CHANGE : ANIM_CHANGE_EMPTY, UseDecrement() != FALSE);
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.0s;
                    m_flTimeCheckState = gpGlobals->time + 1.0s;
                    break;
                }
                case 1:
                {
                    m_iWeaponState = WPNSTATE_VULCANUS7_MODEB;
                    m_flTimeCheckState = invalid_time_point;
                    break;
                }
                default:
                    break;
                }
            }
        }
       

        CBasePlayerWeapon::ItemPostFrame();
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35;

        if (m_flAccuracy > 0.9)
            m_flAccuracy = 0.9;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif
            return;
        }

        m_iClip--;

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();

        if(m_iWeaponState == WPNSTATE_VULCANUS7_MODEB)
            m_iChargeShotsFired++;

        if (m_iChargeShotsFired == m_iTargetShotsFired)
        {
            m_iChargeShotsFired = 0;
            m_iTargetShotsFired = RANDOM_LONG(7, 11);

            for (int i = 0; i < 3; i++)
            {
                m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamageB(), 1.0, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
            }
          
            PLAYBACK_EVENT_FULL(FEV_GLOBAL, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, 0.03, 0.03, 0, 0, TRUE, m_iWeaponState == WPNSTATE_VULCANUS7_MODEB);
        }
        else
        {
            Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.96, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

            PLAYBACK_EVENT_FULL(FEV_GLOBAL, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, m_iWeaponState == WPNSTATE_VULCANUS7_MODEB);
        }

        
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.03s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.68, 0.38, 0.175, 0.045, 3.8, 2.5, 5);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.2, 0.65, 0.25, 0.125, 5.0, 3.5, 20);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.45, 0.32, 0.156, 0.025, 2.3, 1.2, 9);
        else
            KickBack(0.58, 0.35, 0.168, 0.032, 2.8, 1.5, 7);
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(MaxClip, m_iWeaponState < WPNSTATE_VULCANUS7_MODEB ? ANIM_RELOADA : ANIM_RELOADB, 4.37s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif

            m_bDelayFire = false;
            m_iShotsFired = 0;
            m_iChargeShotsFired = 0;
            m_flAccuracy = AccuracyDefault;
            m_iTargetShotsFired = RANDOM_LONG(7, 11);
        }
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_iChargeShotsFired = 0;
        m_iTargetShotsFired = RANDOM_LONG(7, 11);

        int iAnim;

        if (m_iWeaponState == WPNSTATE_VULCANUS7_MODEB)
        {
            if (m_iClip)
                iAnim = ANIM_IDLEB;
            else
                iAnim = ANIM_IDLEB_EMPTY;

        }
        else
        {
            if (m_iClip)
                iAnim = ANIM_IDLEA;
            else
                iAnim = ANIM_IDLEA_EMPTY;
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        SendWeaponAnim(iAnim, UseDecrement() != FALSE);
    }
    
    float GetDamageB()
    {
        float flDamage = 20.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 45.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 95.0f;
#endif
        return flDamage;
    }

};
LINK_ENTITY_TO_CLASS(weapon_vulcanus7, CVulcanus7)
}