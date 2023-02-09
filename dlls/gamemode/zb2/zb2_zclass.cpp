/*
zb2_zclass.cpp - CSMoE Gameplay server : Zombie Mod 2
Copyright (C) 2019 Moemod Yanase

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

#include "gamemode/zb2/zb2_const.h"
#include "gamemode/zb2/zb2_zclass.h"
#include "gamemode/zb2/zb2_skill.h"
#include "gamemode/mod_zb2.h"

#include "player/player_zombie.h"

#include "zb2_zclass_tank.h"
#include "zb2_zclass_speed.h"
#include "zb2_zclass_deimos2.h"
#include "zb2_zclass_deimos.h"
#include "zb2_zclass_pc.h"
#include "zb2_zclass_heavy.h"
#include "zb2_zclass_heal.h"
#include "zb2_zclass_teleport.h"
#include "zb2_zclass_booster.h"
#include "zb2_zclass_china.h"
#include "zb2_zclass_pass.h"
#include "zb2_zclass_flying.h"
#include "zb2_zclass_nemesis.h"
#include "zb2_zclass_revival.h"
#include "zb2_zclass_stamper.h"
#include "zb2_zclass_resident.h"
#include "zb2_zclass_witch.h"
#include "zb2_zclass_zbs_deimos2.h"
#include "zb2_zclass_aksha.h"
#include "zb2_zclass_boomer.h"
#include "zb2_zclass_spider.h"
#include "zb2_zclass_deathknight.h"
#include "zb2_zclass_meatwall.h"
#include "zb2_zclass_siren.h"
#include <random>

namespace sv {

template<class T>
std::shared_ptr<CBaseZombieClass_ZB2> MakeZombieClass(CBasePlayer *player, ZombieLevel lv)
{
	return std::make_shared<T>(player, lv);
}
#ifdef XASH_DEDICATED
const std::tuple<const char*, unsigned int, std::shared_ptr<CBaseZombieClass_ZB2>(*)(CBasePlayer*, ZombieLevel)> g_FindList[] = {
		{"tank", 100, MakeZombieClass<CZombieClass_Default>},
		{"speed", 100, MakeZombieClass<CZombieClass_Speed> },
		{"heavy", 100, MakeZombieClass<CZombieClass_Heavy> },
		{"pc", 100, MakeZombieClass<CZombieClass_Pc> },
		{"heal", 100, MakeZombieClass<CZombieClass_Heal> },
		{"deimos", 100, MakeZombieClass<CZombieClass_Deimos> },
		{"deimos2", 100, MakeZombieClass<CZombieClass_Deimos2> },
		{"zbs_deimos2", 100, MakeZombieClass<CZombieClass_ZBS_Deimos2> },
		{"teleport", 100, MakeZombieClass<CZombieClass_Teleport> },
		{"booster", 100, MakeZombieClass<CZombieClass_Booster> },
		{"china", 100, MakeZombieClass<CZombieClass_China> },
		{"pass", 100, MakeZombieClass<CZombieClass_Pass> },
		{"flying", 100, MakeZombieClass<CZombieClass_Flying> },
		{"revival", 50, MakeZombieClass<CZombieClass_Revival> },
		{"stamper", 100, MakeZombieClass<CZombieClass_Stamper> },
		{"resident", 100, MakeZombieClass<CZombieClass_Resident> },
		{"witch", 100, MakeZombieClass<CZombieClass_Witch> },
		{"aksha", 100, MakeZombieClass<CZombieClass_Aksha> },
		{"boomer", 100, MakeZombieClass<CZombieClass_Boomer> },
		{"nemesis", 25, MakeZombieClass<CZombieClass_Nemesis>},
		{"spider", 25, MakeZombieClass<CZombieClass_Spider>},
		{"deathknight", 25, MakeZombieClass<CZombieClass_DeathKnight>},
		{"siren", 50, MakeZombieClass<CZombieClass_Siren>},
		{"meatwall", 25, MakeZombieClass<CZombieClass_MeatWall>},
};

#else
const std::tuple<const char*, unsigned int, std::shared_ptr<CBaseZombieClass_ZB2>(*)(CBasePlayer*, ZombieLevel)> g_FindList[] = {
	{"tank", 100, MakeZombieClass<CZombieClass_Default>},
	{"speed", 100, MakeZombieClass<CZombieClass_Speed> },
};
#endif

constexpr auto NUM_ZCLASSES = std::extent<decltype(g_FindList)>::value;

template <size_t ...N>
constexpr std::array<unsigned int, NUM_ZCLASSES> MakeZClassWeightList(std::index_sequence<N...>) {
	return { std::get<1>(g_FindList[N])... };
}
auto ZCLASSES_WEIGHT = MakeZClassWeightList(std::make_index_sequence<NUM_ZCLASSES>{});

const char* CPlayerModStrategy_ZB2::ZombieNameByClass(ZombieClassType i)
{
	return std::get<0>(g_FindList[i]);
}
std::shared_ptr<CBaseZombieClass_ZB2> CPlayerModStrategy_ZB2::ZombieClassFactory(ZombieLevel lv, const char *name)
{
	auto player = m_pPlayer;
	if(name == nullptr)
		return std::get<2>(g_FindList[0])(player, lv);

	if(!stricmp(name, "random"))
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		const std::size_t N = std::discrete_distribution<>(ZCLASSES_WEIGHT.begin(), ZCLASSES_WEIGHT.end())(gen);
		player->m_ZombieClass = std::get<0>(g_FindList[N]);
		return std::get<2>(g_FindList[N])(player, lv);
	}

	for(auto &&pr : g_FindList)
	{
		if(!stricmp(name, std::get<0>(pr)))
			return std::get<2>(pr)(player, lv);
	}

	return std::get<2>(g_FindList[0])(player, lv);
}

void CBaseZombieClass_ZB2::Evolution(ZombieLevel iEvolutionLevel)
{
	m_pPlayer->m_iZombieLevel = iEvolutionLevel;
	if (m_pPlayer->m_pActiveItem)
	{
		m_pPlayer->SwitchWeapon(m_pPlayer->m_pActiveItem);
	}

	//InitHUD();
	ResetMaxSpeed();
}

void CBaseZombieClass_ZB2::InitHUD() const
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB2Msg, nullptr, m_pPlayer->pev);
	WRITE_BYTE(ZB2_MESSAGE_SKILL_INIT);
	MESSAGE_END();
}

void CBaseZombieClass_ZB2::ActivateSkill(ZombieSkillSlot which)
{
	if (!m_bCanUseSkills)
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "Cannot Use Skill Currently.");
		return;
	}
	if (m_pZombieSkill && which == SKILL_SLOT_1 && GetSkillStatus(SKILL_SLOT_2) != SKILL_STATUS_USING)
	{
		m_pZombieSkill->Activate();
	}
	else if (m_pZombieSkill2 && which == SKILL_SLOT_2 && GetSkillStatus(SKILL_SLOT_1) != SKILL_STATUS_USING)
	{
		m_pZombieSkill2->Activate();
	}
}

void CBaseZombieClass_ZB2::Think()
{
	if (!m_bCanUseSkills && m_bForceStopSkills)
	{
		m_bForceStopSkills = false;
	}

	if (m_tBanSkillsTime != invalid_time_point && m_tBanSkillsTime <= gpGlobals->time)
	{
		m_tBanSkillsTime = invalid_time_point;
		m_bForceStopSkills = false;
		m_bCanUseSkills = true;
	}

	if (m_flInSmokeTime != invalid_time_point && m_flInSmokeTime <= gpGlobals->time)
	{
		m_flInSmokeTime = invalid_time_point;
		m_bSmokeReduceDamage = false;
		m_flSmokeReduceDamageRate = 1.0;
	}

	if (m_pZombieSkill)
		m_pZombieSkill->Think();
	if (m_pZombieSkill2)
		m_pZombieSkill2->Think();

	return CZombie_ZB1::Think();
}

void CBaseZombieClass_ZB2::ResetMaxSpeed() const
{
	CZombie_ZB1::ResetMaxSpeed();
	if (m_pZombieSkill)
		m_pZombieSkill->ResetMaxSpeed(); // cans replace result of CZombie_ZB1::ResetMaxSpeed()
	if (m_pZombieSkill2)
		m_pZombieSkill2->ResetMaxSpeed(); // cans replace result of CZombie_ZB1::ResetMaxSpeed()
}

bool CBaseZombieClass_ZB2::ApplyKnockback(CBasePlayer *attacker, const KnockbackData &kbd)
{
	// TODO : knockback adjustment by skill
	return CZombie_ZB1::ApplyKnockback(attacker, kbd);
}

float CBaseZombieClass_ZB2::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const
{
	if (m_pZombieSkill)
		flDamage *= m_pZombieSkill->GetDamageRatio();
	if (m_pZombieSkill2)
		flDamage *= m_pZombieSkill2->GetDamageRatio();

	if (m_bSmokeReduceDamage)
		flDamage *= m_flSmokeReduceDamageRate;

	return CZombie_ZB1::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

ZombieSkillStatus CBaseZombieClass_ZB2::GetSkillStatus(ZombieSkillSlot which) const
{
	if (m_pZombieSkill && which == SKILL_SLOT_1)
	{
		return m_pZombieSkill->GetStatus();
	}
	else if (m_pZombieSkill2 && which == SKILL_SLOT_2)
	{
		return m_pZombieSkill2->GetStatus();
	}

	return SKILL_STATUS_USED;
}

float CBaseZombieClass_ZB2::HealthRecoveryAmount() const
{
	if(GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING || GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING)
		return 0;

	if (m_pPlayer->m_ZombieClass)
	{
		if (!stricmp(m_pPlayer->m_ZombieClass, "revival") || 
			!stricmp(m_pPlayer->m_ZombieClass, "teleport") || 
			!stricmp(m_pPlayer->m_ZombieClass, "spider") || 
			!stricmp(m_pPlayer->m_ZombieClass, "deathknight") ||
			!stricmp(m_pPlayer->m_ZombieClass, "nemesis") ||
			!stricmp(m_pPlayer->m_ZombieClass, "meatwall"))
			return 1000;
	}
		
	return 500;
}

CBaseZombieClass_ZB2::~CBaseZombieClass_ZB2() = default;

}
