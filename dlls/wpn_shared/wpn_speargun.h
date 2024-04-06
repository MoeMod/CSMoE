/*
wpn_speargun.h - Black Dragon speargun from CSO
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

#pragma once

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSpearGun : public LinkWeaponTemplate<CSpearGun,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TGetItemInfoDefault
>
{
public:
	static constexpr const char *ClassName = "weapon_speargun";
	enum speargun_e
	{
		SPEARGUN_IDLE1,
		SPEARGUN_SHOOT1,
		SPEARGUN_RELOAD,
		SPEARGUN_DRAW,
		SPEARGUN_DRAW_EMPTY,
		SPEARGUN_IDLE_EMPTY
	};
	static constexpr const char *V_Model = "models/v_speargun.mdl";
	static constexpr const char *P_Model = "models/p_speargun.mdl";
	static constexpr const char *W_Model = "models/w_speargun.mdl";
	static constexpr const char *AnimExtension = "carbine";

	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iFlags = ITEM_FLAG_EXHAUSTIBLE;
		static constexpr const char *szAmmo1 = "SpearAmmo";
		static constexpr int iMaxAmmo1 = 30;
	};

	static constexpr int MaxClip = -1;
	static constexpr float PlayerMaxSpeed = 230;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;

	KnockbackData KnockBack = { 1100.f, 500.f, 700.f, 400.f, 0.9f };

	float GetArmorRatioModifier() override { return 1.9; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

public:
	int GetItemInfo(ItemInfo* p) override;
	void Spawn() override;
	void Precache() override;
	void PrimaryAttack() override;
	BOOL Deploy() override;
	void SecondaryAttack(void) override;
	void ItemPostFrame(void) override;
	void Reload(void) override { };
	bool HasSecondaryAttack() override { return true; }
	void WeaponIdle(void) override;
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_spear" , 180 }; }
#endif

private:
	float GetSpread(void);

	unsigned short m_usFireSpearGun;
	int iShellOn;
	time_point_t m_flTimeReloadSpear;
	EHANDLE m_hSpears[2];
};

#define SPEARGUNM_MAX_CLIP	3
class CSpearGunm : public CSpearGun
{
public:
	float GetArmorRatioModifier() override { return 1.9; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

public:
	const char* GetCSModelName() override { return "models/w_speargunm.mdl"; }
	int GetItemInfo(ItemInfo* p) override;
	void Spawn() override;
	void Precache() override;
	void PrimaryAttack() override;
	BOOL Deploy() override;
	void SecondaryAttack(void) override;
	void ItemPostFrame(void) override;
	void Reload(void) override;
	bool HasSecondaryAttack() override { return true; }
	void WeaponIdle(void) override;
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_spearm" , 450 }; }
#endif

private:
	float GetSpread(void);

	unsigned short m_usFireSpearGunm;
	int iShellOn;
	time_point_t m_flTimeReloadSpear;
	EHANDLE m_hSpears[SPEARGUNM_MAX_CLIP];
};

class CSpearGunEX : public CSpearGun
{
public:
	float GetArmorRatioModifier() override { return 1.9; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

public:
	const char* GetCSModelName() override { return "models/w_speargunex.mdl"; }
	int GetItemInfo(ItemInfo* p) override;
	void Spawn() override;
	void Precache() override;
	void PrimaryAttack() override;
	BOOL Deploy() override;
	void SecondaryAttack(void) override;
	void ItemPostFrame(void) override;
	void Reload(void) override { };
	bool HasSecondaryAttack() override { return true; }
	void WeaponIdle(void) override;
	void SpearGunFire();
	void OnAttack2Released();
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_spearex" , 150 }; }
#endif

private:
	static constexpr float AmmoSpeed = 2000.0;
	static constexpr float AccuracyDefault = 0.05;
	static constexpr float AccuracyAdd = 0.01;
	static constexpr float SpreadMul = 0.001;
	static constexpr float SpreadAddJump = 0.1;
	static constexpr float SpreadMulJump = 0.035;
	static constexpr float SpreadAddMove = 0.02;
	static constexpr float SpreadMulMove = 0.006;
	static constexpr float SpreadAddDuck = 0.0075;
	static constexpr float SpreadMulDuck = 0.001;

	static constexpr float Damage = 95.0;
	static constexpr float DamageZombie = 2500.0;
	static constexpr float DamageScenario = 13500.0;
	static constexpr float ExpDamage = 25.0;
	static constexpr float ExpDamageZombie = 1000.0;
	static constexpr float ExpDamageScenario = 2000.0;

	//static constexpr float ExpDamage_MeDownRatio = 0.0;
	//static constexpr float ExpDamage_MeDownRatioZombie = 0.0;
	//static constexpr float ExpDamage_MeDownRatioScenario = 0.0;

	static constexpr float TouchDamage = 5.0;
	static constexpr float TouchDamageZombie = 500.0;
	static constexpr float TouchDamageScenario = 500.0;

	static constexpr const auto& AmmoHookingTime = 0.0s;
	static constexpr const auto& AmmoHookingTimeZombie = 0.5s;
	static constexpr const auto& AmmoHookingTimeScenario = 0.5s;

	static constexpr const auto& ShootTime = 0.42s;
	static constexpr const auto& CycleTime = 1.87s;
	static constexpr const auto& ReloadTime = 1.4s;

	static constexpr float ExpEffectThrowVel = 400.0;
	static constexpr float ExpEffectThrowCount = 30.0;
	static constexpr const auto& ExpEffectTime = 0.3s;

	static constexpr const auto& AmmoLifeTime = 4.0s;
	
	static constexpr float MainKickSpeed = 0.0;
	static constexpr float MainKickSpeedZ = 0.0;
	static constexpr float SubKickSpeed = 0.0;
	static constexpr float SubKickSpeedZ = 0.0;
	static constexpr float MainKickSpeedZombie = 650.0;
	static constexpr float MainKickSpeedZZombie = 50.0;
	static constexpr float SubKickSpeedZombie = 500.0;
	static constexpr float SubKickSpeedZZombie = 125.0;
	static constexpr float MainKickSpeedScenario = 800.0;
	static constexpr float MainKickSpeedZScenario = 75.0;
	static constexpr float SubKickSpeedScenario = 650.0;
	static constexpr float SubKickSpeedZScenario = 150.0;
	
	static constexpr const auto& SecondaryChargingMinTime = 0.6s;
	static constexpr const auto& SecondaryChargingTime = 1.8s;
	static constexpr const auto& ChargingShotLife = 4.0s;
	static constexpr float ChargingShotSize = 10.0;
	static constexpr float ChargingShotSpeed = 1000.0;
	static constexpr float ChargingHitDamage = 100.0;
	static constexpr float ChargingHitDamageZombie = 3000.0;
	static constexpr float ChargingHitDamageScenario = 16500.0;
	
	static constexpr float ChargingExpDamage = 20.0;
	static constexpr float ChargingExpDamageZombie = 2500.0;
	static constexpr float ChargingExpDamageScenario = 5000.0;

	static constexpr const auto& ChargingExpKnockbackDuration = 0.2s;
	static constexpr float ChargingExpKnockbackSide = 1500.0;
	static constexpr float ChargingExpKnockbackUp = 200.0;
	
	static constexpr float FullChargingHitDamage = 110.0;
	static constexpr float FullChargingHitDamageZombie = 5000.0;
	static constexpr float FullChargingHitDamageScenario = 18000.0;
	static constexpr float FullChargingExpDamage = 40.0;
	static constexpr float FullChargingExpDamageZombie = 5000.0;
	static constexpr float FullChargingExpDamageScenario = 6500.0;
	
	static constexpr const auto& FullChargingExpKnockbackDuration = 0.2s;
	static constexpr float FullChargingExpKnockbackSide = 2000.0;
	static constexpr float FullChargingExpKnockbackUp = 200.0;


	float GetSpread(void);

	unsigned short m_usFireSpearGunEX;
	int iShellOn;
	time_point_t m_flTimeReloadSpear;
	time_point_t m_flTimeCheckCharge;
	time_point_t m_flTimeCheckSound;
};
}
