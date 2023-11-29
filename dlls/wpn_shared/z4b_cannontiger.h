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

class CCannonTiger : public LinkWeaponTemplate<CCannonTiger,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TGetItemInfoDefault,
	TWeaponIdleDefault,
	TDeployDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_cannontiger";
	enum cannontiger_e
	{
		ANIM_IDLE1,
		ANIM_SHOOT,
		ANIM_DRAW,
	};
	static constexpr const char *V_Model = "models/z4b/v_cannontiger.mdl";
	static constexpr const char *P_Model = "models/z4b/p_cannontiger.mdl";
	static constexpr const char *W_Model = "models/z4b/w_cannontiger.mdl";
	static constexpr const char *AnimExtension = "carbine";

	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iId = WEAPON_Z4B_CANNONTIGER;
		static constexpr const char *szAmmo1 = "CannonAmmo";
		static constexpr int iMaxAmmo1 = 20;
	};

	static constexpr int MaxClip = -1;
	static constexpr float PlayerMaxSpeed = 230;
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;

	KnockbackData KnockBack = { 1100.f, 500.f, 700.f, 400.f, 0.9f };
	static constexpr float ArmorRatioModifier = 1.5f;

public:
	void Spawn() override;
	void Precache() override;
	void PrimaryAttack() override;

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "CannonAmmo";
		p->iMaxAmmo1 = -1;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 20;
		p->iSlot = 0;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_CANNONTIGER;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	}

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_cannon" , 200 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }
	WeaponBotStrategy GetBotStrategy() const;

public:
	void CannonTigerFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

private:
	unsigned short m_usFire;
};
}
