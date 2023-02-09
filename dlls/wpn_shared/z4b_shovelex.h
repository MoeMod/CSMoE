/*
z4b_oppovivo.cpp
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
#ifndef CLIENT_DLL
#include "gamemode/mods.h"
#endif

#include "weapons/WeaponTemplate.hpp"

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CKnifeShovelEX: public LinkWeaponTemplate<CKnifeShovelEX,
	TGeneralData,
	BuildTGetItemInfoFromCSW<WEAPON_KNIFE>::template type
>
{
public:
	static constexpr const char *ClassName = "z4b_shovelex";
	static constexpr const char* V_Model = "models/z4b/v_kukriex.mdl";
	static constexpr const char* P_Model = "models/z4b/p_kukriex.mdl";
	static constexpr const char* W_Model = "models/w_knife.mdl";

	static constexpr int MaxClip = -1;
	static constexpr auto ItemSlot = KNIFE_SLOT;
	static constexpr const char *AnimExtension = "knife";
	KnockbackData KnockBack = { 0.0f, 0.0f, 0.0f, 0.0f, 0.7f };

public:
#define SHOVELEX_MAX_TYPE	5

	enum skullaxe_e
	{
		ANIM_IDLE1 = 0,
		ANIM_NONE,
		ANIM_WSSB,
		ANIM_DRAW,
		ANIM_STAB1,
		ANIM_STAB2,
		ANIM_SLASH1,
		ANIM_SLASH2,
		ANIM_RUN,
		ANIM_SKILL,
	};
	void Precache() override;
	BOOL Deploy() override;

	void ItemPostFrame() override;
	int GetItemInfo(ItemInfo* p) override;
	void Holster(int skiplocal) override;
	void Spawn() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	BOOL CanDrop() override { return false; }
	void WeaponIdle() override;

	void ChangeWeapon();

	const char* V_CUSTOM_MODEL[SHOVELEX_MAX_TYPE] =
	{
		"models/z4b/v_shovelice.mdl",
		"models/z4b/v_kukriex.mdl",
		"models/z4b/v_normalknife.mdl",
		"models/z4b/v_hawlchicken.mdl",
		"models/z4b/v_lightknife.mdl",	
	};

	const char* P_CUSTOM_MODEL[SHOVELEX_MAX_TYPE] =
	{
		"models/z4b/p_shovelice.mdl",
		"models/z4b/p_kukriex.mdl",
		"models/z4b/p_normalknife.mdl",
		"models/z4b/p_hawlchicken.mdl",
		"models/z4b/p_lightknife.mdl",
	};


	duration_t m_flNextRunIdle;
	duration_t m_flNextIdle;

public:
	void ResetRunIdle()
	{
		m_flNextIdle = m_flNextRunIdle = UTIL_WeaponTimeBase();
	}

	void DelayPrimaryAttack();
	void DelaySecondaryAttack();

	float GetPrimaryAttackDamage() const
	{
		float flDamage = 47;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 280 + (g_pGameRules->m_iTotalRoundsPlayed * 50);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	float GetSecondaryAttackDamage() const
	{
		float flDamage = 120;
#ifndef CLIENT_DLL
		if (g_pModRunning->DamageTrack() == DT_ZB)
			flDamage = 1105 + (g_pGameRules->m_iTotalRoundsPlayed * 100);
		else if (g_pModRunning->DamageTrack() == DT_ZBS)
			flDamage *= 5.5f;
#endif
		return flDamage;
	}
	

};

}
