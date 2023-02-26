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

#define SHOVELEX_MAX_TYPE	9

static const char* SHOVELEX_CLASSNAME[SHOVELEX_MAX_TYPE] =
{
	"z4b_shovelex",
	"z4b_kukri",
	"z4b_classickukri",
	"z4b_knife",
	"z4b_axe",
	"z4b_hawlchicken",
	"z4b_shovel",
	"z4b_knuckle",
	"z4b_bcaxevip",
};

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

	enum shovelex_type
	{
		TYPE_SHOVELEX,
		TYPE_KUKRI,
		TYPE_CLASSICKUKRI,
		TYPE_KNIFE,
		TYPE_AXE,
		TYPE_HAWLCHICKEN,
		TYPE_SHOVEL,
		TYPE_KNUCKLE,
		TYPE_BCAXE_VIP,
	};

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

	int AddToPlayer(CBasePlayer* pPlayer) override;
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

	const char* CLASSNAME[SHOVELEX_MAX_TYPE] =
	{
		"z4b_shovelex",
		"z4b_kukri",
		"z4b_classickukri",
		"z4b_knife",
		"z4b_axe",
		"z4b_hawlchicken",
		"z4b_shovel",
		"z4b_knuckle",
		"z4b_bcaxevip",
	};

	const char* V_CUSTOM_MODEL[SHOVELEX_MAX_TYPE] =
	{
		"models/z4b/shovel_bornbeast/v_shovel_bornbeast.mdl",
		"models/z4b/kukri/v_kukri.mdl",
		"models/z4b/kukri/v_classickukri.mdl",
		"models/z4b/knife/v_knife.mdl",
		"models/z4b/axe/v_axe.mdl",	
		"models/z4b/hawlchicken/v_hawlchicken.mdl",
		"models/z4b/shovel/v_shovel.mdl",
		"models/z4b/knuckle/v_knuckle.mdl",
		"models/z4b/bcaxevip/v_bcaxevip.mdl",
	};

	const char* P_CUSTOM_MODEL[SHOVELEX_MAX_TYPE] =
	{
		"models/z4b/shovel_bornbeast/p_shovel_bornbeast.mdl",
		"models/z4b/kukri/p_kukri.mdl",
		"models/z4b/kukri/p_kukri.mdl",
		"models/z4b/knife/p_knife.mdl",
		"models/z4b/axe/p_axe.mdl",
		"models/z4b/hawlchicken/p_hawlchicken.mdl",
		"models/z4b/shovel/p_shovel.mdl",
		"models/z4b/knuckle/p_knuckle.mdl",
		"models/z4b/bcaxevip/p_bcaxevip.mdl",
	};

	std::vector<int> m_iStoredChange;
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

	duration_t GetDelayPrimaryAttack()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 0.28s;
			break;
		case TYPE_KNIFE:
			time = 0.08s;
			break;
		case TYPE_AXE:
			time = 0.17s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 0.2s;
			break;
		case TYPE_KNUCKLE:
			time = 0.2s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

	duration_t GetNextPrimaryAttack()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 0.53s;
			break;
		case TYPE_KNIFE:
			time = 0.28s;
			break;
		case TYPE_AXE:
			time = 0.7s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 0.55s;
			break;
		case TYPE_KNUCKLE:
			time = 0.75s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

	duration_t GetPrimaryWeaponIdle()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 0.55s;
			break;
		case TYPE_KNIFE:
			time = 0.32s;
			break;
		case TYPE_AXE:
			time = 0.73s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 0.57s;
			break;
		case TYPE_KNUCKLE:
			time = 0.75s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

	duration_t GetDelaySecondaryAttack()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 0.72s;
			break;
		case TYPE_KNIFE:
			time = 0.52s;
			break;
		case TYPE_AXE:
			time = 0.64s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 0.83s;
			break;
		case TYPE_KNUCKLE:
			time = 0.45s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

	duration_t GetNextSecondaryAttack()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 1.3s;
			break;
		case TYPE_KNIFE:
			time = 1.2s;
			break;
		case TYPE_AXE:
			time = 1.4s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 1.2s;
			break;
		case TYPE_KNUCKLE:
			time = 1.1s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}


	duration_t GetSecondaryWeaponIdle()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 1.4s;
			break;
		case TYPE_KNIFE:
			time = 1.3s;
			break;
		case TYPE_AXE:
			time = 1.5s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 1.3s;
			break;
		case TYPE_KNUCKLE:
			time = 1.2s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

	duration_t GetNextRunTime()
	{
		duration_t time;
		switch (m_iWeaponState)
		{
		case TYPE_SHOVELEX:
		case TYPE_KUKRI:
		case TYPE_CLASSICKUKRI:
			time = 0.75s;
			break;
		case TYPE_KNIFE:
		case TYPE_AXE:
			time = 0.8s;
			break;
		case TYPE_HAWLCHICKEN:
		case TYPE_SHOVEL:
			time = 0.81s;
			break;
		default:
			time = 0s;
			break;
		}

		return time;
	}

};

}
