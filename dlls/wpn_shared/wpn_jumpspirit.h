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

    static constexpr float BirdKnockbackRadius = 3.5;
    static constexpr duration_t BirdKnockbackDuration = 0.5s;
    static constexpr float BirdKnockbackUp = 200.0;
    static constexpr float BirdKnockbackSide = 1510.0;
    static constexpr float SentryModeRadiusMod = 0.4;
    static constexpr duration_t SentryModeInterval = 0.5s;
    static constexpr duration_t SentryModeIntervalMod = 0.1s;
    static constexpr float SentryModeRadius = 5.4;


    static constexpr float BirdSpeed = 900.0;
    static constexpr duration_t BirdFlyTime = 4.0s;

    
#ifndef CLIENT_DLL
    class CJumpSpirit;
    class CJumpSpiritBird : public CBaseAnimating
    {
    public:
        void Spawn(void);
        void Precache(void);
        int GetWeaponsId(void);
        
        KnockbackData GetKnockBackData() override { return { 400.0f, 350.0f, 450.0f, 300.0f, 0.3f }; }
      
    public:
        void EXPORT BirdThink(void);
        
        void UpdateOnRemove(void);
        void FindTarget();
        void Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon);
        void RemoveBird();

        CBasePlayer* m_pOwner;
        CJumpSpirit* m_pWeapon;

    private:
        float m_flRadius;
        EHANDLE m_hTarget;
        time_point_t m_flNextAttack;

    };

    class CJumpSpiritField : public CBaseAnimating
    {
    public:
        void Spawn(void);
        void Precache(void);
        int GetWeaponsId(void);

        KnockbackData GetKnockBackData() override { return { 400.0f, 350.0f, 450.0f, 300.0f, 0.3f }; }

    public:
        void EXPORT FieldThink(void);
       
        void Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon);
        void FindTarget();
        void RemoveField();
        bool CheckEntInBox();

        CBasePlayer* m_pOwner;
        CJumpSpirit* m_pWeapon;

        float GetJumpSpeed() const
        {
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return 720.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return 420.0f;
#endif
            return 420.0f;
        }
    private:
        float m_flRadius;
        EHANDLE m_hTarget;
        time_point_t m_flNextAttack;
        time_point_t m_fLaunchTime;
        int m_iJumpFXIndex;

        static constexpr duration_t BirdActivateTime = 7.0s;
    };

    class CJumpSpiritProjectile : public CBaseAnimating
    {
    public:
        void Spawn(void);
        void Precache(void);
        int GetWeaponsId(void);

        KnockbackData GetKnockBackData() override { return { 400.0f, 350.0f, 450.0f, 300.0f, 0.3f }; }

    public:
        void EXPORT ProjectileThink(void);
        void EXPORT ProjectileTouch(CBaseEntity* pOther);

        void Init(CBasePlayer* pOwner, CJumpSpirit* pWeapon);
    private:
       
        time_point_t m_fLaunchTime;
        int m_iJumpFXIndex;
        CBasePlayer* m_pOwner;
        CJumpSpirit* m_pWeapon;
    };
#endif

    class CJumpSpirit : public LinkWeaponTemplate< CJumpSpirit,
        TGeneralData,
        BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
        TPrecacheEvent
    >
    {
    public:
        static constexpr const char* V_Model = "models/v_jumpspirit.mdl";
        static constexpr const char* P_Model = "models/p_jumpspirit.mdl";
        static constexpr const char* W_Model = "models/w_jumpspirit.mdl";
        static constexpr const char* EventFile = "events/jumpspirit.sc";
        static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
        static constexpr const char* ClassName = "weapon_jumpspirit";
        static constexpr const char* AnimExtension = "onehanded";
        static constexpr int MaxClip = 50;
        static constexpr float PlayerMaxSpeed = 265;

        KnockbackData KnockBack = { 750.0f, 500.0f, 650.0f, 550.0f, 0.75f };
        enum
        {
            ANIM_IDLE = 0,
            ANIM_SHOOT,
            ANIM_RELOAD,
            ANIM_DRAW,
            ANIM_JUMP,
            ANIM_HOLD_START,
            ANIM_HOLD_LOOP,
            ANIM_SHOOTB,    
        };

        

        int m_iJumpFXIndex;
        duration_t m_flTimeWeaponIdle2;
        time_point_t m_flTimeNextCharge;
        time_point_t m_flNextReload;
        time_point_t m_flTimeFlying;
        time_point_t m_flTimeCheckCharge;

        int m_iAModeState;
        time_point_t m_flTimeAModeFullCharge;
        time_point_t m_flTimeCheckAModeCharge;
#ifndef CLIENT_DLL
        CJumpSpiritBird* m_pBird;
#endif
        KnockbackData GetKnockBackData() override
        {
            return  { 750.0f, 500.0f, 650.0f, 550.0f, 0.75f };
        }

#ifndef CLIENT_DLL
        WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_jumpspirit" , 300 }; }
#endif
        float GetArmorRatioModifier() override { return 1.4; }
        WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 0 }; }

        int GetItemInfo(ItemInfo* p)
        {
            p->pszName = STRING(pev->classname);
            p->pszAmmo1 = "jumpspirit";
            p->iMaxAmmo1 = 50;
            p->pszAmmo2 = "jumpspirit_b";
            p->iMaxAmmo2 = 100;
            p->iMaxClip = 50;
            p->iSlot = 1;
            p->iPosition = 4;
            p->iId = m_iId = WEAPON_JUMPSPIRIT;
            p->iFlags = 0;
            p->iWeight = GLOCK18_WEIGHT;
            return 1;
        }

        void Spawn() override;
        void Precache() override;
        BOOL Deploy() override;
        void Reload() override;
        void PrimaryAttack() override;
        int ExtractAmmo(CBasePlayerWeapon* pWeapon) override; // sync Clip -> BpAmmo
        bool HasSecondaryAttack() override { return true; }
        void SecondaryAttack() override;
        void ItemPostFrame() override;
        void WeaponIdle() override;
        void Holster(int skiplocal) override;
        void UpdateOnRemove(void) override;

        void OnAttack2Released();
        void ResetChargeTimer();
        void RemoveBird();
        void ShootProjectile();
        void JumpSpiritFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

        float GetPrimaryAttackDamage() const
        {
            int flDamage = 24;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 240;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 418;
#endif
            return flDamage;
        }

        float GetSentryModeDamage() const
        {
            float flDamage = 3.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 80.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 388.0f;
#endif
            return flDamage;
        }

        float GetJumpSpeed() const
        {
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                return 325.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                return 222.0f;
#endif
            return 194.0f;
        }

        static constexpr float BirdSentryRadius = 5.4;
        static constexpr duration_t BirdSentryInterval = 2.0s;

        static constexpr duration_t AModeEnergyChargeInterval = 0.5s;
        static constexpr duration_t AModeMaxEnergy = 4.0s;
        static constexpr duration_t BModeChargeTime = 0.37s; //0.5s
        static constexpr duration_t BModeEnergyChargeInterval = 0.16s;
        static constexpr int BModeMaxEnergy = 100;
        static constexpr int BModeBirdUseEnergy = 5;
        static constexpr int BModeJumpUseEnergy = 4;
        static constexpr duration_t BModeDelayAfterAMode = 0.1s;
        static constexpr duration_t BModeDelayAfterBird = 0.5s;
        static constexpr duration_t BModeDelayAfterJump = 0.5s;
    };

}
