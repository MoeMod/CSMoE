/*
ak47g.cpp
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
    class CAK47G : public LinkWeaponTemplate< CAK47G,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.45s;
    static constexpr const char *V_Model = "models/v_g_ak47.mdl";
    static constexpr const char *P_Model = "models/p_g_ak47.mdl";
    static constexpr const char *W_Model = "models/w_g_ak47.mdl";
    static constexpr const char *EventFile = "events/ak47.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_ak47g";
    static constexpr const char *AnimExtension = "ak47";
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
    static constexpr float PlayerMaxSpeed = 221;
    static constexpr float ArmorRatioModifier = 1.55f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.0275*A;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.0955s;
    static constexpr int DamageDefault = 36;
    static constexpr int DamageZB = 60;
    static constexpr int DamageZBS = 85;
    static constexpr const auto &AccuracyCalc = (N * N * N / 200.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.5, 0.45, 0.225, 0.05, 6.5, 2.5, 7 };
	KickBackData KickBackNotOnGround = { 2.0, 1.0, 0.5, 0.35, 9.0, 6.0, 5 };
	KickBackData KickBackDucking = { 0.9, 0.35, 0.15, 0.025, 5.5, 1.5, 9 };
	KickBackData KickBackDefault = { 1.0, 0.375, 0.175, 0.0375, 5.75, 1.75, 8 };
    		 	
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
        p->iId = m_iId = WEAPON_AK47G;
        p->iFlags = 0;
        p->iWeight = AK47_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }

};
LINK_ENTITY_TO_CLASS(weapon_ak47g, CAK47G)
}