/*
skull6.cpp
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
    class CSKULL6 : public LinkWeaponTemplate< CSKULL6,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_SG552>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TSecondaryAttackSniperZoom2,
            TGetDamageDefault,
            TFireRifle,
			TRecoilKickBack,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 3.64s;
    static constexpr int ZoomFOV1 = 85;
    static constexpr int ZoomFOV2 = 40;
    static constexpr const char *V_Model = "models/v_skull6.mdl";
    static constexpr const char *P_Model = "models/p_skull6.mdl";
    static constexpr const char *W_Model = "models/w_skull6.mdl";
    static constexpr const char *EventFile = "events/skull6.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull6";
    static constexpr const char *AnimExtension = "m249";
    static constexpr int MaxClip = 100;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
	};
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
    static constexpr int DamageDefault = 30;
    static constexpr int DamageZB = 42;
    static constexpr int DamageZBS = 70;
    static constexpr const auto &AccuracyCalc = (N * N * N / 180.0) + 0.3;
    static constexpr float AccuracyDefault = 0.35;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.95;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 6;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556az";
        p->iMaxAmmo1 = 240;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 100;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_SKULL6;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;

        return 1;
    }

    float GetMaxSpeed(void) override
    {
        if (m_pPlayer->m_iFOV == 90)
            return 220;

        return 200;
    }

    void PrimaryAttack() override
    {
        duration_t flCycleTime = m_pPlayer->m_iFOV == 90 ? 0.1275s : 0.157s;

        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            Fire(0.3 * m_flAccuracy + 0.1, flCycleTime, FALSE);
        else if (m_pPlayer->pev->velocity.Length2D() > 195)
            Fire(0.095 * m_flAccuracy + 0.06, flCycleTime, FALSE);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
           Fire(0.06 * m_flAccuracy + 0.01, flCycleTime, FALSE);
        else
        {
            if(m_pPlayer->m_iFOV == 90)
                Fire(0.05 * m_flAccuracy, flCycleTime, FALSE);
            else
                Fire(0.01 * m_flAccuracy + 0.01, flCycleTime, FALSE);
        }
            
    }

    void Recoil(void)
    {
        if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
            KickBack(1.775, 0.85, 0.3, 0.3, 5.0, 5.0, 5);
        else if (m_pPlayer->pev->velocity.Length2D() > 0)
            KickBack(1.5, 0.6, 0.18, 0.1, 4.0, 3.0, 7);
        else if (m_pPlayer->pev->fov != 90)
            KickBack(0.8, 0.2, 0.08, 0, 2.0, 0.2, 10);
        else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
            KickBack(0.7, 0.2, 0.15, 0.018, 3.2, 2.0, 9);
        else
            KickBack(0.9, 0.38, 0.2, 0.06, 3.6, 2.6, 7);     
    }

#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556az" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_skull6, CSKULL6)
}