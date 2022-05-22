/*
wpn_cannonEX.h - Black Dragon CannonEX from CSO
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

class CCannonEX : public LinkWeaponTemplate<CCannonEX,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TGetItemInfoDefault
>
{
public:
	static constexpr const char *ClassName = "weapon_cannonex";
	enum cannonex_e
	{
		CANNONEX_IDLE,
		CANNONEX_IDLEB,
		CANNONEX_DRAW,
		CANNONEX_DRAWB,
		CANNONEX_SHOOTA,
		CANNONEX_SHOOTB,
		CANNONEX_DRAGON_TRANSFORM,
		CANNONEX_DRAGON_RELOAD1,
		CANNONEX_DRAGON_RELOAD2
	};
	static constexpr const char *V_Model = "models/v_cannonex.mdl";
	static constexpr const char *P_Model = "models/p_cannonex.mdl";
	static constexpr const char *W_Model = "models/w_cannonex.mdl";
	static constexpr const char *AnimExtension = "carbine";

	struct ItemInfoData_t : Base::ItemInfoData_t
	{
		static constexpr int iFlags = ITEM_FLAG_EXHAUSTIBLE;
		static constexpr const char *szAmmo1 = "CannonEXAmmo";
		static constexpr int iMaxAmmo1 = 30;
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
	void SecondaryAttack(void) override;
	void ItemPostFrame(void) override;
	int ExtractAmmo(CBasePlayerWeapon *pWeapon) override; // sync Clip -> BpAmmo
	void Holster(int skiplocal = 0) override;
	void Reload(void) override { };
	void WeaponIdle(void) override;
	BOOL Deploy(void) override;
	void StopIdleSound(void);
	void UpdateOnRemove(void);
	
	bool HasSecondaryAttack() override { return true; }
	const char* GetCSModelName() override 
	{
		if (m_iWeaponState != 0)
			return "models/w_cannonexb.mdl";

		return "models/w_cannonex.mdl";
	}
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_cannonex" , 200 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 0 }; }

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "CannonEXAmmo";
		p->iMaxAmmo1 = 30;
		p->pszAmmo2 = NULL;
		p->iMaxAmmo2 = -1;
		p->iMaxClip = 30;
		p->iSlot = 0;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_CANNONEX;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	}

public:
	void CannonEXFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void Explode(void);
	void RemoveDragon(void);
	void ResetState(void);

	void InitializeData(void);
	
	void CreateDragon(void);
	void CheckWeaponState(void);

public:
	unsigned short m_usFireCannonEx;

#ifndef CLIENT_DLL
	CCannonEXDragon* m_pDragon;
#endif

	bool m_bDragonReloading;
	bool m_bDragonTransforming;
	time_point_t m_flTimeCheckState;
	int m_iWeaponState2;
	int m_iDragonFXModelIndex;
	int m_iExplodeModelIndex;
	int m_iExplodeEffectModelIndex;

	float m_flExplodeRadius;
	duration_t m_flIdleTime;
	duration_t m_flCycleTime;
	float m_flRange;
	float m_flAngleCheck1;
	float m_flAngleCheck2;
	float m_flRangeModifier;
	duration_t m_flTransformTime;
	duration_t m_flDragonFireTime;
	duration_t m_flReloadDelay;
	duration_t m_flReloadTime;

	std::string m_strWeaponModel;
	std::string m_strWeaponModelB;

private:
	float GetExplodeDamage(void);
	float GetDamageAmount(void);
	duration_t GetDragonRemovalTime(void);
};
}
