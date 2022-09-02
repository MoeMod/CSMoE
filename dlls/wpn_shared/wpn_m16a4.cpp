/*
m16a4.cpp
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
    class CM16A4 : public LinkWeaponTemplate< CM16A4,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.2s;
    static constexpr const char *V_Model = "models/v_m16a4.mdl";
    static constexpr const char *P_Model = "models/p_m16a4.mdl";
    static constexpr const char *W_Model = "models/w_m16a4.mdl";
    static constexpr const char *EventFile = "events/m16a4.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m16a4";
    static constexpr const char *AnimExtension = "carbine";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.2f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 130;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A+0.035;
    static constexpr const auto &SpreadCalcWalking = 0.02*A+0.04;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;
    static constexpr const auto& SpreadCalcDucking = 0.02*A;

	static constexpr const auto& CycleTime = 0.465s;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 49;
    static constexpr int DamageZBS = 49;
   
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 2.0;

    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 400.0f, 0.4f };
    		 	
    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_556NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = M4A1_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_M16A4;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.2; }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 130 }; }


    void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18)
    {
        m_iClip--;

        if (m_iClip < 0)
        {
            m_iClip = 0;
            shotsFired = 3;
            shootTime = invalid_time_point;
            return;
        }

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, m_fBurstSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);
    
        int flags;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, int(m_pPlayer->pev->punchangle.x * 100), int(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        shotsFired++;

        if (shotsFired != 3)
        {
            shootTime = gpGlobals->time + 0.04s;
        }
        else
            shootTime = invalid_time_point;
    }


    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_iFamasShotsFired = 0;

        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((float)(m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200.0) + 0.3;

        if (m_flAccuracy > 2)
            m_flAccuracy = 2;

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
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;

        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_556MM, GetDamage(), 0.97, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.y * 100), m_iClip != 0, FALSE);
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
       
        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.75, 0.3, 0.2, 0.06, 3, 2, 3);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.25, 0.45, 0.22, 0.18, 5, 4, 1);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.525, 0.2, 0.09, 0.02, 3, 1.5, 9);
        else
            KickBack(0.7, 0.2, 0.15, 005, 3, 1.5, 6);

        m_fBurstSpread = flSpread;
        m_iFamasShotsFired++;
        m_flFamasShoot = gpGlobals->time + 0.06s;
        
    }

};
LINK_ENTITY_TO_CLASS(weapon_m16a4, CM16A4)
}