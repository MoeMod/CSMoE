/*
wpn_anaconda.cpp @ codename LCSM
Copyright (C) 2019 Moemod Hymei

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
#include "weapons/KnifeAttack.h"

#include <array>
#include "wpn_knife.h"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

using namespace WeaponTemplate::Varibles;

struct CDeagleShura_WeaponData
{
public:
	static constexpr const auto &DefaultReloadTime = 2.3s;
	static constexpr const auto &DefaultDeployTime = 0s;
	static constexpr InventorySlotType ItemSlot = PISTOL_SLOT;
	static constexpr const char *V_Model = "models/z4b/v_deagleshura.mdl";
	static constexpr const char *P_Model = "models/z4b/pw_deagleshura.mdl";
	static constexpr const char *W_Model = "models/z4b/pw_deagleshura.mdl";
	static constexpr const char *EventFile = "events/deagleb.sc";
	static constexpr const char *ClassName = "z4b_deagleb";
	static constexpr const char *AnimExtension = "pistol";
	static constexpr int MaxClip = 7;
	static constexpr float ArmorRatioModifier = 1.6f;
	enum
	{
		ANIM_IDLE1,
		ANIM_SHOOT1,
		ANIM_SHOOT2,
		ANIM_SHOOT3,
		ANIM_RELOAD,
		ANIM_DRAW,
		ANIM_STAB,
		ANIM_IDLE2
	};
	static constexpr const auto & SpreadCalcNotOnGround = 1.5 * (1 - A);
	static constexpr const auto & SpreadCalcWalking = 0.25 * (1 - A);
	static constexpr const auto & SpreadCalcDucking = 0.115 * (1 - A);
	static constexpr const auto & SpreadCalcDefault = 0.13 * (1 - A);
	static constexpr const auto &CycleTime = 0.3s;
	static constexpr int DamageDefault = 57;
	static constexpr int DamageZB = 80;
	static constexpr int DamageZBS = 80;
	static constexpr float AccuracyDefault = 0.9f;
	static constexpr const auto & AccuracyCalc = (0.4 - T) * 0.35;
	static constexpr float AccuracyMin = 0.0;
	static constexpr float AccuracyMax = 0.9;
	static constexpr float RangeModifier = 0.87;
	static constexpr auto BulletType = BULLET_PLAYER_50AE;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 400, 300, 300, 200, 0.7f };
	std::array<float, 3> RecoilPunchAngleDelta = { -2, 0, 0 };

	
};

class CDeagleShura : public LinkWeaponTemplate< CDeagleShura,
		TGeneralData,
		BuildTGetItemInfoFromCSW<WEAPON_DEAGLE>::template type,
		TPrecacheEvent,
		TDeployDefault,
		TReloadDefault,
		TPrimaryAttackRifle,
		TFirePistol,
		TRecoilPunch,
		TWeaponIdleDefault,
		TGetDamageDefault
	>, public CDeagleShura_WeaponData
{
public:
	void Precache() override
	{
		PRECACHE_SOUND("weapons/mastercombat_slash.wav");
		PRECACHE_SOUND("weapons/combatknife_hit_1.wav");
		PRECACHE_SOUND("weapons/combatknife_wall.wav");
		return Base::Precache();
	}
	
	void SecondaryAttack() override
	{
		if (m_iSwing)
		{
			m_iSwing = 2;
			return;
		}
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif
		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);

#ifndef CLIENT_DLL
		switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetSecondaryAttackDamage(), 93, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
		case HIT_NONE:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_hit_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			break;
		}
		}
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.1s;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 80;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetDeployAttackDamage() const
	{
		float flDamage = 35;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetDeployComboAttackDamage() const
	{
		float flDamage = 75;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage *= 9.5f;
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}

	BOOL Deploy() override
	{
		//EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/skullaxe_draw.wav", 0.3, 2.4);

		m_fMaxSpeed = 250;
		m_iSwing = 1;
		m_pPlayer->m_bShieldDrawn = false;

		SetThink(&CDeagleShura::DeployAttack);
		pev->nextthink = gpGlobals->time + 0.1s;

		BOOL ret = Base::Deploy();

		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.23s;

		return ret;
	}

	void DeployAttack()
	{
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
		switch (KnifeAttack3(vecSrc, gpGlobals->v_forward, GetDeployAttackDamage(), 75, 90, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
		case HIT_NONE:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_hit_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			break;
		}
		}
#endif

		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;

		SetThink(&CDeagleShura::DeployComboAttackCheck);
		pev->nextthink = gpGlobals->time + 0.3s;

		//return fDidHit;
	}

	void DeployComboAttackCheck()
	{
		if (m_iSwing != 2)
		{
			m_iSwing = 0;
			m_flNextSecondaryAttack = m_flNextPrimaryAttack; // OK
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
			return;
		}
		m_iSwing = 0;
		BOOL fDidHit = FALSE;
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 110;

		TraceResult tr;
		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

#ifndef CLIENT_DLL
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		SendWeaponAnim(ANIM_STAB, UseDecrement() != FALSE);
		
#ifndef CLIENT_DLL
		switch (KnifeAttack(vecSrc, gpGlobals->v_forward, GetDeployComboAttackDamage(), 85, DMG_NEVERGIB | DMG_BULLET, m_pPlayer->pev, m_pPlayer->pev))
		{
		case HIT_NONE:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/mastercombat_slash.wav", VOL_NORM, ATTN_NORM, 0, 94);
			break;
		}
		case HIT_PLAYER:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_hit_1.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_BODYHIT_VOLUME;
			fDidHit = TRUE;
			break;
		}
		case HIT_WALL:
		{
			EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_WEAPON, "weapons/combatknife_wall.wav", VOL_NORM, ATTN_NORM, 0, 94);
			m_pPlayer->m_iWeaponVolume = KNIFE_WALLHIT_VOLUME * 0.1;
			fDidHit = TRUE;
			break;
		}
		}
#endif

		m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.7s;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1s;
		SetThink(nullptr);

		// TODO knockback
		//return fDidHit;
	}

	void Holster(int skiplocal) override
	{
		SetThink(nullptr);
		return Base::Holster();
	}

	bool HasSecondaryAttack() override
	{
		return true;
	}
};
LINK_ENTITY_TO_CLASS(z4b_deagleb, CDeagleShura)

}
