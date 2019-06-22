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

enum m249_e
{
	M249_IDLE1,
	M249_RELOAD,
	M249_DRAW,
	M249_SHOOT1,
	M249_SHOOT2,
	M249_SHOOT3
};

class CMG36 : public LinkWeaponTemplate<CMG36,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
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
	static constexpr auto DefaultReloadTime = 3.9;
	static constexpr int ZoomFOV = 65;
	static constexpr const char *V_Model = "models/v_mg36.mdl";
	static constexpr const char *P_Model = "models/p_mg36.mdl";
	static constexpr const char *W_Model = "models/w_mg36.mdl";
	static constexpr const char *EventFile = "events/mg36.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char *ClassName = "weapon_mg36";
	static constexpr const char *AnimExtension = "m249";
	static constexpr int MaxClip = 100;
	static constexpr float MaxSpeed = 230;
	static constexpr auto   ANIM_IDLE1 = M249_IDLE1,
		ANIM_RELOAD = M249_RELOAD,
		ANIM_DRAW = M249_DRAW;
	static constexpr const auto & SpreadCalcNotOnGround = 0.05 * A + 0.05;
	static constexpr const auto & SpreadCalcWalking = 0.042 * A + 0.02;
	static constexpr const auto & SpreadCalcDefault = 0.22 * A;
	static constexpr float CycleTime = 0.0119;
	static constexpr int DamageDefault = 37;
	static constexpr const auto & AccuracyCalc = (N * N * N / 220.0) + 0.3;
	static constexpr float AccuracyDefault = 0.22;
	static constexpr float AccuracyMax = 1;
	static constexpr float RangeModifier = 0.94;
	static constexpr auto BulletType = BULLET_PLAYER_556NATOBOX;
	static constexpr int Penetration = 1;
	KickBackData KickBackWalking = { 1.4f, 0.3f, 0.065f, 0.04f, 3.5f, 2.0f, 6.0f };
	KickBackData KickBackNotOnGround = { 1.6f, 0.6f, 0.4f, 0.125f, 5.0f, 3.0f, 8.0f };
	KickBackData KickBackDucking = { 0.6f, 0.15f, 0.15f, 0.01f, 2.0f, 0.8f, 15.0f };
	KickBackData KickBackDefault = { 1.1f, 0.2f, 0.04f, 0.22f, 2.5f, 2.0f, 8.0f };
	KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };
};
LINK_ENTITY_TO_CLASS(weapon_mg36, CMG36)

}
