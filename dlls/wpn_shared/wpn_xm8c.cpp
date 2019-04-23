/*
wpn_xm8c.cpp
Copyright (C) 2018 Moemod Hyakuya

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

class CXM8Carbine : public LinkWeaponTemplate<CXM8Carbine,
	TGeneralData,
	TDoubleModeType,
	TDeployDoubleMode,
	TReloadDefault,
	TSecondaryAttackZoom,
	TWeaponIdleDefault,
	BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
	TFireRifle,
	TRecoilKickBack,
	TPrimaryAttackRifle,
	TPrecacheEvent,
	TGetDamageDefault
>
{
private:
	enum xm8_e
	{
		CARBINE_XM8_IDLE1,
		CARBINE_XM8_RELOAD,
		CARBINE_XM8_DRAW,
		CARBINE_XM8_SHOOT1,
		CARBINE_XM8_SHOOT2,
		CARBINE_XM8_SHOOT3,
		XM8_CHANGE_SHARPSHOOTER,
		SHARPSHOOTER_XM8_IDLE1,
		SHARPSHOOTER_XM8_RELOAD,
		SHARPSHOOTER_XM8_DRAW,
		SHARPSHOOTER_XM8_SHOOT1,
		SHARPSHOOTER_XM8_SHOOT2,
		SHARPSHOOTER_XM8_SHOOT3,
		XM8_CHANGE_CARBINE,
	};

public:
	static constexpr auto MaxClip = 30;
	static constexpr auto DefaultReloadTime = 3.2;
	static constexpr auto DefaultAccuracy = 0.2;
	static constexpr int ZoomFOV = 50;

	static constexpr const char *V_Model = "models/v_xm8.mdl";
	static constexpr const char *P_Model = "models/p_xm8.mdl";
	static constexpr const char *W_Model = "models/w_xm8.mdl";
	static constexpr const char *EventFile = "events/xm8c.sc";
	static constexpr float ArmorRatioModifier = 1.7f;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_xm8c";
	static constexpr const char *AnimExtension = "rifle";

	enum
	{
		ANIM_IDLE1 = CARBINE_XM8_IDLE1,
		ANIM_RELOAD = CARBINE_XM8_RELOAD,
		ANIM_DRAW = CARBINE_XM8_DRAW,
		ANIM_CHANGE = XM8_CHANGE_CARBINE
	};

	static constexpr const auto & SpreadCalcNotOnGround = 0.4 + 0.4 * A;
	static constexpr const auto & SpreadCalcWalking = 0.04 + 0.07 * A;
	static constexpr const auto & SpreadCalcZoomed = 0.0275 * A;
	static constexpr const auto & SpreadCalcDefault = 0.0275 * A;

	static constexpr float CycleTime = 0.0955;
	static constexpr float CycleTimeZoomed = 0.12;

	static constexpr int DamageDefault = 32;
	static constexpr int DamageZB = 48;
	static constexpr int DamageZBS = 48;

	static constexpr const auto & AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.2f;
	static constexpr float AccuracyMax = 1.25f;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	static constexpr int Distance = 8192;

	static constexpr const auto & KickBackWalking = KickBackData{ 1.425f, 0.44999999f, 0.2f, 0.045000002f, 6.25f, 2.5f, (signed int)7.0f };
	static constexpr const auto & KickBackNotOnGround = KickBackData{ 1.825f, 1.2f, 0.40000001f, 0.30000001f, 8.5f, 6.125f, (signed int)5.0f };
	static constexpr const auto & KickBackDucking = KickBackData{ 0.82499999f, 0.32499999f, 0.12f, 0.0275f, 5.125f, 1.5f, (signed int)8.0f };
	static constexpr const auto & KickBackDefault = KickBackData{ 0.85000002f, 0.35499999f, 0.18000001f, 0.035999998f, 5.25f, 1.85f, (signed int)7.0f };

	static constexpr const auto & KnockBack = KnockbackData{ 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	static constexpr const auto DoubleMode_LinkedWeaponClassName = "weapon_xm8s";
	static constexpr bool DoubleMode_SyncAmmo = true;
	static constexpr float DoubleMode_ChangeTime = 6.0f;

public:
	float GetMaxSpeed() override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 235;

		return 200;
	}
};

LINK_ENTITY_TO_CLASS(weapon_xm8c, CXM8Carbine)