/*
charger7.cpp
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
    class CCharger7 : public LinkWeaponTemplate< CCharger7,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack
    >
    {
    public:

    static constexpr const char *V_Model = "models/v_charger7.mdl";
    static constexpr const char *P_Model = "models/p_charger7.mdl";
    static constexpr const char *W_Model = "models/w_charger7.mdl";
    static constexpr const char *EventFile = "events/charger7.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_charger7";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 200;
	enum
	{
		ANIM_IDLE1,
        ANIM_BIDLE1,
        ANIM_BIDLE2,
		ANIM_SHOOT1,
        ANIM_BSHOOT1,
		ANIM_SHOOT2,
        ANIM_BSHOOT2,
		ANIM_RELOAD,
        ANIM_BRELOAD,
        ANIM_SCYTHESHOOT,
        ANIM_SCYTHERELOAD,
		ANIM_DRAW,
        ANIM_BDRAW,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.04;
    static constexpr const auto &SpreadCalcDefault = 0.0275*A;

	static constexpr const auto& CycleTime = 0.14s;
    static constexpr int DamageDefault = 34;
    static constexpr int DamageZB = 62;
    static constexpr int DamageZBS = 62;
    static constexpr const auto &AccuracyCalc = (N * N * N / 200.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.5f, 0.45f, 0.225f, 0.05f, 6.5f, 2.5f, 7 };
	KickBackData KickBackNotOnGround = { 2.0f, 1.0f, 0.5f, 0.35f, 9.0f, 6.0f, 5 };
	KickBackData KickBackDucking = { 0.9f, 0.35f, 0.15f, 0.025f, 5.5f, 1.5f, 9 };
	KickBackData KickBackDefault = { 1.0f, 0.375f, 0.175f, 0.038f, 5.75f, 1.75f, 8 };

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 240;
		p->pszAmmo2 = "charger7Scythe";
		p->iMaxAmmo2 = 1;
        p->iMaxClip = 120;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_charger7;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

	void Precache(void) override
	{
		PRECACHE_SOUND("weapons/charger7_scytheshoot.wav");
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

        return DefaultDeploy(V_Model, P_Model, m_iWeaponState & WPNSTATE_M4A1_SILENCED ? ANIM_BDRAW : ANIM_DRAW, "m249", UseDecrement() != FALSE, 0.75s, 2.0s);
    }

    void FireEvent(const Vector& vecDir)
    {
        int flags = 0;
#ifdef CLIENT_WEAPONS
        flags = FEV_NOTHOST;
#else
        flags = 0;
#endif
        bool bmode = m_iWeaponState & WPNSTATE_M4A1_SILENCED;
        PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), bmode, FALSE);
    }

    void SecondaryAttack() override
    {
        if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
        {
            m_iWeaponState &= ~WPNSTATE_M4A1_SILENCED;
            SendWeaponAnim(ANIM_SCYTHESHOOT, UseDecrement() != FALSE);
            //create entity

			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 0;
        }
        else
        {      
            SendWeaponAnim(ANIM_SCYTHERELOAD, UseDecrement() != FALSE);
            SetThink(&CCharger7::Task_Charge);
            pev->nextthink = gpGlobals->time + 3.8s;         
        }

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.8s;
        m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 3.8s;
        m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 3.8s;
       
    }

    void Task_Charge()
    {
        m_iWeaponState |= WPNSTATE_M4A1_SILENCED;

		m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = 1;
#ifndef CLIENT_DLL
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, m_pPlayer->pev);
		WRITE_BYTE(CBasePlayer::GetAmmoIndex("charger7Scythe"));
		WRITE_BYTE(1);
		MESSAGE_END();
#endif
        SetThink(nullptr);
    }

    void WeaponIdle(void) override
    {
        ResetEmptySound();
        m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

        if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
            return;

        if (m_iWeaponState & WPNSTATE_M4A1_SILENCED)
            SendWeaponAnim(RANDOM_LONG(0,2) ? ANIM_BIDLE1 : ANIM_BIDLE2, UseDecrement() != FALSE);
        else
            SendWeaponAnim(ANIM_IDLE1, UseDecrement() != FALSE);

        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20s;
    }

    void Holster(int skiplocal) override
    {
        SetThink(nullptr);
        CBasePlayerWeapon::Holster();
    }

    void Reload(void) override
    {
        if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
            return;

        if (DefaultReload(120, m_iWeaponState & WPNSTATE_M4A1_SILENCED ? ANIM_BRELOAD : ANIM_RELOAD, 3.5s))
        {
#ifndef CLIENT_DLL
            m_pPlayer->SetAnimation(PLAYER_RELOAD);
#endif
            m_flAccuracy = 0.2;
            m_iShotsFired = 0;
            m_bDelayFire = false;
            m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 3.5s;
        }
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_charger7, CCharger7)
}