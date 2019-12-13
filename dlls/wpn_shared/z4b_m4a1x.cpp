/*
z4b_m4a1x.cpp
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
    class CZ4B_M4A1X : public LinkWeaponTemplate< CZ4B_M4A1X,
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
    static constexpr const char *V_Model = "models/z4b/v_m4a1x.mdl";
    static constexpr const char *P_Model = "models/z4b/p_m4a1x.mdl";
    static constexpr const char *W_Model = "models/z4b/w_m4a1x.mdl";
    static constexpr const char *EventFile = "events/m4a1x.sc";
    static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
    static constexpr const char *ClassName = "z4b_m4a1x";
    static constexpr const char *AnimExtension = "rifle";
    static constexpr int MaxClip = 35;
	enum
	{
		ANIM_IDLE1 = 0,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3
	};
    static constexpr float PlayerMaxSpeed = 235;
    static constexpr float ArmorRatioModifier = 1.4f;
    static constexpr const auto &SpreadCalcNotOnGround = 0.4*A + 0.035;
    static constexpr const auto &SpreadCalcWalking = 0.025*A + 0.07;
    static constexpr const auto &SpreadCalcDucking = 0.0*A;
    static constexpr const auto &SpreadCalcDefault = 0.02*A;
    static constexpr const auto &CycleTime1 = 0.09s;
    static constexpr const auto &CycleTime2 = 0.2s;
	duration_t CycleTime = CycleTime1;
    static constexpr int DamageDefault = 31;
    static constexpr int DamageZB = 56;
    static constexpr int DamageZBS = 65;
    static constexpr const auto &AccuracyCalc = (N * N * N / 210.0) + 0.3;
    static constexpr float AccuracyDefault = 0.19;
    static constexpr float AccuracyMax = 1.25;
    static constexpr float RangeModifier = 0.97;
    static constexpr auto BulletType = BULLET_PLAYER_556MM;
    /*static constexpr*/ int Penetration = 1;
    KnockbackData KnockBack = { 550.0f, 800.0f, 450.0f, 350.0f, 0.3f };

	BOOL Deploy() override
	{
		BOOL result = Base::Deploy();
		return result;
	}
    	
	void Precache() override
	{
		PRECACHE_MODEL("sprites/lgtning.spr");
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
		PLAYBACK_EVENT_FULL(flags, m_pPlayer->edict(), m_usFire, 0, (float*)&g_vecZero, (float*)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(m_pPlayer->pev->punchangle.x * 100), static_cast<int>(m_pPlayer->pev->punchangle.y * 100), bmode, FALSE);
	}
    	
	void PrimaryAttack() override
	{
		if (m_pPlayer->pev->fov != 90)
		{
			CycleTime = CycleTime2;
			Penetration = 9;
		}
		else
		{
			CycleTime = CycleTime1;
			Penetration = 1;
		}
		return Base::PrimaryAttack();
	}
    	
	float GetDamage()
	{
		if (m_pPlayer->pev->fov != 90)
		{
			float flDamage = 33;
#ifndef CLIENT_DLL
			if (g_pModRunning->DamageTrack() == DT_ZB)
				flDamage = 189;
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
			if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.6f, 0.25f, 0.4f, 0.03f, 3.5f, 2.3f, 7);
			else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.1f, 0.27f, 0.47f, 0.135f, 5.0f, 3.1f, 7);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.4f, 0.15f, 0.2f, 0.001f, 3.1f, 2.0f, 7);
			else
				KickBack(0.5f, 0.2f, 0.3f, 0.001f, 3.2f, 2.0f, 7);
		}
		else
		{
			if (m_pPlayer->pev->velocity.Length2D() > 0)
				KickBack(0.7f, 0.275f, 0.425f, 0.0335f, 3.5f, 2.5f, 8);
			else if (!FBitSet(m_pPlayer->pev->flags, FL_ONGROUND))
				KickBack(1.5f, 0.9f, 0.4f, 0.3f, 7.0f, 4.5f, 8);
			else if (FBitSet(m_pPlayer->pev->flags, FL_DUCKING))
				KickBack(0.5f, 0.12f, 0.24f, 0.001f, 3.15f, 2.1f, 6);
			else
				KickBack(0.595f, 0.225f, 0.32f, 0.0115f, 3.25f, 2.1f, 7);
		}
	}

	KnockbackData GetKnockBackData() override
    {
		if (m_pPlayer->pev->fov != 90)
			return { 450.0f, 350.0f, 400.0f, 200.0f, 0.4f };
		return Base::GetKnockBackData();
	}
};
LINK_ENTITY_TO_CLASS(z4b_m4a1x, CZ4B_M4A1X)
}