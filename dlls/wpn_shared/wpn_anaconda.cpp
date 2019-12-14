/*
wpn_anaconda.cpp @ codename LCSM
Copyright (C) 2019 Moemod Hymei

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

#include <array>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using WeaponTemplate::Varibles::A;
using WeaponTemplate::Varibles::T;

struct CAnaconda_WeaponData
{
public:
	static constexpr const auto &DefaultReloadTime = 2.3s;
	static constexpr const auto &DefaultDeployTime = 0.75s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/v_anaconda.mdl";
	static constexpr const char *P_Model = "models/p_anaconda.mdl";
	static constexpr const char *W_Model = "models/w_anaconda.mdl";
	static constexpr const char *EventFile = "events/anaconda.sc";
	static constexpr const char *ClassName = "weapon_anaconda";
	static constexpr const char *AnimExtension = "pistol";
	static constexpr int MaxClip = 7;
	static constexpr float ArmorRatioModifier = 1.6f;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT_EMPTY,
		ANIM_RELOAD,
		ANIM_DRAW
	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.4 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.25 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.1 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.125 * (1 - A);
	static constexpr const auto &CycleTime = 0.2s;
	static constexpr int DamageDefault = 45;
	static constexpr int DamageZB = 76;
	static constexpr int DamageZBS = 76;
	static constexpr float AccuracyDefault = 0.9f;
	static constexpr const auto & AccuracyCalc = A - (0.4 - T) * 0.3;
	static constexpr float AccuracyMin = 0.5;
	static constexpr float AccuracyMax = 0.9;
	static constexpr float RangeModifier = 0.8;
	static constexpr auto BulletType = BULLET_PLAYER_50AE;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 350, 250, 300, 100, 0.6f };
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
};

class CAnaconda : public LinkWeaponTemplate< CAnaconda,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TReloadDefault,
		TPrimaryAttackRifle,
		TFirePistol,
		TRecoilPunch,
		TWeaponIdleDefault,
		TGetDamageDefault
	>, public CAnaconda_WeaponData {};
LINK_ENTITY_TO_CLASS(weapon_anaconda, CAnaconda)

}
