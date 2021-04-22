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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

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
	enum ak47_e
	{
		AK47_IDLE1,
		AK47_RELOAD,
		AK47_DRAW,
		AK47_SHOOT1,
		AK47_SHOOT2,
		AK47_SHOOT3
	};
public:
	static constexpr const auto &DefaultReloadTime = 2.45s;
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
	static constexpr const auto &CycleTime = 0.0955s;
	static constexpr int DamageDefault = 97;
	static constexpr const auto & AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.2;
	static constexpr float AccuracyMax = 1;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	KickBackData KickBackWalking = { 0.4125f, 0.15f, 0.075f, 0.015f, 2.5f, 0.5f, 3 };
	KickBackData KickBackNotOnGround = { 1, 0.5f, 0.25f, 0.175f, 4.5f, 3, 1 };
	KickBackData KickBackDucking = { 0.3f, 0.15f, 0.05f, 0.005f, 2.5f, 0.5f, 4 };
	KickBackData KickBackDefault = { 0.4f, 0.15f, 0.0625f, 0.015f, 2.5f, 0.5f, 3 };
	KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };
};
LINK_ENTITY_TO_CLASS(weapon_ak47l, CAK47_Long)

}
