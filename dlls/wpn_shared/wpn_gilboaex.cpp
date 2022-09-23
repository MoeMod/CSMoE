/*
gilboaex.cpp
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
    class CGilboaEX : public LinkWeaponTemplate< CGilboaEX,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M4A1>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.0s;
    static constexpr const char *V_Model = "models/v_gilboaex.mdl";
    static constexpr const char *P_Model = "models/p_gilboaex.mdl";
    static constexpr const char *W_Model = "models/w_gilboaex.mdl";
    static constexpr const char *EventFile = "events/gilboaex.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_gilboaex";
    static constexpr const char *AnimExtension = "ak47";
    static constexpr int MaxClip = 60;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.07 * A + 0.04;
    static constexpr const auto &SpreadCalcDefault = 0.0275 * A;

	static constexpr const auto& CycleTime = 0.088s;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

    KnockbackData GetKnockBackData() override
    {
        if (m_iWeaponState)
            return { 380.0f, 300.0f, 350.0f, 120.0f, 0.6f };
        else
            return { 200.0f, 220.0f, 250.0f, 90.0f, 0.5f };
    }
    		 	
    void Precache() override
    {
        PRECACHE_MODEL("sprites/muzzleflash21.spr");
        PRECACHE_MODEL("sprites/muzzleflash26.spr");

        PRECACHE_SOUND("weapons/gilboaex-1.wav");
        PRECACHE_SOUND("weapons/gilboaex-2.wav");

        return Base::Precache();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_556NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 60;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_GILBOAEX;
        p->iFlags = 0;
        p->iWeight = M4A1_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

    void PrimaryAttack()
    {
        m_iWeaponState = 0;
        Base::PrimaryAttack();
    }

    void SecondaryAttack()
    {
        m_iWeaponState = 1;
  
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.3 * m_flAccuracy + 0.03, 0.2s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 140)
            Fire(0.06 * m_flAccuracy + 0.03, 0.2s, FALSE);
        else
            Fire(0.0195 * m_flAccuracy, 0.2s, FALSE);
      
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;

        if (!m_iWeaponState)
        {
            m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 200.0) + 0.35;
            if (m_flAccuracy > 1.25)
                m_flAccuracy = 1.25;

            if (m_flAccuracy < 0.2)
                m_flAccuracy = 0.2;
        }
        else
        {
            m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 200.0) + 0.3;
            if (m_flAccuracy > 0.85)
                m_flAccuracy = 0.85;

            if (m_flAccuracy < 0.15)
                m_flAccuracy = 0.15;
        }
       

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();
                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }
            return;
        }

        if (m_iWeaponState)
        {
            if (m_iClip == 1)
            {
                PrimaryAttack();
                return;
            }
            m_iClip -= 2;
        }
        else
            m_iClip--;

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

     
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecForward = gpGlobals->v_forward;
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), RangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState ? true : false, FALSE);
        
        if (m_iWeaponState)
        {
            flSpread += 0.05;
            vecDir = m_pPlayer->FireBullets3(vecSrc, vecForward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), RangeModifier, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
            PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iWeaponState ? true : false, FALSE);

        }

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(2, 1, 0.5, 0.35, 7, 6, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.5, 0.45, 0.225, 0.05, 5.5, 2.5, 7);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.7, 0.35, 0.15, 0.025, 3, 1.3, 9);
        else
            KickBack(1.2, 0.375, 0.175, 0.038, 4.25, 2, 8);
    }


    float GetDamage(void)
    {
        float flDamage;
        if (!m_iWeaponState)
        {
            flDamage = 32.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 50.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 50.0f;
#endif
            return flDamage;
        }
        else
        {
            flDamage = 28.0f;
#ifndef CLIENT_DLL
            if (g_pModRunning->DamageTrack() == DT_ZB)
                flDamage = 50.0f;
            else if (g_pModRunning->DamageTrack() == DT_ZBS)
                flDamage = 66.0f;
#endif
            return flDamage;
        }
    }

};
LINK_ENTITY_TO_CLASS(weapon_gilboaex, CGilboaEX)
}