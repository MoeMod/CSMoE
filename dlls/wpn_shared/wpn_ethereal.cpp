/*
ethereal.cpp
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
    class CEthereal : public LinkWeaponTemplate< CEthereal,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GALIL>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TPrimaryAttackRifle,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr const auto &DefaultReloadTime = 3.1s;
    static constexpr const char *V_Model = "models/v_ethereal.mdl";
    static constexpr const char *P_Model = "models/p_ethereal.mdl";
    static constexpr const char *W_Model = "models/w_ethereal.mdl";
    static constexpr const char *EventFile = "events/ethereal.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_ethereal";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1,   
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,	
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.035;
    static constexpr const auto &SpreadCalcWalking = 0.065*A+ 0.025 + 0.03;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.088s;
    static constexpr float AccuracyDefault = 0.18;

    KnockbackData KnockBack = { 380.0f, 280.0f, 330.0f, 130.0f, 0.6f };

    public:
    void Precache() override
    {
        Base::Precache();
        PRECACHE_MODEL("sprites/muzzleflash19.spr");
    }

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "ether";
        p->iMaxAmmo1 = 90;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 30;
        p->iSlot = 0;
        p->iPosition = 17;
        p->iId = m_iId = WEAPON_ETHEREAL;
        p->iFlags = 0;
        p->iWeight = GALIL_WEIGHT;

        return 1;
    }

    float GetDamage() const
    {
        float flDamage = 33;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 45;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 100;
#endif
        return flDamage;
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3;

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
            if (TheBots != NULL)
            {
                TheBots->OnEvent(EVENT_WEAPON_FIRED_ON_EMPTY, m_pPlayer);
            }
#endif

            return;
        }

        m_iClip--;
        m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
        UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);
        Vector vecSrc = m_pPlayer->GetGunPosition();

        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_ETHEREAL, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        FireEvent(vecDir);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.4, 0.7, 0.32, 0.22, 3.6, 3.4, 7);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(0.68, 0.35, 0.2, 0.04, 3.4, 3.2, 8);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.41, 0.11, 0.16, 0.011, 2.1, 1.61, 9);
        else
            KickBack(0.42, 0.12, 0.13, 0.012, 1.9, 1.53, 9);
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);
    }

    void Reload(void)
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(30, ANIM_RELOAD, 3.1s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    float GetArmorRatioModifier() override { return 1.65; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_ether" , 70 }; }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_ethereal, CEthereal)
}