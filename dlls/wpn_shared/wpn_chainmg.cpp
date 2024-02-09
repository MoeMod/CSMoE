/*
chainmg.cpp
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
    class CChainMG : public LinkWeaponTemplate< CChainMG,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 5.0s;
    static constexpr const char *V_Model = "models/v_chainmg.mdl";
    static constexpr const char *P_Model = "models/p_chainmg.mdl";
    static constexpr const char *W_Model = "models/w_chainmg.mdl";
    static constexpr const char *EventFile = "events/m249.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_chainmg";
    static constexpr const char *AnimExtension = "chainmg";
    static constexpr int MaxClip = 300;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_DRAW,
        ANIM_RELOAD,
        ANIM_SHOOT,
        ANIM_FIRE_AFTER,
        ANIM_FIRE_CHANGE,
        ANIM_IDLE_CHANGE,
	};

    static constexpr int SPMax = 100;
    static constexpr int SPCost = 50;
    static constexpr const auto& SPChargingTime = 0.32s;
    static constexpr const auto& SPChargingTimeZombie = 0.32s;
    static constexpr const auto& SPChargingTimeScenario = 0.32s;
    static constexpr const auto& SPCycleTime = 0.32s;

    static constexpr float IdleEffectSize = 50.0;   //???
    static constexpr float ShadowShootRadius = 20.0;
    static constexpr int ShadowCountLimit = 3;

    static constexpr float ShadowKnockbackGround = 0.0;
    static constexpr float ShadowKnockbackJump = 0.0;
    static constexpr float ShadowKnockbackJumpForward = 0.0;
    static constexpr float ShadowKnockbackDucking = 0.0;
    static constexpr float ShadowVelocityModifier = 0.0;

    static constexpr const auto& ShadowModelLifeTime = 0.32s;
    static constexpr const auto& ShadowModelFadeInTime = 0.32s;
    static constexpr const auto& ShadowModelFadeOutTime = 0.32s;

    static constexpr float ShadowModelFadeInStartAlpha = 10.0;
    static constexpr float ShadowModelBaseAlpha = 160.0;
    static constexpr float ShadowModelFadeOutEndAlpha = 10.0;
    static constexpr float ShadowModelOffsetLength = 50.0;
    static constexpr float ShadowModelNoTargetOffsetLength = -50.0;
    static constexpr float ShadowModelNoTargetDirection = 1.0;
    static constexpr float ShadowModelNoTargetAngle = 180.0;
    static constexpr float ShadowShowEffect1Scale = 0.8;
    static constexpr float ShadowShowEffect1ZOffset = 30.0;
    static constexpr float ShadowShowEffect2Scale = 1.5;
    static constexpr float ShadowShowEffect2ZOffset = 130.0;

    static constexpr float ShadowMP5Damage = 2.0;
    static constexpr float ShadowMP5DamageZombie = 190.0;
    static constexpr float ShadowMP5DamageScenario = 410.0;

    static constexpr const auto& ShadowMP5EnableDelay = 1.0s;
    static constexpr const auto& ShadowMP5LifeTime = 2.4s;
    static constexpr const auto& ShadowMP5CycleTime = 0.6s;
    static constexpr const auto& ShadowMP5FixDuration = 0.1s;
    static constexpr float ShadowMP5EnableFixSpeed = 150.0;

    static constexpr float ShadowChainMGDamage = 3.0;
    static constexpr float ShadowChainMGDamageZombie = 200.0;
    static constexpr float ShadowChainMGDamageScenario = 430.0;

    static constexpr const auto& ShadowChainMGEnableDelay = 1.0s;
    static constexpr const auto& ShadowChainMGLifeTime = 2.4s;
    static constexpr const auto& ShadowChainMGCycleTime = 0.06s;
    static constexpr const auto& ShadowChainMGFixDuration = 0.1s;
    static constexpr float ShadowChainMGFixSpeed = 150.0;

    static constexpr float ShadowAT4DirectDamage = 20.0;
    static constexpr float ShadowAT4DirectDamageZombie = 2000.0;
    static constexpr float ShadowAT4DirectDamageScenario = 9300.0;

    static constexpr float ShadowAT4ExplodeDamage = 20.0;
    static constexpr float ShadowAT4ExplodeDamageZombie = 6800.0;
    static constexpr float ShadowAT4ExplodeDamageScenario = 9300.0;

    static constexpr const auto& ShadowAT4EnableDelay = 1.0s;
    static constexpr float ShadowAT4SpawnAngle = 45.0;
    static constexpr float ShadowAT4SpawnDist = 3.0;
    static constexpr float ShadowAT4ExplodeRadius = 5.0;

    static constexpr const auto& ShadowAT4FixDuration = 1.25s;
    static constexpr float ShadowAT4FixSpeed = 70.0;
    static constexpr float ShadowAT4FixGravity = 100.0;
    static constexpr float ShadowAT4RocketSpeed = 2000.0;

    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 0;
    static constexpr const auto &SpreadCalcNotOnGround = 0.2*A + 0.03;
    static constexpr const auto &SpreadCalcWalking = 0.04*A + 0.02;
    static constexpr const auto &SpreadCalcDefault = 0.01*A;

	static constexpr const auto& CycleTime = 0.6s;
    static constexpr int DamageDefault = 38;
    static constexpr int DamageZB = 230;
    static constexpr int DamageZBS = 585;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyLimit = 1.2;
    static constexpr float AccuracyDiv = 200.0;
    static constexpr float AccuracyAdd = 0.3;
    static constexpr float RangeMod = 0.99;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 3;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8 };
	KickBackData KickBackNotOnGround = { 1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8 };
	KickBackData KickBackDucking = { 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 };
	KickBackData KickBackDefault = { 0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9 };

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/chainmg_hit1.wav");
        PRECACHE_SOUND("weapons/chainmg_hit2.wav");
        PRECACHE_SOUND("weapons/chainmg_shoot3.wav");
        PRECACHE_SOUND("weapons/chainmg_shoot4.wav");
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
        p->iId = m_iId = WEAPON_CHAINMG;
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


    float GetMaxSpeed(void)
    {
        switch (m_pPlayer->pev->weaponanim)
        {
        case ANIM_SHOOT:
        case ANIM_FIRE_CHANGE:
            return 60;
        case ANIM_RELOAD:
            return 80;
        default:
            return PlayerMaxSpeed;
        }
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        if (m_iClip <= 0)
        {
            SendWeaponAnim(ANIM_FIRE_AFTER, UseDecrement() != FALSE);

            if (m_fFireOnEmpty)
            {
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
            }

#ifndef CLIENT_DLL
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        SendWeaponAnim(ANIM_SHOOT, UseDecrement() != FALSE);

        if (m_pPlayer->pev->weaponanim != ANIM_SHOOT)
            return;

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
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, Penetration, BULLET_PLAYER_762MM, GetDamage(), RangeMod, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 0.4, 0.2, 0.15, 3.0, 2.0, 0);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.3, 0.075, 0.05, 0.02, 2.5, 1.5, 0);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.175, 0.04, 0.03, 0.01, 1.5, 1.0, 0);
        else
            KickBack(0.2, 0.05, 0.035, 0.015, 2.0, 1.25, 0);
    }

    void WeaponIdle(void)
    {

    }


    float GetSecondaryAttackDamage() const
    {
        float flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1150;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 600;
#endif
        return flDamage;
    }

    void SecondaryAttack(void)
    {
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
       
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_chainmg, CChainMG)
}