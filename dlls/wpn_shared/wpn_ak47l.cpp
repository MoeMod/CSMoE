/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "weapons/WeaponTemplate.hpp"

enum ak47_e
{
	AK47_IDLE1,
	AK47_RELOAD,
	AK47_DRAW,
	AK47_SHOOT1,
	AK47_SHOOT2,
	AK47_SHOOT3
};

class CAK47_Long : public LinkWeaponTemplate<CAK47_Long,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TPrecacheEvent,
	TDeployDefault,
	TReloadDefault,
	TSecondaryAttackZoom,
	TPrimaryAttackRifle,
	TFireRifle,
	TRecoilKickBack,
	TWeaponIdleDefault,
	TGetDamageDefault
>
{
public:
	static constexpr auto DefaultReloadTime = 2.45;
	static constexpr int ZoomFOV = 55;
	static constexpr const char *V_Model = "models/v_ak47_long.mdl";
	static constexpr const char *P_Model = "models/p_ak47_long.mdl";
	static constexpr const char *W_Model = "models/w_ak47_long.mdl";
	static constexpr const char *EventFile = "events/ak47l.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_ak47l";
	static constexpr const char *AnimExtension = "ak47";
	static constexpr int MaxClip = 60;
	static constexpr auto   ANIM_IDLE1 = AK47_IDLE1,
		ANIM_RELOAD = AK47_RELOAD,
		ANIM_DRAW = AK47_DRAW;
	static constexpr const auto & SpreadCalcNotOnGround = 0.04 + 0.4 * A;
	static constexpr const auto & SpreadCalcWalking = 0.03 + 0.02 * A;
	static constexpr const auto & SpreadCalcDefault = 0.02 * A;
	static constexpr float CycleTime = 0.0955;
	static constexpr int DamageDefault = 97;
	static constexpr const auto & AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyMax = 1;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	static constexpr const auto & KickBackWalking = KickBackData{ 0.4125, 0.15, 0.075, 0.015, 2.5, 0.5, 3 };
	static constexpr const auto & KickBackNotOnGround = KickBackData{ 1, 0.5, 0.25, 0.175, 4.5, 3, 1 };
	static constexpr const auto & KickBackDucking = KickBackData{ 0.3, 0.15, 0.05, 0.005, 2.5, 0.5, 4 };
	static constexpr const auto & KickBackDefault = KickBackData{ 0.4, 0.15, 0.0625, 0.015, 2.5, 0.5, 3 };
	static constexpr const auto & KnockBack = KnockbackData{ 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };
};
LINK_ENTITY_TO_CLASS(weapon_ak47l, CAK47_Long)
