/*
wpn_cannon.cpp - Black Dragon Cannon from CSO
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/KnifeAttack.h"

#include "weapons/WeaponTemplate.hpp"
#ifndef CLIENT_DLL
#include "soundent.h"
#include "gamemode/mods.h"
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif


    class CM4A1_S_Transformers : public LinkWeaponTemplate<CM4A1_S_Transformers,
        TGeneralData,
        BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
        TPrecacheEvent,
        TPrimaryAttackRifle,
        TReloadDefault,
        TFireRifle,
        TRecoilKickBack,
        TGetDamageDefault
    >
    {
    public:
        static constexpr const char* ClassName = "z4b_m4a1_s_transformers";
        enum m4a1_s_transformers_e
        {
            ANIM_IDLE1,
            ANIM_RELOAD,
            ANIM_DRAW,
            ANIM_SHOOT1,
            ANIM_SHOOT2,
            ANIM_SHOOT3,
            ANIM_RUN,
            ANIM_KNIFEATTACK,
        };

        static constexpr const auto& DefaultReloadTime = 1.6s;
        static constexpr const char* V_Model = "models/z4b/PV-M4A1_S_Transformers.mdl";
        static constexpr const char* P_Model = "models/z4b/pw_m4a1_s_transformers.mdl";
        static constexpr const char* W_Model = "models/z4b/pw_m4a1_s_transformers.mdl";
        static constexpr const char* EventFile = "events/m4a1_s_transformers.sc";
        static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
        static constexpr const char* AnimExtension = "rifle";
        static constexpr int MaxClip = 35;
        static constexpr float PlayerMaxSpeed = 235;
        static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
        static constexpr const auto& SpreadCalcWalking = 0.07 * A + 0.04;
        static constexpr const auto& SpreadCalcDucking = 0.0 * A;
        static constexpr const auto& SpreadCalcDefault = 0.0275 * A;
        static constexpr const auto& CycleTime = 0.09s;
        static constexpr int DamageDefault = 36;
        static constexpr int DamageZB = 57;
        static constexpr int DamageZBS = 57;
        static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
        static constexpr float AccuracyDefault = 0.2;
        static constexpr float AccuracyMax = 1.25;
        static constexpr float RangeModifier = 0.98;
        static constexpr auto BulletType = BULLET_PLAYER_762MM;
        static constexpr int Penetration = 2;
        KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

        KickBackData KickBackWalking = { 0.9f, 0.35f, 0.225f, 0.05f, 5.5f, 2.5f, 8 };
        KickBackData KickBackNotOnGround = { 1.5f, 0.9f, 0.4f, 0.3f, 7.0f, 4.5f, 8 };
        KickBackData KickBackDucking = { 0.55f, 0.3f, 0.12f, 0.023f, 3.5f, 1.15f, 8 };
        KickBackData KickBackDefault = { 0.665f, 0.35f, 0.155f, 0.035f, 4.0f, 1.55f, 7 };


    public:

        void Spawn(void) override
        {
            Base::Spawn();
            m_iId = WEAPON_Z4B_M4A1_S_TRANSFORMERS;
        }

        int GetItemInfo(ItemInfo* p) override
        {
            p->pszName = STRING(pev->classname);
            p->pszAmmo1 = "556Nato";
            p->iMaxAmmo1 = MAX_AMMO_556NATO;
            p->pszAmmo2 = NULL;
            p->iMaxAmmo2 = -1;
            p->iMaxClip = 35;
            p->iSlot = 0;
            p->iPosition = 1;
            p->iId = m_iId = WEAPON_Z4B_M4A1_S_TRANSFORMERS;
            p->iFlags = 0;
            p->iWeight = M4A1_WEIGHT;

            return 1;
        }

        float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
        WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
        WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }
    public:

        void ResetRunIdle()
        {
            m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
        }

        BOOL Deploy() override;
        void Reload() override;
        void WeaponIdle(void) override;
        void SecondaryAttack(void) override;
        void DelaySecondaryAttack();
        float GetSecondaryAttackDamage() const;
     
    private:
        duration_t m_flNextRunIdle;
        duration_t m_flNextIdle;
     
    };

        LINK_ENTITY_TO_CLASS(z4b_m4a1_s_transformers, CM4A1_S_Transformers)

 BOOL CM4A1_S_Transformers::Deploy()
{
    ResetRunIdle();
    m_flAccuracy = AccuracyDefault;
  
    if (DefaultDeploy(V_Model, P_Model, ANIM_DRAW, AnimExtension, UseDecrement() != FALSE))
    {
        pev->body = m_pPlayer->m_bIsFemale ? 3 : 1;
        return TRUE;
    }
  
    return FALSE;
}

void CM4A1_S_Transformers::Reload(void)
{
    if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
        return;

    if (DefaultReload(MaxClip, ANIM_RELOAD, DefaultReloadTime))
    {
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_RELOAD);

        //UTIL_EMIT_PV_PARTICLE(m_pPlayer->edict(), "resource/weapontexture/SGFX_se_smoke_04.tga", 1, 6, 3, 60, 400, 2, 255, 255, 255, 1, 2, { 0.0, 0.0, 0.0 }, { 0, -0.5, -0.5 }, { 255, 255, 255 }, { 255, 255, 255 }, 255, 0);
#endif
        ResetRunIdle();
        m_iWeaponState = 0;
        m_flAccuracy = 0.0;
        m_iShotsFired = 0;
        m_bDelayFire = false;
    }
}

float CM4A1_S_Transformers::GetSecondaryAttackDamage() const
{
    float flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 600;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 1000;
#endif
    return flDamage;
}

void CM4A1_S_Transformers::DelaySecondaryAttack()
{
#ifndef CLIENT_DLL
    switch (KnifeAttack5(GetSecondaryAttackDamage(), 90, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
    {
    case HIT_NONE:
    {
        break;
    }
    case HIT_PLAYER:
    {
        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Hit.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
        break;
    }
    case HIT_WALL:
    {
        UTIL_EMIT_SOUND(m_pPlayer->edict(), CHAN_WEAPON, "CK_WPN/WEAPON/DEAGLE_EL/DEAGLE_EL_Stab.wav", VOL_NORM, ATTN_NORM, 0, PITCH_NORM);
        break;
    }
    }
#endif
}


void CM4A1_S_Transformers::WeaponIdle(void)
{
    ResetEmptySound();
    m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

    if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
    {
        if (m_pPlayer->pev->button & (IN_FORWARD | IN_MOVELEFT | IN_MOVERIGHT | IN_BACK))
        {
            if (m_flNextRunIdle <= UTIL_WeaponTimeBase())
            {
                SendWeaponAnim(ANIM_RUN, UseDecrement() != FALSE);

                m_flNextRunIdle = UTIL_WeaponTimeBase() + 0.83s;
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

void CM4A1_S_Transformers::SecondaryAttack(void)
{
#ifndef CLIENT_DLL
    m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif

    pev->nextthink = gpGlobals->time + 0.147s; 

    m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.66s;
    m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.66s;
    m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.66s;
    
    SendWeaponAnim(ANIM_KNIFEATTACK, UseDecrement() != FALSE);
    SetThink(&CM4A1_S_Transformers::DelaySecondaryAttack);

    ResetRunIdle();
}

}
