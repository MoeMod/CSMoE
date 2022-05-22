/*
m60.cpp
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
    class CM60 : public LinkWeaponTemplate< CM60,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
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
    static constexpr const auto &DefaultReloadTime = 3.8s;
    static constexpr const char *V_Model = "models/v_m60.mdl";
    static constexpr const char *P_Model = "models/p_m60.mdl";
    static constexpr const char *W_Model = "models/w_m60.mdl";
    static constexpr const char *EventFile = "events/m60.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m60";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 195;
    static constexpr const auto &SpreadCalcNotOnGround = 0.2*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.03*A + 0.04;
    static constexpr const auto& SpreadCalcDucking = 0.02*A + 0.01;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.115s;
    static constexpr int DamageDefault = 36;
    static constexpr int DamageZB = 43;
    static constexpr int DamageZBS = 43;
    static constexpr const auto &AccuracyCalc = (N * N * N / 190.0) + 0.3;
    static constexpr float AccuracyDefault = 0.25;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 6;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1f, 0.3f, 0.2f, 0.06f, 4.0f, 3.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.5f, 0.55f, 0.3f, 0.3f, 5.0f, 5.0f, 5 };
	KickBackData KickBackDucking = { 0.75f, 0.1f, 0.1f, 0.018f, 3.5f, 1.8f, 9 };
	KickBackData KickBackDefault = { 0.9f, 0.25f, 0.2f, 0.02f, 3.2f, 2.5f, 7 };

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 200;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_M60;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_m60, CM60)
}