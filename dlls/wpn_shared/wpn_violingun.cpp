/*
violingun.cpp
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
    class CViolingun : public LinkWeaponTemplate< CViolingun,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.48s;
    static constexpr const char *V_Model = "models/v_violingun.mdl";
    static constexpr const char *P_Model = "models/p_violingun.mdl";
    static constexpr const char *W_Model = "models/w_violingun.mdl";
    static constexpr const char *EventFile = "events/violingun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_violingun";
    static constexpr const char *AnimExtension = "violingun";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_IDLE2,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
	};
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.04;
    static constexpr const auto &SpreadCalcDefault = 0.0275*A;

	static constexpr const auto& CycleTime = 0.0955s;
    static constexpr int DamageDefault = 36;
    static constexpr int DamageZB = 42;
    static constexpr int DamageZBS = 79;

    static constexpr float AccuracyDefault = 0.16;

    KnockbackData KnockBack = { 400.0f, 300.0f, 350.0f, 150.0f, 0.6f };

    duration_t m_flTimeWeaponIdle2;
    
    void Spawn()
    {
        Base::Spawn();
    }

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762Nato";
        p->iMaxAmmo1 = MAX_AMMO_762NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = AK47_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_VIOLINGUN;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash8.spr");
        PRECACHE_MODEL("sprites/muzzleflash9.spr");

        Base::Precache();
    }

    BOOL Deploy(void)
    {
        m_flAccuracy = 0.16;
        m_iShotsFired = 0;

        return DefaultDeploy("models/v_violingun.mdl", "models/p_violingun.mdl", ANIM_DRAW, "violingun", UseDecrement() != FALSE, 1.82s, 1.36s);
    }

    void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
    {
        m_bDelayFire = true;
        m_iShotsFired++;
        m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35;

        if (m_flAccuracy > 1.21)
            m_flAccuracy = 1.21;

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

        Vector vecDir = m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, 8192, 2, BULLET_PLAYER_762MM, GetDamage(), 0.98, m_pPlayer->pev, FALSE, m_pPlayer->random_seed);

        int flags = 0;

#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), FALSE, FALSE);

        m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
        m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
        if (!m_iClip && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.04s;
        m_flTimeWeaponIdle2 = 0s;

        if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.6, 0.46, 0.226, 0.06, 6.6, 2.6, 8);
        else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(2.2, 1.2, 0.6, 0.36, 9.0, 6.0, 6);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.88, 0.33, 0.13, 0.023, 5.3, 1.3, 9);
        else
            KickBack(0.96, 0.365, 0.165, 0.0365, 5.65, 1.65, 8);
    }


    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_flTimeWeaponIdle2 < 30.0s)
        {
            m_flTimeWeaponIdle2 += m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.6s;

            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);
        }
        else
        {
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 10.62s;

            SendWeaponAnim(ANIM_IDLE2, UseDecrement() != FALSE);
            m_flTimeWeaponIdle2 = 0s;
        }
    }


    void Holster(int skiplocal)
    {
        CBasePlayerWeapon::Holster();

        StopIdleSound();
    }

    void StopIdleSound()
    {
#ifndef CLIENT_DLL
        int iPlayerIndex = m_pPlayer->entindex();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(iPlayerIndex);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/violingun_draw.wav");
        MESSAGE_END();

        MESSAGE_BEGIN(MSG_ONE, gmsgMPToCL, NULL, m_pPlayer->pev);
        WRITE_BYTE(79);
        WRITE_SHORT(iPlayerIndex);
        WRITE_SHORT(CHAN_AUTO);
        WRITE_STRING("weapons/violingun_Idle2.wav");
        MESSAGE_END();
#endif
    }

};
LINK_ENTITY_TO_CLASS(weapon_violingun, CViolingun)
}