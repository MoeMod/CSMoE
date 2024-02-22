/*
m134hero.cpp
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
#include "weapons/knifeattack.h"
#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CM134hero : public LinkWeaponTemplate< CM134hero,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 4.8s;
    static constexpr const char *V_Model = "models/v_m134hero.mdl";
    static constexpr const char *P_Model = "models/p_m134hero.mdl";
    static constexpr const char *W_Model = "models/w_m134hero.mdl";
    static constexpr const char *EventFile = "events/m134hero.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m134hero";
    static constexpr const char *AnimExtension = "m134";
    static constexpr int MaxClip = 300;
	enum
	{
        ANIM_IDLE1,
        ANIM_DRAW,
        ANIM_RELOAD,
        ANIM_FIRE_READY,
        ANIM_SHOOT1,
        ANIM_FIRE_AFTER,
        ANIM_FIRE_CHANGE,
        ANIM_IDLE_CHANGE,
        ANIM_DRAW_OVERHEAT,
        ANIM_FIRE_AFTER_OVERHEAT,
        ANIM_IDLE_OVERHEAT,
        ANIM_OVERHEAT_END,
        ANIM_FIRE_READY_SHOOTB,
        ANIM_SHOOT2,
        ANIM_FIRE_AFTER_SHOOTB
	};

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 0;
    static constexpr const auto& SpreadCalcNotOnGround = 0.1 * A + 0.05;
    static constexpr const auto& SpreadCalcWalking = 0.02 * A + 0.04;
    static constexpr const auto& SpreadCalcDefault = 0.0125 * A + 0.025;

	static constexpr const auto& CycleTime = 0.06s;
    static constexpr const auto& CycleTime2 = 0.02s;
    static constexpr int DamageDefault = 20;
    static constexpr int DamageZB = 85;
    static constexpr int DamageZBS = 85;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyLimit = 2.5;
    static constexpr float AccuracyDiv = 200.0;
    static constexpr float AccuracyAdd = 0.4;
    static constexpr float RangeMod = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8 };
	KickBackData KickBackNotOnGround = { 1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8 };
	KickBackData KickBackDucking = { 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 };
	KickBackData KickBackDefault = { 0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9 };

    time_point_t m_flNextCheck;
    bool m_bOverheat;  //iuser1
    bool m_bRapidFiring;  //iuser2
    bool m_bShoting;
    int m_iShooted;
    unsigned short m_usM134heroFireAfter;


    float GetRapidFireDamage()
    {
        float flDamage = 15.0f;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 75.0f;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 75.0f;
#endif
        return flDamage;
    }

    void Reload() override
    {
        if (m_bOverheat)
            return;

        if (m_pPlayer->pev->weaponanim == ANIM_OVERHEAT_END && m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        Base::Reload();
    }

    void Spawn()
    {
        m_flNextCheck = invalid_time_point;
        Base::Spawn();
    }


    void Precache(void)
    {
        m_usM134heroFireAfter = PRECACHE_EVENT(1, "events/m134heroFA.sc");
        Base::Precache();
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "chainmg";
        p->iMaxAmmo1 = 300;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = MaxClip;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_M134HERO;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_chainmg" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_SPEED, 6, 3, 140 }; }

    BOOL Deploy(void)
    {
        m_flAccuracy = 0;
        m_iShotsFired = 0;
        m_bRapidFiring = false;

        return DefaultDeploy(V_Model, P_Model, m_bOverheat ? ANIM_DRAW_OVERHEAT : ANIM_DRAW, AnimExtension, UseDecrement() != FALSE, 1.0s, 1.0s);
    }


    void ItemPostFrame() override
    {
        int iButton = m_pPlayer->pev->button;
        int flags;

        if (gpGlobals->time > m_flNextCheck)
        {
            if (m_bOverheat)
            {
                SendWeaponAnim(ANIM_OVERHEAT_END, UseDecrement() != FALSE);
                m_bOverheat = false;
                m_bRapidFiring = false;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
                m_flNextCheck = gpGlobals->time + 2.0s;
            }
            else if (!m_bRapidFiring && !m_bShoting && (iButton & IN_ATTACK2) && m_iClip)
            {
                SendWeaponAnim(ANIM_FIRE_READY_SHOOTB, UseDecrement() != FALSE);
                m_iShooted = 0;
                m_bShoting = true;
                m_iShotsFired = 0;
                m_bRapidFiring = true;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.15s;
                m_flNextCheck = gpGlobals->time + 0.15s;
            }
            else if (m_bRapidFiring && m_bShoting && (iButton & IN_ATTACK2) && m_iClip)
            {
                SecondaryAttack();
                m_flNextCheck = gpGlobals->time + CycleTime2;       
                return;
            }
            else if (m_bRapidFiring && m_bShoting && (!(iButton & IN_ATTACK2) || !m_iClip))
            {
                m_iShotsFired = 0;
                if (m_iShooted)
                {
                    SendWeaponAnim(ANIM_FIRE_AFTER_OVERHEAT, UseDecrement() != FALSE);

                    if (m_iShooted > 250)
                    {
                        m_flNextCheck = gpGlobals->time + 20.0s;
                    }
                    else if (m_iShooted > 10)
                    {
                        m_flNextCheck = gpGlobals->time + (0.08s * float(m_iShooted));
                    }
                    else
                        m_flNextCheck = gpGlobals->time + 3.0s;

                    m_bOverheat = true;
                    m_bRapidFiring = false;
                    m_iShooted = 0;
                    m_bShoting = false;
                    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 2.0s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5s;
                    m_pPlayer->m_flNextAttack = 2.0s;
                }
                else
                {
                    SendWeaponAnim(ANIM_FIRE_AFTER_SHOOTB, UseDecrement() != FALSE);
                    m_flNextCheck = gpGlobals->time + 0.2s;
                    m_bRapidFiring = false;
                    m_bShoting = false;
                    m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.85s;
                    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.85s;
                    m_pPlayer->m_flNextAttack = 0.85s;
                }


#ifdef CLIENT_WEAPONS
                flags = FEV_NOTHOST;
#else
                flags = 0;
#endif
                PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usM134heroFireAfter, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, FALSE);

            }
            else if (!m_bRapidFiring && !m_bShoting && (iButton & IN_ATTACK) && m_iClip)
            {
                SendWeaponAnim(ANIM_FIRE_READY, UseDecrement() != FALSE);
                m_bShoting = true;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5s;
                m_flNextCheck = gpGlobals->time + 0.1s;
            }
            else if (!m_bRapidFiring && m_bShoting && (iButton & IN_ATTACK) && m_iClip)
            {
                PrimaryAttack();
                m_flNextCheck = gpGlobals->time + CycleTime;
                return;
            }
            else if ((!m_bRapidFiring && m_bShoting && (!(iButton & IN_ATTACK) || !m_iClip)))
            {
                SendWeaponAnim(ANIM_FIRE_AFTER, UseDecrement() != FALSE);
                m_flNextCheck = gpGlobals->time + 0.2s;
                m_bShoting = false;
                m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0s;
                m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

#ifdef CLIENT_WEAPONS
                flags = FEV_NOTHOST;
#else
                flags = 0;
#endif
                PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usM134heroFireAfter, 0, g_vecZero, g_vecZero, 0.0, 0.0, 0, 0, FALSE, FALSE);

            }
        }

        CBasePlayerWeapon::ItemPostFrame();
    }

    void PrimaryAttack() override
    {
        if (m_flNextCheck > gpGlobals->time)
            return;

        Base::PrimaryAttack();
    }

    void SecondaryAttack() override
    {
        if (m_flNextCheck > gpGlobals->time)
            return;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire((0.1) * m_flAccuracy + 0.05, CycleTime2, false);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            Fire((0.02) * m_flAccuracy + 0.04, CycleTime2, false);
        else
            Fire((0.01) * m_flAccuracy + 0.025, CycleTime2, false);
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;

        SendWeaponAnim(m_bOverheat ? ANIM_IDLE_OVERHEAT : ANIM_IDLE1, UseDecrement() != FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_iClip <= 0)
        {     
            PlayEmptySound();
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / AccuracyDiv + AccuracyAdd;

        if (m_flAccuracy > AccuracyLimit)
            m_flAccuracy = AccuracyLimit;

        m_iShotsFired++;
        m_iClip--;
        m_bDelayFire = true;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_762MM, m_bRapidFiring ? GetRapidFireDamage() : GetDamage(), RangeMod, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_bRapidFiring, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.2s;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        if (m_bRapidFiring)
        {
            m_iShooted++;
            if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
                KickBackCustom(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
            else if (m_pPlayer->pev->velocity.Length2D() > 0)
                KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
            else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
                KickBackCustom(0.175, 0.04, 0.03, 0.01, 1.5, 1.0, 0);
            else
                KickBackCustom(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
        }
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBackCustom(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBackCustom(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBackCustom(0.175, 0.04, 0.03, 0.01, 1.5, 1.0, 0);
        else
            KickBackCustom(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
    }


    void KickBackCustom(float up_base, float lateral_base, float up_modifier, float lateral_modifier, float up_max, float lateral_max, int direction_change)
    {
        float up, lateral;
        int iRan;

        up = ((rand() % 10) * up_base) * 0.1;
        lateral = ((rand() % 10) * lateral_base) * 0.1;

        if (m_iShotsFired != 1)
        {
            up += (rand() % (m_iShotsFired * 2)) * up_modifier;
            lateral += (rand() % (m_iShotsFired * 2)) * lateral_modifier;
        }

        iRan = rand();

        if (!iRan || iRan % 2)
        {
            m_pPlayer->pev->punchangle.x += up;

            if (m_pPlayer->pev->punchangle.x > up_max)
                m_pPlayer->pev->punchangle.x = up_max;
        }
        else
        {
            m_pPlayer->pev->punchangle.x -= up;

            if (m_pPlayer->pev->punchangle.x < -up_max)
                m_pPlayer->pev->punchangle.x = -up_max;
        }

        if (rand() % 2 == 1)
        {
            m_pPlayer->pev->punchangle.y += lateral;

            if (m_pPlayer->pev->punchangle.y > lateral_max)
                m_pPlayer->pev->punchangle.y = lateral_max;
        }
        else
        {
            m_pPlayer->pev->punchangle.y -= lateral;

            if (m_pPlayer->pev->punchangle.y < -lateral_max)
                m_pPlayer->pev->punchangle.y = -lateral_max;
        }
    }
	 	
};
LINK_ENTITY_TO_CLASS(weapon_m134hero, CM134hero)
}