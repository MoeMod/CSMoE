/*
mk48.cpp
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
    class CMK48 : public LinkWeaponTemplate< CMK48,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TPrimaryAttackRifle,
            TSecondaryAttackZoom,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:

    static constexpr int ZoomFOV = 65;
    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_mk48.mdl";
    static constexpr const char *P_Model = "models/p_mk48.mdl";
    static constexpr const char *W_Model = "models/w_mk48.mdl";
    static constexpr const char *EventFile = "events/mk48.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_mk48";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 120;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.6f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.05*A + 0.05;
    static constexpr const auto &SpreadCalcWalking = 0.042*A + 0.03;
    static constexpr const auto &SpreadCalcDefault = 0.03*A + 0.025;

	static constexpr const auto& CycleTime = 0.14s;
    static constexpr int DamageDefault = 38;
    static constexpr int DamageZB = 40;
    static constexpr int DamageZBS = 48;
    static constexpr const auto &AccuracyCalc = (N * N * N / 195.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.8;
    static constexpr float RangeModifier = 0.94;
    static constexpr auto BulletType = BULLET_PLAYER_762MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 370.0f, 270.0f, 320.0f, 120.0f, 0.6f };

	KickBackData KickBackWalking = { 1.4f, 0.3f, 0.25f, 0.04f, 3.6f, 2.6f, 7 };
	KickBackData KickBackNotOnGround = { 1.6f, 0.6f, 0.4f, 0.125f, 5.0f, 3.8f, 8 };
	KickBackData KickBackDucking = { 0.7f, 0.16f, 0.15f, 0.01f, 2.7f, 1.7f, 10 };
	KickBackData KickBackDefault = { 1.0f, 0.25f, 0.17f, 0.02f, 3.3f, 2.5f, 8 };

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "762NatoBox";
        p->iMaxAmmo1 = 240;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 120;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_M249;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_mk48, CMK48)
}