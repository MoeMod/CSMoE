/*
wpn_cannon.h - Black Dragon Cannon from CSO
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

class CAK47_AngelicBeast : public LinkWeaponTemplate<CAK47_AngelicBeast,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_AK47>::template type,
	TPrecacheEvent,
	TDeployDefault,
	TPrimaryAttackRifle,
	TFireRifle,
	TRecoilKickBack,
	TGetDamageDefault
>
{
public:
	static constexpr const char *ClassName = "z4b_ak47_angelicbeast";
	enum ak47_angelicbeast_e
	{
		ANIM_IDLE1,
		ANIM_RUN,
		ANIM_SHOOT1,
		ANIM_HANDSKILL,
		ANIM_KNIFEATTACK,
		ANIM_KNIFEATTACK2,
		ANIM_KNIFEATTACK3,
		ANIM_CHANGEMODE,
		ANIM_RELOAD,
		ANIM_DRAW_R1,
		ANIM_DRAW,
		ANIM_INSEPCT,
		ANIM_RELOAD_R1,
		ANIM_SHOOT_SKILL,
		ANIM_SHOOT2, 
		ANIM_SHOOT3,
		ANIM_IDLEB,
		ANIM_RUNB,
		ANIM_CHANGEMODEB,
	};

	static constexpr const auto& DefaultReloadTime = 1.8s;

	static constexpr const char* V_Model = "models/z4b/v_ak47_angelicbeast.mdl";
	static constexpr const char* P_Model = "models/z4b/pw_ak47_angelicbeast.mdl";
	static constexpr const char* W_Model = "models/z4b/pw_ak47_angelicbeast.mdl";
	static constexpr const char* EventFile = "events/ak47_angelicbeast.sc";
	static constexpr InventorySlotType ItemSlot = PRIMARY_WEAPON_SLOT;
	static constexpr const char* AnimExtension = "ak47";
	static constexpr int MaxClip = 35;
	static constexpr float PlayerMaxSpeed = 235;
	static constexpr const auto& SpreadCalcNotOnGround = 0.4 * A + 0.04;
	static constexpr const auto& SpreadCalcWalking = 0.07 * A + 0.04;
	static constexpr const auto& SpreadCalcDucking = 0.0 * A;
	static constexpr const auto& SpreadCalcDefault = 0.0275 * A;
	static constexpr const auto& CycleTime = 0.09s;
	static constexpr int DamageDefault = 36;
	static constexpr int DamageZB = 57;
	static constexpr int DamageZBS = 57;
	static constexpr const auto& AccuracyCalc = (N * N * N / 200.0) + 0.35;
	static constexpr float AccuracyDefault = 0.2;
	static constexpr float AccuracyMax = 1.25;
	static constexpr float RangeModifier = 0.98;
	static constexpr auto BulletType = BULLET_PLAYER_762MM;
	static constexpr int Penetration = 2;
	KnockbackData KnockBack = { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f };

	KickBackData KickBackWalking = { 0.9f, 0.35f, 0.225f, 0.05f, 5.5f, 2.5f, 8 };
	KickBackData KickBackNotOnGround = { 1.5f, 0.9f, 0.4f, 0.3f, 7.0f, 4.5f, 8 };
	KickBackData KickBackDucking = { 0.55f, 0.3f, 0.12f, 0.023f, 3.5f, 1.15f, 8 };
	KickBackData KickBackDefault = { 0.665f, 0.35f, 0.155f, 0.035f, 4.0f, 1.55f, 7 };


public:
	void Spawn() override;
	void Precache() override;

	int GetItemInfo(ItemInfo* p) override
	{
		p->pszName = STRING(pev->classname);
		p->pszAmmo1 = "762Nato";
		p->iMaxAmmo1 = MAX_AMMO_762NATO;
		p->pszAmmo2 = "ak47angelicbeast";
		p->iMaxAmmo2 = 100;
		p->iMaxClip = 35;
		p->iSlot = 0;
		p->iPosition = 1;
		p->iId = m_iId = WEAPON_Z4B_AK47_ANGELICBEAST;
		p->iFlags = 0;
		p->iWeight = AK47_WEIGHT;

		return 1;
	}

	float GetArmorRatioModifier() override { return 1.55; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_AK47); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 4, 140 }; }
public:

	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override
	{
		if (m_iDefaultAmmo2)
		{
			m_pPlayer->m_rgAmmo[m_iSecondaryAmmoType] = m_iDefaultAmmo2;
			m_iDefaultAmmo2 = 0;
		}
		return CBasePlayerWeapon::ExtractAmmo(pWeapon);
	}

	float GetMaxSpeed(void)
	{
		if (m_iWeaponState > 1)
			return 250;

		return 235;
	}

	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	void Holster(int skiplocal) override
	{
		SetThink(nullptr);
		m_iWeaponState = 0;
		m_flTimeHostler = gpGlobals->time;
		return CBasePlayerWeapon::Holster(skiplocal);
	}

	void WeaponIdle(void) override;
	void Reload(void) override;
	BOOL Deploy(void) override;
	void PrimaryAttack(void) override;
	void SecondaryAttack(void) override;
	void ItemPostFrame() override;

	float GetDamage();
	void DelaySecondaryAttack();
	float GetSecondaryAttackDamage() const;
	float GetRadiusDamage() const;
	void ChangeMode();

private:
	int m_iDefaultAmmo2;
	int m_iKnifeAnim;
	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;
	time_point_t m_flTimeHostler;
	time_point_t m_flTimeNextCharge;

};
}
