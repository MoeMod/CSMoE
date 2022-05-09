/*
HK23.cpp
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
    class CHK23 : public LinkWeaponTemplate< CHK23,
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
    static constexpr const char *V_Model = "models/v_hk23.mdl";
    static constexpr const char *P_Model = "models/p_hk23.mdl";
    static constexpr const char *W_Model = "models/w_hk23.mdl";
    static constexpr const char *EventFile = "events/hk23.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_hk23";
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
    static constexpr float PlayerMaxSpeed = 225;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.07*A + 0.05;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 31;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 40;
    static constexpr const auto &AccuracyCalc = (N * N * N / 180.0) + 0.4;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1;
    static constexpr float RangeModifier = 0.96;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.0f, 0.4f, 0.3f, 0.06f, 4.0f, 3.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.6f, 0.6f, 0.4f, 0.125f, 5.0f, 3.0f, 8 };
	KickBackData KickBackDucking = { 0.65f, 0.65f, 0.2f, 0.02f, 3.0f, 2.0f, 9 };
	KickBackData KickBackDefault = { 0.7f, 0.28f, 0.23f, 0.02f, 3.3f, 2.3f, 9 };
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_hk23, CHK23)
}