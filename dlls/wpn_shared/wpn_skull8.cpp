/*
skull8.cpp
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
#include "weapons/knifeattack.h"
#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif
    class CSKULL8 : public LinkWeaponTemplate< CSKULL8,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M249>::template type,
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
    static constexpr const auto &DefaultReloadTime = 4.4s;
    static constexpr const char *V_Model = "models/v_skull8.mdl";
    static constexpr const char *P_Model = "models/p_skull8.mdl";
    static constexpr const char *W_Model = "models/w_skull8.mdl";
    static constexpr const char *EventFile = "events/skull8.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_skull8";
    static constexpr const char *AnimExtension = "skull8";
    static constexpr int MaxClip = 120;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_RELOAD,
		ANIM_DRAW,
        ANIM_SHOOT3,
        ANIM_SHOOT4,
	};
    static constexpr float PlayerMaxSpeed = 230;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.045;
    static constexpr const auto &SpreadCalcWalking = 0.095 * A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.12s;
    static constexpr int DamageDefault = 33;
    static constexpr int DamageZB = 72;
    static constexpr int DamageZBS = 72;
    static constexpr const auto &AccuracyCalc = (N * N * N / 170.0) + 0.35;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.95;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 6;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.2f, 0.4f, 0.27f, 0.05f, 4.0f, 3.0f, 8 };
	KickBackData KickBackNotOnGround = { 1.8f, 0.65f, 0.45f, 0.3f, 5.0f, 5.0f, 5 };
	KickBackData KickBackDucking = { 0.75f, 0.17f, 0.17f, 0.018f, 3.5f, 2.3f, 9 };
	KickBackData KickBackDefault = { 1.0f, 0.25f, 0.21f, 0.021f, 3.4f, 2.7f, 7 };

    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556az";
        p->iMaxAmmo1 = 240;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = 120;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_SKULL8;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556az" , 200 }; }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }


    float GetSecondaryAttackDamage() const
    {
        float flDamage = 100;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1500;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 600;
#endif
        return flDamage;
    }

    void DelaySecondaryAttack()
    {
#ifndef CLIENT_DLL
        switch (KnifeAttack5(GetSecondaryAttackDamage(), 76, 170, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
        {
        case HIT_NONE:
        {
            break;
        }
        case HIT_PLAYER:
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.9s;
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_hit.wav", VOL_NORM, ATTN_NORM, 0, 94);
            m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
            break;
        }
        case HIT_WALL:
        {
            m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.9s;
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/skullaxe_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
            m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.5;
            break;
        }
        }
#endif
    }

    void SecondaryAttack(void)
    {
#ifndef CLIENT_DLL
        m_pPlayer->SetAnimation(PLAYER_ATTACK2);
#endif
        m_iSwing++;
        if (m_iSwing & 1)
            SendWeaponAnim(ANIM_SHOOT3, UseDecrement() != FALSE);
        else
            SendWeaponAnim(ANIM_SHOOT4, UseDecrement() != FALSE);

        SetThink(&CSKULL8::DelaySecondaryAttack);
        pev->nextthink = gpGlobals->time + 0.9s;

        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_skull8, CSKULL8)
}