/*
wpn_cannon.h - Black Dragon Cannon from CSO
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

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
        static constexpr const auto& DefaultReloadTime = 5.0s;
        static constexpr const char* V_Model = "models/v_chainmg.mdl";
        static constexpr const char* P_Model = "models/p_chainmg.mdl";
        static constexpr const char* W_Model = "models/w_chainmg.mdl";
        static constexpr const char* EventFile = "events/chainmg.sc";
        static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
        static constexpr const char* ClassName = "weapon_chainmg";
        static constexpr const char* AnimExtension = "chainmg";
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

        enum m134_state_e
        {
            WPNSTATE_M134_IDLE,
            WPNSTATE_M134_SPIN_UP,
            WPNSTATE_M134_SPINNING,
            WPNSTATE_M134_SPIN_DOWN

        };

        static constexpr int SPMax = 100;
        static constexpr int SPCost = 50;
        static constexpr const auto& SPChargingTime = 0.32s;
        static constexpr const auto& SPChargingTimeZombie = 0.32s;
        static constexpr const auto& SPChargingTimeScenario = 0.32s;
        static constexpr const auto& SPCycleTime = 0.32s;

        //static constexpr float IdleEffectSize = 50.0;   //???
        static constexpr float ShadowShootRadius = 20.0;
        static constexpr int ShadowCountLimit = 3;

        //static constexpr float ShadowModelOffsetLength = 50.0;
        //static constexpr float ShadowModelNoTargetOffsetLength = -50.0;
        //static constexpr float ShadowModelNoTargetDirection = 1.0;
        //static constexpr float ShadowModelNoTargetAngle = 180.0;
        //static constexpr float ShadowShowEffect1Scale = 0.8;
        //static constexpr float ShadowShowEffect1ZOffset = 30.0;
        //static constexpr float ShadowShowEffect2Scale = 1.5;
        //static constexpr float ShadowShowEffect2ZOffset = 130.0;

        static constexpr float ShadowAT4SpawnAngle = 45.0;
        static constexpr float ShadowAT4SpawnDist = 3.0;

        static constexpr float PlayerMaxSpeed = 230;
        static constexpr float ArmorRatioModifier = 1.5f;
        static constexpr float PrimaryAttackWalkingMiniumSpeed = 0;
        static constexpr const auto& SpreadCalcNotOnGround = 0.2 * A + 0.03;
        static constexpr const auto& SpreadCalcWalking = 0.04 * A + 0.02;
        static constexpr const auto& SpreadCalcDefault = 0.01 * A;

        static constexpr const auto& CycleTime = 0.06s;
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

        int m_iLastAnim;
        int m_iSkillModelIndex[2];

        void Spawn() override;
        void Precache() override;
       
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
        void CreateEnt(float flYaw);
#endif
        float GetArmorRatioModifier() override { return 1.5; }
        WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_SPEED, 6, 3, 140 }; }

        void WeaponIdle() override;
        void SecondaryAttack() override;
        void WpnDeathCallBack(CBaseEntity* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;

        void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
        void ShadowSearchForEnemy(CBaseEntity* pIgnored, Vector vecOrigin, int iType, float flDamage, int iLastHitGroup);
       
    };
}
