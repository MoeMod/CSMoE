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
	int ExtractAmmo(CBasePlayerWeapon *pWeapon) override; // sync Clip -> BpAmmo
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
}
