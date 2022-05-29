/*
m249ep.cpp
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
    class CM249EP : public LinkWeaponTemplate< CM249EP,
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
    static constexpr const auto &DefaultReloadTime = 4.7s;
    static constexpr const char *V_Model = "models/v_m249ep.mdl";
    static constexpr const char *P_Model = "models/p_m249ep.mdl";
    static constexpr const char *W_Model = "models/w_m249ep.mdl";
    static constexpr const char *EventFile = "events/m249.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_m249ep";
    static constexpr const char *AnimExtension = "m249ep";
    static constexpr int MaxClip = 100;
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
    static constexpr float PlayerMaxSpeed = 220;
    static constexpr float ArmorRatioModifier = 1.5f;
	static constexpr float PrimaryAttackWalkingMiniumSpeed = 140;
    static constexpr const auto &SpreadCalcNotOnGround = 0.5*A + 0.045;
    static constexpr const auto &SpreadCalcWalking = 0.095*A + 0.045;
    static constexpr const auto &SpreadCalcDefault = 0.03*A;

	static constexpr const auto& CycleTime = 0.1s;
    static constexpr int DamageDefault = 32;
    static constexpr int DamageZB = 38;
    static constexpr int DamageZBS = 38;
    static constexpr const auto &AccuracyCalc = (N * N * N / 175.0) + 0.4;
    static constexpr float AccuracyDefault = 0.2;
    static constexpr float AccuracyMax = 0.9;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f };

	KickBackData KickBackWalking = { 1.1, 0.5, 0.3, 0.06, 4.0, 3.0, 8 };
	KickBackData KickBackNotOnGround = { 1.8, 0.65, 0.45, 0.125, 5.0, 3.5, 8 };
	KickBackData KickBackDucking = { 0.75, 0.325, 0.25, 0.025, 3.5, 2.5, 9 };
	KickBackData KickBackDefault = { 0.8, 0.35, 0.3, 0.03, 3.75, 3.0, 9 };

    void Precache(void)
    {
        Base::Precache();
        PRECACHE_SOUND("weapons/m249ep_hit1.wav");
        PRECACHE_SOUND("weapons/m249ep_hit2.wav");
        PRECACHE_SOUND("weapons/m249ep_shoot3.wav");
        PRECACHE_SOUND("weapons/m249ep_shoot4.wav");
    }


    int GetItemInfo(ItemInfo* p) override
    {
        p->pszName = STRING(pev->classname);
        p->pszAmmo1 = "556NatoBox";
        p->iMaxAmmo1 = MAX_AMMO_556NATOBOX;
        p->pszAmmo2 = NULL;
        p->iMaxAmmo2 = -1;
        p->iMaxClip = M249_MAX_CLIP;
        p->iSlot = 0;
        p->iPosition = 4;
        p->iId = m_iId = WEAPON_M249EP;
        p->iFlags = 0;
        p->iWeight = M249_WEIGHT;
        return 1;
    }

    bool HasSecondaryAttack() override { return true; }
#ifndef CLIENT_DLL
    WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M249); }
#endif
    float GetArmorRatioModifier() override { return 1.5; }
    WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 195 }; }


    float GetSecondaryAttackDamage() const
    {
        float flDamage = 50;
#ifndef CLIENT_DLL
        if (g_pModRunning->DamageTrack() == DT_ZB)
            flDamage = 1150;
        else if (g_pModRunning->DamageTrack() == DT_ZBS)
            flDamage = 600;
#endif
        return flDamage;
    }

    void DelaySecondaryAttack()
    {
        SendWeaponAnim(ANIM_SHOOT4, UseDecrement() != FALSE);
#ifndef CLIENT_DLL
        switch (KnifeAttack5(GetSecondaryAttackDamage(), 48, 90, DMG_NEVERGIB | DMG_BULLET, this->pev, m_pPlayer))
        {
        case HIT_NONE:
        {
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m249ep_shoot4.wav", VOL_NORM, ATTN_NORM, 0, 94);
            break;
        }
        case HIT_PLAYER:
        {
            
            SendWeaponAnim(ANIM_SHOOT3, UseDecrement() != FALSE);
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m249ep_shoot3.wav", VOL_NORM, ATTN_NORM, 0, 94);
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/m249ep_hit2.wav", VOL_NORM, ATTN_NORM, 0, 94);
            m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
            break;
        }
        case HIT_WALL:
        {
            SendWeaponAnim(ANIM_SHOOT3, UseDecrement() != FALSE);
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/m249ep_shoot3.wav", VOL_NORM, ATTN_NORM, 0, 94);
            EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/m249ep_hit1.wav", VOL_NORM, ATTN_NORM, 0, 94);
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
        DelaySecondaryAttack();
        m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.7s;
        m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2s;
    }
    		 	
};
LINK_ENTITY_TO_CLASS(weapon_m249ep, CM249EP)
}