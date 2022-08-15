/*
charger5.cpp
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
#include "weapons/KnifeAttack.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CCharger5 : public LinkWeaponTemplate< CCharger5,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_charger5.mdl";
    static constexpr const char *P_Model = "models/p_charger5.mdl";
    static constexpr const char *W_Model = "models/w_charger5.mdl";
    static constexpr const char *EventFile = "events/charger5.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_charger5";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1,
        ANIM_IDLE2,
        ANIM_SHOOT1,
        ANIM_SHOOT2,
        ANIM_SHOOT_LASER1,
        ANIM_SHOOT_LASER2,
        ANIM_RELOAD1,
        ANIM_RELOAD2,
		ANIM_DRAW1,
        ANIM_DRAW2,
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.0375;
    static constexpr const auto &SpreadCalcWalking = 0.05*A + 0.0325;
    static constexpr const auto &SpreadCalcDefault = 0.0325*A;

	static constexpr const auto& CycleTime = 0.09s;
    static constexpr int DamageDefault = 39;
    static constexpr int DamageZB = 69;
    static constexpr int DamageZBS = 69;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.6;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 170.0f, 0.6f };

    int m_iChargeShotsFired;

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "939mm";
        p->iMaxAmmo1 = 90;
		p->pszAmmo2 = "charger5laser";
		p->iMaxAmmo2 = 20;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CHARGER5;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    void Precache(void) override
    {
        PRECACHE_MODEL("sprites/muzzleflash63.spr");
        Base::Precache();
    }

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

    BOOL Deploy(void) override
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;

        return DefaultDeploy(V_Model, P_Model, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_DRAW2 : ANIM_DRAW1, "rifle", UseDecrement() != FALSE);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220.0 + 0.35;

        if (m_flAccuracy > 1.0)
            m_flAccuracy = 1.0;

        if (m_iClip <= 0)
        {
            if (m_fFireOnEmpty)
            {
                PlayEmptySound();

                m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            }

#ifndef CLIENT_DLL
            if (TheBots)
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
#endif

            return;
        }

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponVolume = BRIGHT_GUN_FLASH;

        Vector vecSrc = m_pPlayer->GetGunPosition();
        Vector vecDir;

        float flDamage = GetDamage();
        vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, flDamage, 0.6, m_pPlayer->pev, false, m_pPlayer->random_seed);

        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 20)
        {
            m_iChargeShotsFired++;
            if (m_iChargeShotsFired >= 5)
            {
                m_iChargeShotsFired = 0;
                m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]++;
            }
        }

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0, FALSE);

        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + flCycleTime;
#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", 0, 0);
#endif
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.0, 1.75, 0.55, 0.35, 8.0, 7.0, 4);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.5, 1.5, 0.225, 0.07, 6.0, 3.5, 6);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.25, 0.5, 0.1, 0.025, 1.75, 2.5, 7);
        else
            KickBack(0.3, 0.6, 0.15, 0.035, 2.5, 2.75, 6);
    }

    void SecondaryAttack() override
    {
        if (m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] <= 0)
            return;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            LaserFire(0.5, 1.0s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 120)
            LaserFire(0.08, 1.0s, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 10)
            LaserFire(0.04, 1.0s, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            LaserFire(0.0, 1.0s, FALSE);
        else
            LaserFire(0.0, 1.0s, FALSE);
    }

    void LaserFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType]--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

        m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
        m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

        Vector vecDir = m_pPlayer->FireBullets3(m_pPlayer->GetGunPosition(), gpGlobals->v_forward, flSpread, 8192, 3, BULLET_PLAYER_RAILBEAM, GetDamageB(), 0.99, m_pPlayer->pev, TRUE, m_pPlayer->random_seed);

        int flags;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, ENT(m_pPlayer->pev), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, (int)(m_pPlayer->pev->punchangle.x * 100), (int)(m_pPlayer->pev->punchangle.x * 100), m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] != 0, TRUE);
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.2s;

        m_pPlayer->pev->punchangle.x -= 2;
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        SendWeaponAnim(m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_IDLE2 : ANIM_IDLE1, UseDecrement() != FALSE);
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(30, m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] ? ANIM_RELOAD2 : ANIM_RELOAD1, 2.5s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    float GetDamageB() const
    {
        float flDamage = 69;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 299;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 299;
#endif
        return flDamage;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_939mm" , 90 }; }
#endif
    float GetArmorRatioModifier() override { return 1.6; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_charger5, CCharger5)
}