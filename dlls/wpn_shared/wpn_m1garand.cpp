/*
m1garand.cpp
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
    class CM1Garand : public LinkWeaponTemplate< CM1Garand,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_m1garand.mdl";
    static constexpr const char *P_Model = "models/p_m1garand.mdl";
    static constexpr const char *W_Model = "models/w_m1garand.mdl";
    static constexpr const char *EventFile = "events/m1garand.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m1garand";
    static constexpr const char *AnimExtension = "mp5";
    static constexpr int MaxClip = 8;
	enum
	{
		ANIM_IDLEA = 0,
        ANIM_IDLEA_EMPTY,
        ANIM_IDLEB,
        ANIM_IDLEB_EMPTY,
        ANIM_DRAWA,
        ANIM_DRAWA_EMPTY,
        ANIM_CHANGE1,
        ANIM_CHANGE1_EMPTY,
        ANIM_CHANGE2,
        ANIM_CHANGE2_EMPTY,
        ANIM_RELOAD,
		ANIM_SHOOTA,
		ANIM_SHOOTA_EMPTY,
        ANIM_SHOOTB,
        ANIM_SHOOTB_EMPTY,
	};

    static constexpr float PlayerMaxSpeed = 210;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.4;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.04;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.32s;
    static constexpr int DamageDefault = 63;
    static constexpr int DamageZB = 72;
    static constexpr int DamageZBS = 78;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.3;
    static constexpr float AccuracyDefault = 0.0;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.95;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.5f };

	KickBackData KickBackWalking = { 5, 1, 1.25, 0.035, 5, 6.8, 1 };
	KickBackData KickBackNotOnGround = { 1.15, 0.65, 0.35, 0.035, 4.75, 3.75, 4 };
	KickBackData KickBackDucking = { 3, 0.86, 0.5, 0.01, 3, 3.45 };
	KickBackData KickBackDefault = { 6, 1.5, 0.5, 0.25, 6, 7, 1 };
    		 	
    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762Nato";
        p->iMaxAmmo1 = MAX_AMMO_762NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 8;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_M1GARAND;
        p->iFlags = 0;
        p->iWeight = SCOUT_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

    BOOL Deploy(void)
    {
        m_flAccuracy = 0.2;
        m_iShotsFired = 0;
        m_iWeaponState = 0;

        return DefaultDeploy(V_Model, P_Model, m_iClip ? ANIM_DRAWA : ANIM_DRAWA_EMPTY, "mp5", UseDecrement() != FALSE);
    }

    void PrimaryAttack()
    {
        if (m_pPlayer->pev->fov == 90)
        {
            Fire((0.02) * m_flAccuracy, 0.32s, FALSE);
        }
        else
            Base::PrimaryAttack();
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif

        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), m_iClip, m_pPlayer->pev->fov == 90);
    }

    void SecondaryAttack()
    {
        if (!m_iWeaponState)
        {
            pev->iuser1 = 1;
            SendWeaponAnim(m_iClip ? ANIM_CHANGE1 : ANIM_CHANGE1_EMPTY, UseDecrement() != FALSE);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
        }
        else
        {
            if (m_pPlayer->pev->fov != 90)
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;

            m_iWeaponState = 0;
            SendWeaponAnim(m_iClip ? ANIM_CHANGE2 : ANIM_CHANGE2_EMPTY, UseDecrement() != FALSE);

            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2s;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.2s;
        }
    }

    void ItemPostFrame() override
    {
        if (m_flNextSecondaryAttack <= 0.0s)
        {
            if (pev->iuser1)
            {
                pev->iuser1 = 0;
                m_iWeaponState = 1;

                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 70;
            }
        }
        return CBasePlayerWeapon::ItemPostFrame();
    }

    void Reload(void)
    {
        if (m_pPlayer->ammo_762nato <= 0)
            return;

        if (m_iClip != 0)
            return;

        if (DefaultReload(8, ANIM_RELOAD, 2.2s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            if (m_pPlayer->pev->fov != 90)
                m_pPlayer->pev->fov = m_pPlayer->m_iFOV = 90;

            m_iWeaponState = 0;
            m_flAccuracy = 0.0;
            m_iShotsFired = 0;
            m_bDelayFire = false;
        }
    }

    void WeaponIdle(void)
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
        if(m_iWeaponState)
            SendWeaponAnim(m_iClip ? ANIM_IDLEB : ANIM_IDLEB_EMPTY, UseDecrement() != FALSE);
        else
            SendWeaponAnim(m_iClip ? ANIM_IDLEA : ANIM_IDLEA_EMPTY, UseDecrement() != FALSE);

    }


};
LINK_ENTITY_TO_CLASS(weapon_m1garand, CM1Garand)
}