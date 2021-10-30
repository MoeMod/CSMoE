/*
mod_zbz.cpp - CSMoE Gameplay server : Zombie Hero
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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"

#include "mod_zbz.h"
#include "player/player_zombie.h"
#include "player/player_mod_strategy.h"

#include "util/u_range.hpp"

#include <vector>
#include <algorithm>
#include <functional>
#include <random>

#define SKILL_FUNCTION(skillSet, func, ...) { \
	for (auto &skill : skillSet) { \
		skill->func(__VA_ARGS__); \
	} \
}

namespace sv {
	constexpr int MAX_LEVELS = 30;
	constexpr int HURT_PER_XP = 10; // 满足累计承受伤害后获得经验值
	constexpr int HURT_AMOUNT = 400; // 满足累计承受伤害值
	constexpr int DAMAGE_PER_XP = 10; // 满足累计造成伤害后获得经验值
	constexpr int DAMAGE_AMOUNT = 200; // 满足累计造成伤害值

	CPlayerModStrategy_ZBZ::CPlayerModStrategy_ZBZ(CBasePlayer* player, CMod_ZombieZ* mp)
		: CPlayerModStrategy_ZB3(player, static_cast<CMod_ZombieHero*>(mp)),
		m_pModZBZ(mp),
		m_eventTakeDamageListener(mp->m_eventTakeDamage.subscribe(&CPlayerModStrategy_ZBZ::Event_TakeDamage, this)),
		m_eventTraceAttackListener(mp->m_eventTraceAttack.subscribe(&CPlayerModStrategy_ZBZ::Event_TraceAttack, this)),
		m_iLevel(0),
		m_iExp(0),
		m_iPoint(0),
		m_flTakeDamage(0.0f),
		m_flSufferDamage(0.0f)
{
	
}

void CPlayerModStrategy_ZBZ::OnSpawn()
{
	SKILL_FUNCTION(m_vecSkill, OnSpawn);
	return CPlayerModStrategy_ZB3::OnSpawn();
}

void CPlayerModStrategy_ZBZ::BecomeHuman()
{
	CPlayerModStrategy_ZB3::BecomeHuman();
	SKILL_FUNCTION(m_vecSkill, OnBecomeHuman);
}

void CPlayerModStrategy_ZBZ::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	CPlayerModStrategy_ZB3::BecomeZombie(iEvolutionLevel);
	SKILL_FUNCTION(m_vecSkill, OnBecomeZombie, iEvolutionLevel);
}

void CPlayerModStrategy_ZBZ::Event_OnInfection(CBasePlayer * victim, CBasePlayer * attacker)
{
	SKILL_FUNCTION(m_vecSkill, OnInfection, victim, attacker);
	return CPlayerModStrategy_ZB3::Event_OnInfection(victim, attacker);
}

float CPlayerModStrategy_ZBZ::AdjustDamageTaken(entvars_t * pevInflictor, entvars_t * pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = CPlayerModStrategy_ZB3::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	for (auto& skill : m_vecSkill)
	{
		flDamage = skill->OnAdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
		if (flDamage <= 0.0)
			break;
	}
	
	return flDamage;
}

void CPlayerModStrategy_ZBZ::Event_AdjustHumanDamage(CBasePlayer* attacker, float& flDamage)
{
	if (attacker != m_pPlayer)
		return;

	SKILL_FUNCTION(m_vecSkill, OnAdjustDamage, attacker, flDamage);
}

void CPlayerModStrategy_ZBZ::Event_TakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType)
{
	if (victim == pevAttacker)
		return;

	if (victim == m_pPlayer && pevAttacker) {
		m_flSufferDamage += flDamage;
		while (m_flSufferDamage >= HURT_AMOUNT) {
			AddExp(HURT_PER_XP);
			m_flSufferDamage -= HURT_AMOUNT;
		}
	}

	if (pevAttacker == m_pPlayer) {
		m_flTakeDamage += flDamage;
		while (m_flTakeDamage >= DAMAGE_AMOUNT) {
			AddExp(DAMAGE_PER_XP);
			m_flTakeDamage -= DAMAGE_AMOUNT;
		}
	}
	SKILL_FUNCTION(m_vecSkill, OnTakeDamage, victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPlayerModStrategy_ZBZ::AddExp(int exp)
{
	if (m_iLevel >= MAX_LEVELS)
		return;

	bool bLevelUp = false;
	m_iExp += exp;
	while (m_iExp >= ((m_iLevel + 1) * 2 * 50))
	{
		m_iLevel++;
		m_iPoint++;
		m_iExp = 0;
		bLevelUp = true;
	}

	if (bLevelUp)
	{
		char buf[64];
		Q_snprintf(buf, ARRAYSIZE(buf), "等级提升！Level %i", m_iLevel);
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, buf);
		/*PlaySound(id, g_szSound[0])*/
	}
}

void CPlayerModStrategy_ZBZ::Event_TraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType)
{
	SKILL_FUNCTION(m_vecSkill, OnTraceAttack, victim, pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CPlayerModStrategy_ZBZ::OnThink()
{
	SKILL_FUNCTION(m_vecSkill, OnThink);
	return CPlayerModStrategy_ZB3::OnThink();
}

void CPlayerModStrategy_ZBZ::OnKilled(entvars_t * pKiller, entvars_t * pInflictor)
{
	SKILL_FUNCTION(m_vecSkill, OnKilled, pKiller, pInflictor);
	return CPlayerModStrategy_ZB3::OnKilled(pKiller, pInflictor);
}

void CPlayerModStrategy_ZBZ::Event_OnRoundStart()
{
	SKILL_FUNCTION(m_vecSkill, OnRoundStart);
	return CPlayerModStrategy_ZB3::Event_OnRoundStart();
}

void CPlayerModStrategy_ZBZ::CmdStart(struct usercmd_s* cmd, unsigned int random_seed)
{
	auto& bitsCurButton = cmd->buttons;
	auto& bitsOldButton = m_pPlayer->pev->oldbuttons;

	if (bitsCurButton & ~bitsOldButton & IN_USE)
	{
		if (m_iPoint <= 0)
			return;

		auto data = HumanSkillFactory(m_pPlayer, m_setSkillID);
		if (data.second == nullptr)
			return;

		m_iPoint--;
		m_setSkillID.insert(data.first);
		m_vecSkill.insert(data.second);
		ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "获得技能：%s-%s", data.second->GetName(), data.second->GetDesc());
		data.second->OnGetSkill();
	}

	SKILL_FUNCTION(m_vecSkill, OnButtonEvent, bitsCurButton, bitsOldButton);
	return CPlayerModStrategy_ZB1::CmdStart(cmd, random_seed);
}

void CPlayerModStrategy_ZBZ::OnResetMaxSpeed() {
	CPlayerModStrategy_ZB3::OnResetMaxSpeed();
}

CMod_ZombieZ::CMod_ZombieZ()
{
	Skill_Precache();
}

void CMod_ZombieZ::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZBZ(player, this));
}

void CMod_ZombieZ::UpdateGameMode(CBasePlayer * pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZBZ);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

void CMod_ZombieZ::RestartRound()
{
	return CMod_ZombieHero::RestartRound();
}

void CMod_ZombieZ::PlayerKilled(CBasePlayer * pVictim, entvars_t * pKiller, entvars_t * pInflictor)
{
	return CMod_ZombieHero::PlayerKilled(pVictim, pKiller, pInflictor);
}

float CMod_ZombieZ::GetAdjustedEntityDamage(CBaseEntity* victim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = CMod_ZombieMod2::GetAdjustedEntityDamage(victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);

	CBasePlayer* pVictim = dynamic_ent_cast<CBasePlayer*>(victim);
	CBasePlayer* pAttacker = dynamic_ent_cast<CBasePlayer*>(pevAttacker);
	m_eventTakeDamage.dispatch(pVictim, pevInflictor, pAttacker, flDamage, bitsDamageType);

	return flDamage;
}

HitBoxGroup CMod_ZombieZ::GetAdjustedTraceAttackHitgroup(CBaseEntity* victim, entvars_t* pevAttacker, float flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType)
{
	HitBoxGroup hbg = CMod_ZombieMod2::GetAdjustedTraceAttackHitgroup(victim, pevAttacker, flDamage, vecDir, ptr, bitsDamageType);

	CBasePlayer* pVictim = dynamic_ent_cast<CBasePlayer*>(victim);
	CBasePlayer* pAttacker = dynamic_ent_cast<CBasePlayer*>(pevAttacker);
	m_eventTraceAttack.dispatch(pVictim, pAttacker, flDamage, vecDir, ptr, bitsDamageType);
	hbg = static_cast<HitBoxGroup>(ptr->iHitgroup);
	return hbg;
}

}
