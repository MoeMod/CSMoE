/*
turbulent1.cpp
Copyright (C) 2019 Moemod Haoyuan

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

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CTurbulent1 : public LinkWeaponTemplate< CTurbulent1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_GLOCK18>::template type,
            TPrecacheEvent,
            TReloadDefault,
            TPrimaryAttackRifle,
            TFirePistol,
			TGetDamageDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 4.03s;
    static constexpr const char *V_Model = "models/v_turbulent1.mdl";
    static constexpr const char *P_Model = "models/p_turbulent1.mdl";
    static constexpr const char *W_Model = "models/w_turbulent1.mdl";
    static constexpr const char *EventFile = "events/turbulent1.sc";
    static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
    static constexpr const char *ClassName = "weapon_turbulent1";
    static constexpr const char *AnimExtension = "onehanded";
    static constexpr int MaxClip = 150;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_IDLE1_EMPTY,
		ANIM_SHOOT,
		ANIM_SHOOT_EMPTY,
		ANIM_RELOAD,
		ANIM_RELOAD_EMPTY,
		ANIM_DRAW,
		ANIM_DRAW_EMPTY,
	};
    static constexpr float PlayerMaxSpeed = 250;
    static constexpr float ArmorRatioModifier = 1.0f;
    static constexpr const auto &SpreadCalcNotOnGround = 1.0*(1-A);
    static constexpr const auto &SpreadCalcWalking = 0.097*(1-A);
    static constexpr const auto& SpreadCalcDucking = 0.075*(1-A);
    static constexpr const auto &SpreadCalcDefault = 0.08*(1-A);

	static constexpr const auto& CycleTime = 0.06s;
    static constexpr int DamageDefault = 15;
    static constexpr int DamageZB = 38;
    static constexpr int DamageZBS = 45;

    static constexpr float AccuracyDefault = 0.9;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 250.0f, 150.0f, 200.0f, 100.0f, 0.4f };

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_turbulent1" , 100 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 8, 3, 0 }; }

	time_point_t m_flTimePlaySound;
    time_point_t m_flNextReload;

    int GetItemInfo(ItemInfo* p) override;
	void Precache() override;
	void Spawn() override;
	BOOL Deploy() override;
    int DefaultReload(int iClipSize, int iAnim, duration_t fDelay, int body = 0) override;
	void Reload(void) override;
	void WeaponIdle(void) override;
	void Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
    void ItemPostFrame() override;
};
}