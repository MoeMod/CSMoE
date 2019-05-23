/*
wpn_cannon.h - Black Dragon Cannon from CSO
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

class CCannon : public LinkWeaponTemplate<CCannon,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TGetItemInfoDefault,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "weapon_cannon";
	enum cannon_e
	{
		ANIM_IDLE1,
		ANIM_SHOOT,
		ANIM_DRAW,
	};
	static constexpr const char *V_Model = "models/v_cannon.mdl";
	static constexpr const char *P_Model = "models/p_cannon.mdl";
	static constexpr const char *W_Model = "models/w_cannon.mdl";
	static constexpr const char *AnimExtension = "carbine";

	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iFlags = ITEM_FLAG_EXHAUSTIBLE;
		static constexpr const char *szAmmo1 = "CannonAmmo";
		static constexpr int iMaxAmmo1 = 20;
	};

	static constexpr int MaxClip = -1;
	static constexpr float MaxSpeed = 230;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;

	KnockbackData KnockBack = { 1100.f, 500.f, 700.f, 400.f, 0.9f };
	static constexpr float ArmorRatioModifier = 1.5f;

public:
	void Spawn() override;
	void Precache() override;
	void PrimaryAttack() override;
	int ExtractAmmo(CBasePlayerWeapon *pWeapon) override; // sync Clip -> BpAmmo
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_cannon" , 200 }; }
#endif

public:
	void CannonFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

private:
	unsigned short m_usFire;
};
}
