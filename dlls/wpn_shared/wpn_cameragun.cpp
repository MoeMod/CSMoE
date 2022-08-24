/*
cameragun.cpp
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
    class CCameraGun : public LinkWeaponTemplate< CCameraGun,
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
    static constexpr const auto &DefaultReloadTime = 3.9s;
    static constexpr const char *V_Model = "models/v_cameragun.mdl";
    static constexpr const char *P_Model = "models/p_cameragun.mdl";
    static constexpr const char *W_Model = "models/w_cameragun.mdl";
    static constexpr const char *EventFile = "events/cameragun.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_cameragun";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 150;
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
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.031*A + 0.045;
    static constexpr const auto &SpreadCalcWalking = 0.09*A + 0.04;
    static constexpr const auto &SpreadCalcDefault = 0.031*A;

	static constexpr const auto& CycleTime = 0.11s;
    static constexpr int DamageDefault = 22;
    static constexpr int DamageZB = 53;
    static constexpr int DamageZBS = 53;
    static constexpr const auto &AccuracyCalc = (N * N * N / 175.0) + 0.35;
    static constexpr float AccuracyDefault = 0.21;
    static constexpr float AccuracyMax = 0.95;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 450.0f, 300.0f, 350.0f, 150.0f, 0.7f };

	KickBackData KickBackWalking = { 1.5,0.45,0.45,0.08,4.0,3.0,8 };
	KickBackData KickBackNotOnGround = { 2.1,0.75,0.5,0.155,5.0,3.5,8 };
	KickBackData KickBackDucking = { 0.9,0.4,0.3,0.045,3.5,2.0,9 };
	KickBackData KickBackDefault = { 1.2,0.55,0.5,0.045,3.7,2.8,10 };

    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 150;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_CAMERAGUN;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_cameragun, CCameraGun)
}