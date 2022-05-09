/*
k3.cpp
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
    class CK3 : public LinkWeaponTemplate< CK3,
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
    static constexpr const char *V_Model = "models/v_k3.mdl";
    static constexpr const char *P_Model = "models/p_k3.mdl";
    static constexpr const char *W_Model = "models/w_k3.mdl";
    static constexpr const char *EventFile = "events/k3.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_k3";
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
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.09*A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 29;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 40;
    static constexpr const auto &AccuracyCalc = (N * N * N / 175.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.95;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.4, 0.6, 0.45, 0.075, 4.0, 3.0, 8 };
	KickBackData KickBackNotOnGround = { 2.0, 0.7, 0.5, 0.15, 5.0, 3.5, 8 };
	KickBackData KickBackDucking = { 0.8, 0.35, 0.3, 0.03, 3.5, 2.0, 9 };
	KickBackData KickBackDefault = { 1.0, 0.4, 0.45, 0.035, 3.75, 3.0, 9 };
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_k3, CK3)
}