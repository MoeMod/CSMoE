/*
wpn_buffm4a1.cpp
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
    class CBuffM4A1 : public LinkWeaponTemplate< CBuffM4A1,
            TGeneralData,
            BuildTGetItemInfoFromCSW<WEAPON_M4A1>::template type,
            TPrecacheEvent,
            TDeployDefault,
            TReloadDefault,
            TGetDamageDefault,
            TSecondaryAttackZoom,
            TPrimaryAttackRifle,
            TFireRifle,
            TWeaponIdleDefault
    >
    {
    public:
    static constexpr const auto &DefaultReloadTime = 2.0s;
    static constexpr int ZoomFOV = 80;
    static constexpr const char *V_Model = "models/v_buffm4.mdl";
    static constexpr const char *P_Model = "models/p_buffm4.mdl";
    static constexpr const char *W_Model = "models/w_buffm4.mdl";
    static constexpr const char *EventFile = "events/buffm4a1.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "weapon_buffm4";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 50;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
	};
    static constexpr float PlayerMaxSpeed = 240;
    static constexpr float ArmorRatioModifier = 1.6f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.04;
    static constexpr const auto &SpreadCalcWalking = 0.025*A + 0.07;
    static constexpr const auto &SpreadCalcDucking = 0.0*A;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;
    static constexpr const auto &CycleTime1 = 0.0875s;
    static constexpr const auto &CycleTime2 = 0.2s;
	duration_t CycleTime = CycleTime1;
    static constexpr int DamageDefault = 35;
    static constexpr int DamageZB = 113;
    static constexpr int DamageZBS = 130;
    static constexpr const auto &AccuracyCalc = (N * N * N / 210.0) + 0.3;
    static constexpr float AccuracyDefault = 0.19;
    static constexpr float AccuracyMax = 1.0;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 2;
    KnockbackData KnockBack = { 550.0f, 800.0f, 450.0f, 350.0f, 0.3f };

	BOOL Deploy() override
	{
		BOOL result = Base::Deploy();
		return result;
	}
    	
	void Precache() override
	{

		PRECACHE_MODEL("sprites/muzzleflash43.spr");
		PRECACHE_MODEL("sprites/muzzleflash44.spr");
		PRECACHE_MODEL("sprites/muzzleflash45.spr");
		return Base::Precache();
	}
    	
	void FireEvent(const Vector& vecDir)
	{
		int flags = 0;
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		bool bmode = m_pPlayer->pev->fov != 90;
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, g_vecZero, g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), bmode, FALSE);
	}
    	
	void PrimaryAttack() override
	{
		if (m_pPlayer->pev->fov != 90)
		{
			CycleTime = CycleTime2;
			Penetration = 10;
		}
		else
		{
			CycleTime = CycleTime1;
			Penetration = 2;
		}
		return Base::PrimaryAttack();
	}
    	
	float GetDamage()
	{
		if (m_pPlayer->pev->fov != 90)
		{
			float flDamage = 50;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 230;
			else if (g_pModRunning->DamageTrack() == DT_ZBS)
				flDamage = 269;
#endif

			return flDamage;
		}
		return Base::GetDamage();
	}
    	
	void Recoil(void)
	{
		if (m_pPlayer->pev->fov != 90)
		{
			if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.1, 0.47, 0.295, 0.135, 5.2, 3.3, 8);
			else
			{
				if (m_pPlayer->pev->velocity.Length2D() > 0)
					KickBack(0.7, 0.425, 0.275, 0.0335, 3.5, 2.5, 8);
				else
				{
					if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
						KickBack(0.5, 0.24, 0.12, 0.01, 3.15, 2.0, 6);
					else
						KickBack(0.595, 0.32, 0.225, 0.0115, 3.25, 2.1, 7);
				}
			}
		}
		else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
			KickBack(1.1, 0.47, 0.27, 0.135, 5.0, 3.1, 5);
		else
		{
			if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.4, 0.2, 0.15, 0.01, 3.1, 2.0, 5);
			else
				KickBack(0.5, 0.3, 0.2, 0.01, 3.2, 2.0, 5);
		}
	}

	KnockbackData GetKnockBackData() override
    {
		if (m_pPlayer->pev->fov != 90)
			return { 450.0f, 350.0f, 400.0f, 200.0f, 0.4f };
		return Base::GetKnockBackData();
	}
};
LINK_ENTITY_TO_CLASS(weapon_buffm4, CBuffM4A1);
}