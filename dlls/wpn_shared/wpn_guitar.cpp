/*
guitar.cpp
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
    class CGuitar : public LinkWeaponTemplate< CGuitar,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.3s;
    static constexpr const char *V_Model = "models/v_guitar.mdl";
    static constexpr const char *P_Model = "models/p_guitar.mdl";
    static constexpr const char *W_Model = "models/w_guitar.mdl";
    static constexpr const char *EventFile = "events/guitar.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_guitar";
    static constexpr const char *AnimExtension = "ak47";
    static constexpr int MaxClip = 30;
	enum
	{
		ANIM_IDLE1 = 0,
        ANIM_RELOAD,
        ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
        ANIM_SHOOT3,
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.4f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.035;
    static constexpr const auto &SpreadCalcWalking = 0.07 * A + 0.035;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;

	static constexpr const auto& CycleTime = 0.115s;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 57;
    static constexpr int DamageZBS = 87;
    static constexpr const auto &AccuracyCalc = (N * N * N / 220.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.98;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.0, 0.45, 0.28, 0.045, 3.75, 3.0, 7 };
	KickBackData KickBackNotOnGround = { 1.2, 0.5, 0.23, 0.15, 5.5, 3.5, 6 };
	KickBackData KickBackDucking = { 0.6, 0.3, 0.2, 0.0125, 3.25, 2.0, 7 };
	KickBackData KickBackDefault = { 0.6, 0.3, 0.2, 0.015, 3.2, 2.1, 7 };
    		 	
    int GetItemInfo(ItemInfo* p)
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556Nato";
        p->iMaxAmmo1 = MAX_AMMO_556NATO;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = M4A1_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 1;
        p->iId = m_iId = WEAPON_GUITAR;
        p->iFlags = 0;
        p->iWeight = M4A1_WEIGHT;

        return 1;
    }

    float GetArmorRatioModifier() override { return 1.4; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M4A1); }
#endif
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }

    void Precache()
    {
        PRECACHE_MODEL("sprites/muzzleflash8.spr");
        PRECACHE_MODEL("sprites/muzzleflash9.spr");

        Base::Precache();
    }


};
LINK_ENTITY_TO_CLASS(weapon_guitar, CGuitar)
}