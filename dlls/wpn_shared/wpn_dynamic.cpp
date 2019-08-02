/*
wpn_dynamic.cpp
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

#include <string>

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using namespace WeaponTemplate::Varibles;

struct SerializableWeaponData
{
	ItemInfo ItemInfoData;
	WeaponIdType WeaponId;
	InventorySlotType ItemSlot;
	std::string ClassName;

	int MaxClip;

	std::string V_Model;
	std::string P_Model;
	std::string W_Model;
	std::string EventFile;

	std::string AnimExtension;

	float DefaultReloadTime;
	int ZoomFOV;
	int ZoomFOV1;
	int ZoomFOV2;

	int ANIM_IDLE1;
	int ANIM_RELOAD;
	int ANIM_DRAW;

	decltype(std::declval<float>() + std::declval<float>() * A) SpreadCalcNotOnGround;
	decltype(std::declval<float>() + std::declval<float>() * A) SpreadCalcWalking;
	decltype(std::declval<float>() + std::declval<float>() * A) SpreadCalcDefault;
	decltype(N * N * N / std::declval<float>() + std::declval<float>()) AccuracyCalc;

	float CycleTime;

	int DamageDefault;
	int DamageZB;
	int DamageZBS;

	float AccuracyDefault;
	float AccuracyMax;
	float RangeModifier;
	Bullet BulletType;
	int Penetration;
	KickBackData KickBackWalking;
	KickBackData KickBackNotOnGround;
	KickBackData KickBackDucking;
	KickBackData KickBackDefault;
	KnockbackData KnockBack;
	Vector RecoilPunchAngleDelta;
};

template<class Final> using WeaponTemplateAll = LinkWeaponTemplate<Final,
		TGeneralData,
		TGetItemInfoDefault,
		TPrecacheEvent,

		TDeployDefault,
		TReloadDefault,

		TPrimaryAttackRifle,
		TFireRifle,
		TFirePistol,

		TSecondaryAttackZoom,
		TSecondaryAttackSniperZoom1,
		TSecondaryAttackSniperZoom2,


		TRecoilPunch,
		TRecoilKickBack,

		TWeaponIdleDefault,
		TGetDamageDefault
>;

class CWeaponDynamic : public WeaponTemplateAll<CWeaponDynamic>, public SerializableWeaponData
{
public:
	void PrimaryAttack() override
	{
		//TPrimaryAttackRifle::PrimaryAttack();
	}

	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim)
	{
		//TFirePistol::Fire(flSpread, flCycleTime, fUseAutoAim);
		//TFireRifle::Fire(flSpread, flCycleTime, fUseAutoAim);
	}

	void SecondaryAttack() override
	{
		TSecondaryAttackZoom::SecondaryAttack();
		TSecondaryAttackSniperZoom1::SecondaryAttack();
		TSecondaryAttackSniperZoom2::SecondaryAttack();
	}
};

}
