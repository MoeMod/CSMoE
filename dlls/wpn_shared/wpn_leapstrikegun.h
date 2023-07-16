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

    class CLeapStrikeGun : public LinkWeaponTemplate< CLeapStrikeGun,
        TGeneralData,
        BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
        TPrecacheEvent
    >
    {
    public:
        static constexpr const char* V_Model = "models/v_leapstrikegun.mdl";
        static constexpr const char* P_Model = "models/p_bufffiveseven.mdl";
        static constexpr const char* W_Model = "models/w_bufffiveseven.mdl";
        static constexpr const char* EventFile = "events/leapstrikegun.sc";
        static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
        static constexpr const char* ClassName = "weapon_leapstrikegun";
        static constexpr const char* AnimExtension = "leapstrikegun";
        static constexpr int MaxClip = 50;
        KnockbackData KnockBack = { 450.0f, 350.0f, 350.0f, 260.0f, 0.3f };
        enum
        {
            ANIM_IDLE = 0,
            ANIM_SHOOT1,
            ANIM_SHOOT2,
            ANIM_DRAW,
            ANIM_RELOAD,
        };

        enum part_e
        {
            PART_IDLE = 0,
            PART_CHANGE_START,
            PART_CHANGE,
            PART_CHANGE_END,
        };

        int m_iJumpFXIndex;
        int m_iLandingIndex;
        int m_iExpIndex;
        int m_iFieldIndex;
        int m_iChargingShotIndex[2];
        int m_iDefaultAmmo2;
        time_point_t m_flTimeNextCharge;
        time_point_t m_flNextReload;
        time_point_t m_flTimeFlying;

        KnockbackData GetKnockBackData() override
        {
            return  { 420.0f, 380.0f, 380.0f, 550.0f, 0.8f };
        }

#ifndef CLIENT_DLL
        WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_restrictionpistol" , 300 }; }
        void FireBullet3CallBack(int iPenetration, Vector vecDir, TraceResult* pTrace) override;
#endif
        float GetArmorRatioModifier() override { return 1.5; }
        WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 3, 0 }; }

        int GetItemInfo(ItemInfo* p)
        {
            p->pszName = STRING(pev->classname);
            p->pszAmmo1 = "restrictionpistol";
            p->iMaxAmmo1 = 50;
            p->pszAmmo2 = "leapstrikegun2";
            p->iMaxAmmo2 = 100;
            p->iMaxClip = 50;
            p->iSlot = 1;
            p->iPosition = 4;
            p->iId = m_iId = WEAPON_LEAPSTRIKEGUN;
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

        void RestrictionPistoFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
        void RadiusAttack1();
        void RadiusAttack2();
        void DropAttack();
        void FlyThink();

        void Holster(int skiplocal)
        {
            m_flTimeNextCharge = gpGlobals->time;

            return CBasePlayerWeapon::Holster(skiplocal);
        }

        duration_t GetFlyingTime()
        {
            duration_t flTime = 8.5s;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flTime = 4.0s;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flTime = 1.5s;
#endif
            return flTime;
        }

        duration_t GetAvoidInfectTime()
        {
            duration_t flTime = 0.0s;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flTime = 1.5s;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flTime = 1.5s;
#endif
            return flTime;
        }

        duration_t GetChargeTime()
        {
            duration_t flTime = 0.05s;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flTime = 0.1s;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flTime = 0.25s;
#endif
            return flTime;
        }

        float GetPrimaryAttackDamage() const
        {
            int flDamage = 20;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 200;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 310;
#endif
            return flDamage;
        }

        float GetSecondaryAttackDamage() const
        {
            float flDamage = 5.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 108.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 280.0f;
#endif
            return flDamage;
        }

        float GetRadiusDamage(int iState) const
        {
            float flDamage;
            if (iState == 2)
                flDamage = 11.0;
            else if (iState == 1)
                flDamage = 20.0;
            else
                flDamage = 32.0;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
            {
                if (iState == 2)
                    flDamage = 314.0;
                else if (iState == 1)
                    flDamage = 900.0;
                else
                    flDamage = 3200.0;
            }
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
            {
                if (iState == 2)
                    flDamage = 320.0;
                else if (iState == 1)
                    flDamage = 1400.0;
                else
                    flDamage = 4000.0;
            }
#endif
            return flDamage;
        }


        float GetRadius(int iState) const
        {
            float flRadius;
            if (iState == 2)
                flRadius = 393.7;
            else if (iState == 1)
                flRadius = 157.48;
            else
                flRadius = 393.7;
            return flRadius;
        }


    };
}
