/*
wpn_mp7a1c.cpp
Copyright (C) 2019 Moemod Hyakuya

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

class CMP7A1_Carbine : public LinkWeaponTemplate< CMP7A1_Carbine,
	TGeneralData,
	TDoubleModeType,
	TDeployDoubleMode,
	TReloadDefault,
	TSecondaryAttackZoom,
	TWeaponIdleDefault,
	BuildTGetItemInfoFromCSW<WEAPON_P90>::template type,
	TFireRifle,
	TRecoilKickBack,
	TPrimaryAttackRifle,
	TPrecacheEvent,
	TGetDamageDefault
>
{
private:
	enum mp7a1_e
	{
		MP7A1_PISTOL_IDLE,
		MP7A1_PISTOL_RELOAD,
		MP7A1_PISTOL_DRAW,
		MP7A1_PISTOL_SHOOT1,
		MP7A1_PISTOL_SHOOT2,
		MP7A1_CHANGE_CARBINE,
		MP7A1_CARBINE_IDLE1,
		MP7A1_CARBINE_RELOAD,
		MP7A1_CARBINE_DRAW,
		MP7A1_CARBINE_SHOOT1,
		MP7A1_CARBINE_SHOOT2,
		MP7A1_CHANGE_PISTOL,
	};

public:
	static constexpr auto MaxClip = 20;
	static constexpr auto DefaultReloadTime = 3.6;
	static constexpr auto DefaultAccuracy = 0;
	static constexpr int ZoomFOV = 50;

	static constexpr const char *V_Model = "models/v_mp7a1.mdl";
	static constexpr const char *P_Model = "models/p_mp7a1.mdl";
	static constexpr const char *W_Model = "models/w_mp7a1.mdl";
	static constexpr const char *EventFile = "events/mp7a1c.sc";
	static constexpr float ArmorRatioModifier = 1.0f;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_mp7a1c";
	static constexpr const char *AnimExtension = "pistol";

	static constexpr const auto & KnockBack = KnockbackData{ 250.0f, 200.0f, 250.0f, 90.0f, 0.7f };

	enum
	{
		ANIM_IDLE1 = MP7A1_CARBINE_IDLE1,
		ANIM_RELOAD = MP7A1_CARBINE_RELOAD,
		ANIM_DRAW = MP7A1_CARBINE_DRAW,
		ANIM_CHANGE = MP7A1_CHANGE_CARBINE
	};

	// change ammo to 46mm
	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr const char *szAmmo1 = "46MM";
		static constexpr int iMaxAmmo1 = MAX_AMMO_46MM;
	};
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_46mm" , AMMO_46MM_PRICE }; }
#endif

	static constexpr float PrimaryAttackWalkingMiniumSpeed = 170;
	static constexpr const auto & SpreadCalcNotOnGround = 0.3 * A;
	static constexpr const auto & SpreadCalcWalking = 0.1 * A;
	static constexpr const auto & SpreadCalcDefault = 0.025 * A;

	static constexpr float CycleTime = 0.18;
	static constexpr float CycleTimeZoomed = 0.18;

	static constexpr int DamageDefault = 29;
	static constexpr int DamageZB = 52;
	static constexpr int DamageZBS = 52;

	static constexpr const auto & AccuracyCalc = (N * N / 210.0) + 0.3;
	static constexpr float AccuracyMax = 1.f;
	static constexpr float RangeModifier = 0.89;
	static constexpr auto BulletType = BULLET_PLAYER_57MM;
	static constexpr int Penetration = 2;
	static constexpr int Distance = 8192;

	static constexpr const auto & KickBackWalking = KickBackData{ 0.5f, 0.25f, 0.2f, 0.03f, 3, 2.25f, (signed int)10 };
	static constexpr const auto & KickBackNotOnGround = KickBackData{ 0.75f, 0.45f, 0.35f, 0.04f, 4, 3, (signed int)9 };
	static constexpr const auto & KickBackDucking = KickBackData{ 0.2f, 0.1f, 0.1f, 0.01f, 2, 1, (signed int)12 };
	static constexpr const auto & KickBackDefault = KickBackData{ 0.225f, 0.3f, 0.25f, 0.08f, 2.75f, 2.5f, (signed int)11 };


	static constexpr const auto DoubleMode_LinkedWeaponClassName = "weapon_mp7a1p";
	static constexpr bool DoubleMode_SyncAmmo = true;
	static constexpr float DoubleMode_ChangeTime = 6.0f;

public:
	float GetMaxSpeed() override
	{
		if (m_pPlayer->m_iFOV == 90)
			return 230;

		return 250;
	}
};

LINK_ENTITY_TO_CLASS(weapon_mp7a1c, CMP7A1_Carbine)