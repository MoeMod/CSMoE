/*
wpn_infinitys.cpp - implement for infinityss, infinitysb, infinitysr
Copyright (C) 2019 Moemod Yanase

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

using namespace WeaponTemplate::Varibles;

struct InfinitySS_WeaponData
{
public:
	static constexpr const auto &DefaultReloadTime = 3s;
	static constexpr const auto &DefaultDeployTime = 1s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/v_infinityss.mdl";
	static constexpr const char *P_Model = "models/p_infinityss.mdl";
	static constexpr const char *W_Model = "models/w_infinityss.mdl";
	static constexpr const char *EventFile = "events/infinityss.sc";
	static constexpr const char *ClassName = "weapon_infinityss";
	static constexpr const char *AnimExtension = "pistol";
	static constexpr int MaxClip = 8;
	static constexpr float ArmorRatioModifier = 1.5f;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT_EMPTY,
		ANIM_RELOAD,
		ANIM_DRAW
	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.2 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.225 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.1 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.12 * (1 - A);
	static constexpr const auto &CycleTime = 0.21s;
	static constexpr int DamageDefault = 40;
	static constexpr int DamageZB = 69;
	static constexpr int DamageZBS = 69;
	static constexpr float AccuracyDefault = 0.9f;
	static constexpr const auto & AccuracyCalc = (0.35 - T) * 0.175;
	static constexpr float AccuracyMin = 0.5;
	static constexpr float AccuracyMax = 0.9;
	static constexpr float RangeModifier = 0.8;
	static constexpr auto BulletType = BULLET_PLAYER_45ACP;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 250, 200, 250, 90, 0.7f };
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };
};

struct InfinitySR_WeaponData : InfinitySS_WeaponData
{
	static constexpr const char *V_Model = "models/v_infinitysr.mdl";
	static constexpr const char *P_Model = "models/p_infinitysr.mdl";
	static constexpr const char *W_Model = "models/w_infinitysr.mdl";
	static constexpr const char *EventFile = "events/infinitysr.sc";
	static constexpr const char *ClassName = "weapon_infinitysr";
	static constexpr const auto &DefaultReloadTime = 2s;
	static constexpr int DamageDefault = 38;
	static constexpr int DamageZB = 62;
	static constexpr int DamageZBS = 62;
	static constexpr int MaxClip = 15;
	static constexpr const auto & SpreadCalcDefault = 0.11 * (1 - A);
	static constexpr const auto & AccuracyCalc = (0.35 - T) * 0.25;
	static constexpr const auto &CycleTime = 0.175s;
};

struct InfinitySB_WeaponData : InfinitySS_WeaponData
{
	static constexpr const char *V_Model = "models/v_infinitysb.mdl";
	static constexpr const char *P_Model = "models/p_infinitysb.mdl";
	static constexpr const char *W_Model = "models/w_infinitysb.mdl";
	static constexpr const char *EventFile = "events/infinitysb.sc";
	static constexpr const char *ClassName = "weapon_infinitysb";
	static constexpr int DamageDefault = 44;
	static constexpr int DamageZB = 79;
	static constexpr int DamageZBS = 79;
	static constexpr const auto & SpreadCalcDefault = 0.125 * (1 - A);
	static constexpr const auto & AccuracyCalc = (0.35 - T) * 0.25;
	static constexpr const auto &CycleTime = 0.225s;
};

template<class Derived>
using WeaponTemplateBase_InfinityS = LinkWeaponTemplate< Derived,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_USP>::template type,
	TPrecacheEvent,
	TDeployDefault,
	TReloadDefault,
	TPrimaryAttackRifle,
	TFirePistol,
	TRecoilPunch,
	TWeaponIdleDefault,
	TGetDamageDefault
>;

class CInfinitySS : public WeaponTemplateBase_InfinityS<CInfinitySS>, public InfinitySS_WeaponData {};
class CInfinitySR : public WeaponTemplateBase_InfinityS<CInfinitySR>, public InfinitySR_WeaponData {};
class CInfinitySB : public WeaponTemplateBase_InfinityS<CInfinitySB>, public InfinitySB_WeaponData {};
LINK_ENTITY_TO_CLASS(weapon_infinityss, CInfinitySS)
LINK_ENTITY_TO_CLASS(weapon_infinitysr, CInfinitySR)
LINK_ENTITY_TO_CLASS(weapon_infinitysb, CInfinitySB)

}
