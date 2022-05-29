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


 //#define __DEBUG

#define SKILL_FUNCTION(skillSet, func, ...) { \
	for (auto &skill : skillSet) { \
		skill->func(__VA_ARGS__); \
	} \
}

namespace sv {
	constexpr int MAX_LEVELS = 40;
	constexpr int HURT_PER_XP = 10; // 满足累计承受伤害后获得经验值
	constexpr int HURT_AMOUNT = 400; // 满足累计承受伤害值
	constexpr int DAMAGE_PER_XP = 10; // 满足累计造成伤害后获得经验值
	constexpr int DAMAGE_AMOUNT = 200; // 满足累计造成伤害值
	constexpr float EXP_SCALE = 0.1f; // 每回合每升一级所减少经验获取百分比
	constexpr duration_t ROUND_PERIOD_ADD = 60s; // 每回合时间超过多少秒后下一回合才会给予技能点
	constexpr duration_t UPGRADE_SKILL_DELAY = 1s;

	CPlayerModStrategy_ZBZ::CPlayerModStrategy_ZBZ(CBasePlayer* player, CMod_ZombieZ* mp)
		: CPlayerModStrategy_ZB3(player, static_cast<CMod_ZombieHero*>(mp)),
		m_pModZBZ(mp),
		m_eventTakeDamageListener(mp->m_eventTakeDamage.subscribe(&CPlayerModStrategy_ZBZ::Event_TakeDamage, this)),
		m_eventTraceAttackListener(mp->m_eventTraceAttack.subscribe(&CPlayerModStrategy_ZBZ::Event_TraceAttack, this)),
		m_iLevel(0),
		m_iExp(0),
		m_iPoint(0),
		m_flTakeDamage(0.0f),
		m_flSufferDamage(0.0f),
		m_flExpScale(1.0f)

	{
		m_iChangeClassTime[0] = 0;
		m_iChangeClassTime[1] = 0;
		m_iChangeClassTime[2] = 0;
		m_tNextChangeClass = invalid_time_point;
		m_flNextTimeChangeZombie = invalid_time_point;
		m_iZB5EvolutionEffect = PRECACHE_MODEL("models/zb5_za_evolution_effect.mdl");
}

void CPlayerModStrategy_ZBZ::OnInitHUD()
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZBZ_MESSAGE_LEVEL_INFO);
	WRITE_BYTE(m_iLevel);
	WRITE_BYTE(m_iExp);
	WRITE_BYTE(m_iPoint);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZBZ_MESSAGE_SKILL_REMOVE);
	WRITE_BYTE(0);
	MESSAGE_END();
}

void CPlayerModStrategy_ZBZ::Evolution(ZombieLevel iEvolutionLevel)
{
	CPlayerModStrategy_ZB3::Evolution(iEvolutionLevel);

	if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 7000.0f + g_pGameRules->m_iTotalRoundsPlayed * 500;
		m_pPlayer->pev->armorvalue = 500.0f + g_pGameRules->m_iTotalRoundsPlayed * 100;
	}
	else if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 14000.0f + g_pGameRules->m_iTotalRoundsPlayed * 500;
		m_pPlayer->pev->armorvalue = 1000.0f + g_pGameRules->m_iTotalRoundsPlayed * 100;
	}

	SKILL_FUNCTION(m_vecSkill, OnEvolution, iEvolutionLevel);
}

void CPlayerModStrategy_ZBZ::GrenadeExploded(Vector vecOrigin)
{
	SKILL_FUNCTION(m_vecSkill, OnGrenadeExploded, vecOrigin);
}

void CPlayerModStrategy_ZBZ::ZombiBombExploded(float &flKickRate, float &flRadius, float &flDamage)
{
	SKILL_FUNCTION(m_vecSkill, OnZombiBombExploded, flKickRate, flRadius, flDamage);
}

void CPlayerModStrategy_ZBZ::FlashbangExploded(float& flKickRate, float& flRadius, time_point_t& flFreezeTime)
{
	//SKILL_FUNCTION(m_vecSkill, OnFlashbangExploded, flKickRate, flRadius, flFreezeTime);
}

void CPlayerModStrategy_ZBZ::ShootGrenade(const std::function<void(void)>& f)
{
	SKILL_FUNCTION(m_vecSkill, OnShootGrenade, f);
}

float CPlayerModStrategy_ZBZ::AdjustZombieKnifeRange(bool bStab, float flRange)
{
	SKILL_FUNCTION(m_vecSkill, OnAdjustZombieKnifeRange, bStab, flRange);
	return flRange;
}

Vector CPlayerModStrategy_ZBZ::AdjustFireBullets3Dir(Vector vecDir)
{
	SKILL_FUNCTION(m_vecSkill, OnAdjustFireBullets3Dir, vecDir);
	return vecDir;
}

int CPlayerModStrategy_ZBZ::ComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax)
{
	int ret = CPlayerModStrategy_ZB3::ComputeMaxAmmo(szAmmoClassName, iOriginalMax);
	if (!strcmp(szAmmoClassName, "HEGrenade") || !strcmp(szAmmoClassName, "Flashbang") || !strcmp(szAmmoClassName, "SmokeGrenade"))
		return ret;

	SKILL_FUNCTION(m_vecSkill, OnComputeMaxAmmo, szAmmoClassName, iOriginalMax, ret);
	return ret;
}

int CPlayerModStrategy_ZBZ::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	int result = CPlayerModStrategy_ZB3::AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);
	SKILL_FUNCTION(m_vecSkill, OnAddToFullPack_Post, state, e, ent, host, hostflags, player, pSet);
	return result;
}

duration_t CPlayerModStrategy_ZBZ::OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t fDelay)
{
	SKILL_FUNCTION(m_vecSkill, OnDefaultReload_Pre, weapon, fDelay);
	return fDelay;
}

void CPlayerModStrategy_ZBZ::OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int &iClip, int &iBackAmmo)
{
	SKILL_FUNCTION(m_vecSkill, OnAttack_Post, weapon, iClipBeforeAttack, iClip, iBackAmmo);
}

void CPlayerModStrategy_ZBZ::Holster_Post(CBasePlayerWeapon* weapon)
{
	SKILL_FUNCTION(m_vecSkill, OnHolster_Post, weapon);
}

BOOL CPlayerModStrategy_ZBZ::DeployWeapon(CBasePlayerItem *item)
{
	CPlayerModStrategy_ZB2::DeployWeapon(item);
	SKILL_FUNCTION(m_vecSkill, OnDeployWeapon, item);
	return TRUE;
}

int CPlayerModStrategy_ZBZ::AdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int iClip)
{
	SKILL_FUNCTION(m_vecSkill, OnAdjustDefaultReloadClip, weapon, iClip);
	return iClip;
}

void CPlayerModStrategy_ZBZ::UpdatePlayerEvolutionHUD()
{
	CPlayerModStrategy_ZB3::UpdatePlayerEvolutionHUD();
}

bool CPlayerModStrategy_ZBZ::ClientCommand(const char *pcmd)
{

#ifdef __DEBUG
	if (!Q_stricmp(pcmd, "zskill"))
	{
		m_iPoint = 99;
		return true;
	}
	else if (!Q_stricmp(pcmd, "zombie"))
	{
		m_pPlayer->m_iTeam = TERRORIST;
		const char* pszArg1 = CMD_ARGV_(1);
		if (pszArg1)
			m_pPlayer->m_ZombieClass = pszArg1;
		else
			m_pPlayer->m_ZombieClass = "random";

		m_pPlayer->pev->sequence = 0;
		m_pModZBZ->MakeZombie(m_pPlayer, ZOMBIE_LEVEL_ORIGIN);
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 14000.0f;
		m_pPlayer->pev->armorvalue = 3000;
		return true;
	}
	else if (!Q_stricmp(pcmd, "human"))
	{
		m_pPlayer->m_iTeam = CT;
		m_pPlayer->m_bIsZombie = false;
		BecomeHuman();
		m_pPlayer->GiveDefaultItems();
		return true;
	}
	else if (!Q_stricmp(pcmd, "spawn"))
	{
		m_pPlayer->pev->deadflag = DEAD_RESPAWNABLE;
		m_pPlayer->RoundRespawn();
		return true;
	}
	else if (!Q_stricmp(pcmd, "bot_place"))
	{
		Vector vecSrc = m_pPlayer->GetGunPosition();
		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecOrigin = vecSrc + gpGlobals->v_forward * 60;

		moe::range::PlayersList list;
		std::random_device rd;
		std::vector<CBasePlayer*> players(list.begin(), list.end());
		players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer* player) {
			return !player->IsAlive() || !player->IsBot();
		}), players.end());
		std::shuffle(players.begin(), players.end(), rd);
		UTIL_SetOrigin(players.front()->pev, vecOrigin);
	}
#endif

	if (!m_pPlayer->m_bIsZombie)
	{
		if (!Q_strnicmp(pcmd, "MoE_HumanSkill", 14)) // MoE_HumanSkill[1-4]
		{
			switch (pcmd[14])
			{
			case '1':
				if (!CheckSkill(Skill_Specialist))
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "你没有[持之以恒]无法使用特殊技能");
					return true;
				}
				GetCharacter()->ActivateSkill(SKILL_SLOT_1);
				break;
			case '2':
				if (!CheckSkill(Skill_Specialist))
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "你没有[持之以恒]无法使用特殊技能");
					return true;
				}
				GetCharacter()->ActivateSkill(SKILL_SLOT_2);
				break;
			case '3':
				if (!CheckSkill(Skill_Knife2X))
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "你没有[屠戮之刃]无法使用特殊技能");
					return true;
				}
				GetCharacter()->ActivateSkill(SKILL_SLOT_3);
				break;
			case '4':
				if (!CheckSkill(Skill_Specialist))
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "你没有[持之以恒]无法使用特殊技能");
					return true;
				}
				GetCharacter()->ActivateSkill(SKILL_SLOT_4);
				break;
			}
			return true;
		}
	}

	return CPlayerModStrategy_ZB3::ClientCommand(pcmd);
}

void CPlayerModStrategy_ZBZ::OnSpawn()
{
	ClearFire();
	if (!m_pPlayer->IsUsingPC())
	{
		while (m_iPoint > 0 && UpgradeSkill()) {}
	}

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
	// this is a bad solution....
	/*if (!CheckSkill(Skill_HeroicPresence) && m_pPlayer->m_ZombieClass == "revival")
	{
		m_pPlayer->m_ZombieClass = "random";
		m_pModZBZ->MakeZombie(m_pPlayer, m_pPlayer->m_iZombieLevel);
	}*/
	SKILL_FUNCTION(m_vecSkill, OnBecomeZombie, iEvolutionLevel);
}

void CPlayerModStrategy_ZBZ::Event_OnInfection(CBasePlayer * victim, CBasePlayer * attacker)
{
	SKILL_FUNCTION(m_vecSkill, OnInfection, victim, attacker);

	if (CheckSkill(Skill_Fanatic))
	{
		CPlayerModStrategy_ZB3::Event_OnInfection(victim, attacker);
	}
	else
	{
		CPlayerModStrategy_ZB2::Event_OnInfection(victim, attacker);
	}
}

float CPlayerModStrategy_ZBZ::AdjustDamageTaken(entvars_t * pevInflictor, entvars_t * pevAttacker, float flDamage, int bitsDamageType)
{
	if (CheckSkill(Skill_Fanatic))
	{
		flDamage = CPlayerModStrategy_ZB3::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}
	else
	{
		flDamage = CPlayerModStrategy_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

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
	CPlayerModStrategy_ZB2::Event_AdjustHumanDamage(attacker, flDamage);

	SKILL_FUNCTION(m_vecSkill, OnAdjustDamage, attacker, flDamage);
}

void CPlayerModStrategy_ZBZ::Event_AdjustHumanHitgroup(CBasePlayer* attacker, HitBoxGroup& iHitgroup)
{
	CPlayerModStrategy_ZB2::Event_AdjustHumanHitgroup(attacker, iHitgroup);

	if (CheckSkill(Skill_SwordMaster))
	{
		if (!m_pPlayer->m_bIsZombie && GetCharacter()->GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING)
		{
			iHitgroup = HITGROUP_HEAD;
		}
	}
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

void CPlayerModStrategy_ZBZ::SetLevel(int lv)
{
	lv = min(lv, MAX_LEVELS);
	m_iLevel = lv;
	m_iPoint = lv;

	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZBZ_MESSAGE_LEVEL_INFO);
	WRITE_BYTE(m_iLevel);
	WRITE_BYTE(m_iExp);
	WRITE_BYTE(m_iPoint);
	MESSAGE_END();
}

void CPlayerModStrategy_ZBZ::AddLevel(int lv)
{
	if (m_iLevel >= MAX_LEVELS)
		return;

	m_iLevel++;
	m_iPoint++;

	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZBZ_MESSAGE_LEVEL_INFO);
	WRITE_BYTE(m_iLevel);
	WRITE_BYTE(m_iExp);
	WRITE_BYTE(m_iPoint);
	MESSAGE_END();
}

void CPlayerModStrategy_ZBZ::AddExp(int exp)
{
	if (m_iLevel >= MAX_LEVELS)
		return;

	bool bLevelUp = false;
	m_iExp += exp * m_flExpScale;
	while (m_iExp >= ((m_iLevel + 1) * 2 * 50))
	{
		m_iLevel++;
		m_iPoint++;
		m_iExp = 0;
		m_flExpScale = max(0.0f, m_flExpScale - EXP_SCALE);
		bLevelUp = true;

		if (!m_pPlayer->IsUsingPC() || m_pPlayer->IsBot())
		{
			UpgradeSkill();
		}
	}

	if (bLevelUp)
	{
		char buf[64];
		Q_snprintf(buf, ARRAYSIZE(buf), "等级提升！Level %i", m_iLevel);
		ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, buf);
		CLIENT_COMMAND(m_pPlayer->edict(), "spk metalarena/level_up.wav\n");
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZBZ_MESSAGE_LEVEL_INFO);
	WRITE_BYTE(m_iLevel);
	WRITE_BYTE(m_iExp);
	WRITE_BYTE(m_iPoint);
	MESSAGE_END();
}

void CPlayerModStrategy_ZBZ::Event_TraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType)
{
	SKILL_FUNCTION(m_vecSkill, OnTraceAttack, victim, pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
}

void CPlayerModStrategy_ZBZ::OnThink()
{
	if (IsLadderDisabled() && m_pPlayer->pev->flags & FL_ONGROUND)
	{
		SET_PHYSICS_KEY_VALUE(m_pPlayer->edict(), "drp", "0");
	}

	if (IsOnFire() && m_flFireDmg <= gpGlobals->time && m_pPlayer->m_bIsZombie)
	{
		bool fireElapsed = true;
		for (int i = 1; i <= gpGlobals->maxClients; ++i)
		{
			if (m_vecFireList[i] <= gpGlobals->time)
				continue;

			CBasePlayer* atk = static_cast<CBasePlayer*>(UTIL_PlayerByIndex(i));
			if (!(atk && !atk->m_bIsZombie))
				continue;

			fireElapsed = false;
			m_pPlayer->TakeDamage(atk->pev, atk->pev, 20.0f + 20.0f * g_pGameRules->m_iTotalRoundsPlayed, DMG_BURN | DMG_NOMORALE);
		}
		float flSlow = 0.8f - 0.01f * g_pGameRules->m_iTotalRoundsPlayed;
		if(m_pPlayer->m_flVelocityModifier > flSlow && m_pPlayer->m_iBuff & BUFF_IGNORE_KNOCKBACK)
			m_pPlayer->m_flVelocityModifier = flSlow;

		m_bIsFiring = !fireElapsed;
		m_flFireDmg = gpGlobals->time + 0.5s;
	}

	SKILL_FUNCTION(m_vecSkill, OnThink);
	return CPlayerModStrategy_ZB3::OnThink();
}

void CPlayerModStrategy_ZBZ::OnTouch(CBaseEntity* other)
{
	SKILL_FUNCTION(m_vecSkill, OnTouch, other);
	return CPlayerModStrategy_ZB3::OnTouch(other);
}

void CPlayerModStrategy_ZBZ::OnKilled(entvars_t * pKiller, entvars_t * pInflictor)
{
	SKILL_FUNCTION(m_vecSkill, OnKilled, pKiller, pInflictor);
	return CPlayerModStrategy_ZB3::OnKilled(pKiller, pInflictor);
}

void CPlayerModStrategy_ZBZ::Event_OnRoundStart()
{
	m_flExpScale = 1.0f;
	if (m_bRoundStartAddLevel)
	{
		AddLevel(1);
		m_bRoundStartAddLevel = false;
	}

	SKILL_FUNCTION(m_vecSkill, OnRoundStart);
	return CPlayerModStrategy_ZB3::Event_OnRoundStart();
}

//int CPlayerModStrategy_ZBZ::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
//{
//	return CPlayerModStrategy_ZB3::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
//}

bool CPlayerModStrategy_ZBZ::IsSkillAvailable(ZombieZSkillSkillId skill)
{
    return m_setSkillID.find(skill) == m_setSkillID.end();
}

bool CPlayerModStrategy_ZBZ::UpgradeSkill()
{
    auto data = ZSkillFactory(m_pPlayer, std::bind(&CPlayerModStrategy_ZBZ::IsSkillAvailable, this, std::placeholders::_1));
	if (!data.second)
		return false;

	m_iPoint--;
	m_setSkillID.insert(data.first);
	m_vecSkill.insert(data.second);
	data.second->OnGetSkill();

	if (!m_pPlayer->IsBot())
	{
		ClientPrint(m_pPlayer->pev, HUD_PRINTTALK, "获得技能：%s-%s", data.second->GetName(), data.second->GetDesc());
		CLIENT_COMMAND(m_pPlayer->edict(), "spk player/zombiez_mutation.wav\n");

		MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
		WRITE_BYTE(ZBZ_MESSAGE_SKILL_ADD);
		WRITE_BYTE(data.first);
		MESSAGE_END();
	}

	if (m_pPlayer->IsUsingPC())
	{
#ifndef __DEBUG
		m_flSkillDelay = gpGlobals->time + UPGRADE_SKILL_DELAY;
		m_pPlayer->ResetMaxSpeed();
#else
		m_flSkillDelay = gpGlobals->time + 0.01s;
#endif
		if (!m_pPlayer->m_bIsZombie)
		{
			m_pPlayer->pev->viewmodel = MAKE_STRING("models/v_emotion.mdl");
			m_pPlayer->pev->weaponmodel = 0;
		}
		m_pPlayer->m_bHolsterDisabled = true;
		if (CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_pActiveItem)
		{
			pWeapon->SendWeaponAnim(28, TRUE);
			pWeapon->m_flNextPrimaryAttack = UPGRADE_SKILL_DELAY;
			pWeapon->m_flNextSecondaryAttack = UPGRADE_SKILL_DELAY;
			pWeapon->m_flTimeWeaponIdle = UPGRADE_SKILL_DELAY;
		}
	}
	return true;
}

void CPlayerModStrategy_ZBZ::Event_OnPickAsZombie(CBasePlayer* who)
{
	if (m_pPlayer != who)
		return;

	SetPickAsZombie(true);

	if (!CheckSkill(Skill_SixthSense))
		return;

	char szHudText[64];
	Q_sprintf(szHudText, "您已被感染，可在[变异阶段]使用[E]键变成僵尸");

	hudtextparms_t textParms;
	textParms.x = -1.0f;
	textParms.y = 0.3f;
	textParms.effect = 2;
	textParms.r1 = 200;
	textParms.g1 = 200;
	textParms.b1 = 50;
	textParms.r2 = 255;
	textParms.g2 = 255;
	textParms.b2 = 255;
	textParms.fadeinTime = 0.1f;
	textParms.fadeoutTime = 1.5f;
	textParms.holdTime = 3.0f;
	textParms.fxTime = 0.1f;
	textParms.channel = 1;
	UTIL_HudMessage(m_pPlayer, textParms, szHudText);
}

void CPlayerModStrategy_ZBZ::CmdStart(struct usercmd_s* cmd, unsigned int random_seed)
{
	auto& bitsCurButton = cmd->buttons;
	auto& bitsOldButton = m_pPlayer->pev->oldbuttons;

	if (m_flSkillDelay != invalid_time_point && m_flSkillDelay <= gpGlobals->time)
	{
		m_pPlayer->m_bHolsterDisabled = false;
		m_flSkillDelay = invalid_time_point;
		if (m_pPlayer->IsAlive() && m_pPlayer->m_pActiveItem)
		{
			m_pPlayer->SwitchWeapon(m_pPlayer->m_pActiveItem);
		}
	}

	if (bitsCurButton & ~bitsOldButton & IN_USE)
	{
		if (m_pPlayer->IsAlive() && !m_pPlayer->m_bIsZombie && IsPickAsZombie() && CheckSkill(Skill_SixthSense))
		{
			m_tNextChangeClass = gpGlobals->time + UPGRADE_SKILL_DELAY;
			auto iNumZombies = m_pModZBZ->ZombieOriginNum();
			auto iNumPlayers = m_pModZBZ->m_iNumTerrorist + m_pModZBZ->m_iNumCT;

			m_pPlayer->m_ZombieClass = "random";
			m_pPlayer->pev->sequence = 0;
			m_pPlayer->m_iCustomSequence = -1;
			m_pModZBZ->MakeZombie(m_pPlayer, ZOMBIE_LEVEL_ORIGIN);
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 1000.0f * iNumPlayers / iNumZombies + 3000.0f;
			m_pPlayer->pev->armorvalue = 1100;
			m_pModZBZ->TeamCheck();

			// sound effect
			m_pModZBZ->InfectionSound();
			m_pModZBZ->CheckWinConditions();
			return;
		}

		if (m_tNextChangeClass <= gpGlobals->time && m_pPlayer->IsAlive() && m_iChangeClassTime[1] > 0 && !m_pPlayer->m_bIsZombie && !m_pPlayer->IsBot())
		{
			m_iChangeClassTime[1] = -1;
			m_tNextChangeClass = gpGlobals->time + UPGRADE_SKILL_DELAY;
			SetHero(true);
			m_pPlayer->SetScoreboardAttributes();
			m_pPlayer->pev->max_health = 10000.0f;
			m_pPlayer->pev->health += 6000.0f;
			if (m_pPlayer->pev->health >= m_pPlayer->pev->max_health)
				m_pPlayer->pev->health = m_pPlayer->pev->max_health;

			m_pPlayer->pev->armorvalue = 1000.0f;
			SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", "ghosthunter");
			m_pPlayer->SetNewPlayerModel("models/player/ghosthunter/ghosthunter.mdl");
			m_pPlayer->RemoveAllItems(FALSE);
			m_pPlayer->m_bHasPrimary = false;
			m_pPlayer->GiveNamedItem("z4b_dualkukri");
			m_pPlayer->m_bHolsterDisabled = true;
			m_pPlayer->m_iBuff |= BUFF_GHOSTHUNTER;

			MESSAGE_BEGIN(MSG_ONE, gmsgZBZMsg, nullptr, m_pPlayer->edict());
			WRITE_BYTE(ZBZ_MESSAGE_GHOSTHUNTER);
			WRITE_BYTE(0);
			WRITE_BYTE(2);
			MESSAGE_END();

			UTIL_ClientPrintAll(HUD_PRINTCENTER, "%s 成为幽灵猎手", STRING(m_pPlayer->pev->netname));

			EMIT_SOUND(m_pPlayer->edict(), CHAN_AUTO, "zombi/ghosthunterappear.wav", VOL_NORM, ATTN_NORM);
			return;
		}

		if (m_flSkillDelay <= gpGlobals->time && m_iPoint > 0 && m_pPlayer->IsAlive())
		{
			m_tNextChangeClass = gpGlobals->time + UPGRADE_SKILL_DELAY;
			UpgradeSkill();
			return;
		}
			
		if (m_tNextChangeClass <= gpGlobals->time && m_pPlayer->IsAlive() && m_iChangeClassTime[0] > 0 && m_pPlayer->m_bIsZombie && !m_pPlayer->IsBot())
		{
			if (GetCharacter()->GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING || GetCharacter()->GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING)
			{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "当前技能使用完之后再使用生化超越技能");
				return;
			}

			if (m_pPlayer && m_pPlayer->m_ZombieClass && !stricmp(m_pPlayer->m_ZombieClass, "nemesis"))
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
				if (pWeapon)
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "检测到主武器，请移除主武器再使用生化超越技能");
					return;
				}
			}

			m_iChangeClassTime[0] = -1;
			m_tNextChangeClass = gpGlobals->time + UPGRADE_SKILL_DELAY;
			switch (RANDOM_LONG(0, 2))
			{
			case 0: m_pPlayer->m_ZombieClass = "nemesis"; break;
			case 1: m_pPlayer->m_ZombieClass = "spider"; break;
			case 2: m_pPlayer->m_ZombieClass = "deathknight"; break;
			}		
			
			m_pPlayer->pev->sequence = 0;
			m_pPlayer->m_iCustomSequence = -1;
			CPlayerModStrategy_ZB3::BecomeZombie(ZOMBIE_LEVEL_ORIGIN_LV2);
			m_pPlayer->pev->health = m_pPlayer->pev->max_health = 30000;
			m_pPlayer->pev->armorvalue = 9000;

			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY);
			WRITE_BYTE(TE_TEMPMODEL);
			WRITE_COORD(m_pPlayer->pev->origin.x);
			WRITE_COORD(m_pPlayer->pev->origin.y);
			WRITE_COORD(m_pPlayer->pev->origin.z);
			WRITE_ANGLE(g_vecZero.x);
			WRITE_ANGLE(g_vecZero.y);
			WRITE_ANGLE(g_vecZero.z);
			WRITE_COORD(g_vecZero.x);
			WRITE_COORD(g_vecZero.y);
			WRITE_COORD(g_vecZero.z);
			WRITE_SHORT(m_iZB5EvolutionEffect);
			WRITE_BYTE(10);	//life
			WRITE_SHORT(0);
			WRITE_BYTE(1);
			WRITE_BYTE(false);
			WRITE_BYTE(255);
			WRITE_BYTE(kRenderNormal);
			WRITE_SHORT(-1);
			WRITE_BYTE(0);
			WRITE_BYTE(false);
			WRITE_BYTE(0);
			WRITE_BYTE(0);
			WRITE_SHORT(31);
			WRITE_LONG(0);
			MESSAGE_END();

			auto strategy = dynamic_cast<CPlayerModStrategy_ZB1*>(m_pPlayer->m_pModStrategy.get());
			if (strategy)
			{
				strategy->SetStunSpeedTime(1.0s, 1.0);
			}
			return;
		}

		if (m_tNextChangeClass <= gpGlobals->time && m_pPlayer->IsAlive() && m_iChangeClassTime[2] > 0 && m_pPlayer->m_bIsZombie && !m_pPlayer->IsBot())
		{
			if (GetCharacter()->GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING || GetCharacter()->GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING)
			{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "当前技能使用完之后再使用适应力技能");
				return;
			}

			if (m_pPlayer && m_pPlayer->m_ZombieClass && !stricmp(m_pPlayer->m_ZombieClass, "nemesis"))
			{
				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];
				if (pWeapon)
				{
					ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "检测到主武器，请移除主武器再使用适应力技能");
					return;
				}
			}

			float flHealth = m_pPlayer->pev->health;
			float flMaxHealth = m_pPlayer->pev->max_health;
			m_tNextChangeClass = gpGlobals->time + UPGRADE_SKILL_DELAY;
			if (flHealth <= 3000 || m_flNextTimeChangeZombie > gpGlobals->time)
			{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "血量不足或切换冷却时间未到，无法改变僵尸种类");
				return;
			}

			m_flNextTimeChangeZombie = gpGlobals->time + 10.0s;
			m_pPlayer->m_ZombieClass = "random";
			m_pPlayer->pev->sequence = 0;
			m_pPlayer->m_iCustomSequence = -1;
			CPlayerModStrategy_ZB3::BecomeZombie(m_pPlayer->m_iZombieLevel);
			m_pPlayer->pev->max_health = flMaxHealth;
			m_pPlayer->pev->health = flHealth - 3000;
			return;
		}
	}

	SKILL_FUNCTION(m_vecSkill, OnButtonEvent, bitsCurButton, bitsOldButton);
	return CPlayerModStrategy_ZB2::CmdStart(cmd, random_seed);
}

void CPlayerModStrategy_ZBZ::OnResetMaxSpeed() {
	CPlayerModStrategy_ZB3::OnResetMaxSpeed();
	if (m_flSkillDelay > gpGlobals->time)
		m_pPlayer->pev->maxspeed = 1.0f;
	if (m_pPlayer->pev->maxspeed <= 1.0f)
		return;
	SKILL_FUNCTION(m_vecSkill, OnResetMaxSpeed);
}

bool CPlayerModStrategy_ZBZ::ApplyKnockback(CBasePlayer* attacker, const KnockbackData& data)
{
	KnockbackData knockback = {
		data.flOnGround,
		data.flNotOnGround,
		data.flFlying,
		data.flDucking,
		data.flVelocityModifier
	};
	SKILL_FUNCTION(m_vecSkill, OnApplyKnockback, attacker, knockback);
	return m_pCharacter->ApplyKnockback(attacker, knockback);
}

void CPlayerModStrategy_ZBZ::ClientPutInServer()
{
	int iMinLevel = 9999;
	moe::range::PlayersList list;
	for (CBasePlayer* player : list)
	{
		if (m_pPlayer == player)
			continue;
		auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(player->m_pModStrategy.get());
		if (!strategy) continue;
		if(strategy->m_iLevel < iMinLevel)
			iMinLevel = strategy->m_iLevel;
	}
	SetLevel(iMinLevel == 9999 ? 1 : iMinLevel);
}

CMod_ZombieZ::CMod_ZombieZ()
{
	Skill_Precache();

	PRECACHE_MODEL("models/v_emotion.mdl");
	PRECACHE_SOUND("metalarena/level_up.wav");
	PRECACHE_SOUND("player/zombiez_mutation.wav");
	PRECACHE_SOUND("zombi/ghosthunterappear.wav");
	PRECACHE_SOUND("zombi/KILLNANOCOUNTSND.WAV");
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
	if(pInflictor && (FClassnameIs(pInflictor, "z4b_dualkukri") || FClassnameIs(pInflictor, "z4b_dualslayer") || FClassnameIs(pInflictor, "z4b_holyfist")))
		UTIL_BroadcastStuffText("spk \"zombi/KILLNANOCOUNTSND\"\n");
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
	CBasePlayer* pVictim = dynamic_ent_cast<CBasePlayer*>(victim);
	CBasePlayer* pAttacker = dynamic_ent_cast<CBasePlayer*>(pevAttacker);
	m_eventTraceAttack.dispatch(pVictim, pAttacker, flDamage, vecDir, ptr, bitsDamageType);
	HitBoxGroup hbg = static_cast<HitBoxGroup>(ptr->iHitgroup);
	hbg = CMod_ZombieMod2::GetAdjustedTraceAttackHitgroup(victim, pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
	return hbg;
}

void CMod_ZombieZ::PickHero()
{
	// randomize player list
	moe::range::PlayersList list;
	std::random_device rd;
	std::vector<CBasePlayer*> players(list.begin(), list.end());
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer* player) {
		auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(player->m_pModStrategy.get());
		if (!strategy || !strategy->CheckSkill(Skill_HeroicPresence))
		{
			return true;
		}
		return !player->IsAlive() || player->m_iTeam != TEAM_CT || player->m_bIsZombie;
	}), players.end());
	std::shuffle(players.begin(), players.end(), rd);
	// make heroes
	const auto iNumHeroes = std::min(players.size(), players.size() / 10 + std::uniform_int_distribution<size_t>(0, 1)(rd));
	std::for_each(players.begin(), players.begin() + iNumHeroes, std::bind(&CMod_ZombieZ::MakeHero, this, std::placeholders::_1));
}

void CMod_ZombieZ::HumanWin()
{
	CMod_ZombieHero::HumanWin();
	if (gpGlobals->time - m_fRoundCount >= ROUND_PERIOD_ADD)
	{
		for (CBasePlayer* player : moe::range::PlayersList())
		{
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(player->m_pModStrategy.get());
			if (!strategy) continue;
			strategy->SetRoundStartAddLevel(true);
		}
	}
}

void CMod_ZombieZ::ZombieWin()
{
	CMod_ZombieHero::ZombieWin();
	if (gpGlobals->time - m_fRoundCount >= ROUND_PERIOD_ADD)
	{
		for (CBasePlayer* player : moe::range::PlayersList())
		{
			auto strategy = dynamic_cast<CPlayerModStrategy_ZBZ*>(player->m_pModStrategy.get());
			if (!strategy) continue;
			strategy->SetRoundStartAddLevel(true);
		}
	}
}

}
