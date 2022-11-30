/*
mod_zb3.cpp - CSMoE Gameplay server : Zombie Hero
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
#include "client.h"
#include "entity_state.h"
#include "triggers.h"

#include "map_manager.h"
#include "mod_zb3zp.h"
#include "player/player_zombie.h"
#include "player/player_mod_strategy.h"
#include "zb3zp/zp_zclass_alien_boss.h"
#include "zb3zp/zp_zclass_source.h"
#include "zb3zp/zp_zclass_tank.h"
#include "zb3zp/zp_zclass_alien_infected.h"
#include "zb3zp/zp_zclass_ap.h"
#include "zb3zp/zp_zclass_hostex.h"
#include "zb3zp/zp_zclass_licker.h"
#include "zb3zp/zp_zclass_nohead.h"
#include "zb3zp/zp_zclass_pyrammid.h"
#include "zb3zp/zp_zclass_tf.h"
#include "zb3zp/zp_zclass_human.h"
#include "zb2/zb2_zclass_siren.h"

#include "weapons_moe_buy.h"

#include "util/u_range.hpp"

#include "Reciter.h"

#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <memory>
#include "player/csdm_randomspawn.h"


namespace sv {

void CMod_ZB3ZP::CheckMapConditions()
{
	CBaseEntity* fog = nullptr;
	while ((fog = UTIL_FindEntityByClassname(fog, "env_fog")) != nullptr)
	{
		REMOVE_ENTITY(fog->edict());
	}

	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("env_fog"));
	if (!FNullEnt(pent))
	{
		CClientFog* pEntity = static_cast<CClientFog*>(CBaseEntity::Instance(pent));
		DispatchSpawn(pEntity->edict());
		float fDenity = MapMgr_GetMapDensity(STRING(gpGlobals->mapname));
		pEntity->m_fDensity = fDenity;
		float iDensity_r = MapMgr_GetMapDensity_r(STRING(gpGlobals->mapname));
		float iDensity_g = MapMgr_GetMapDensity_g(STRING(gpGlobals->mapname));
		float iDensity_b = MapMgr_GetMapDensity_b(STRING(gpGlobals->mapname));
		pEntity->pev->rendercolor = { iDensity_r, iDensity_g, iDensity_b };
		m_pFogEntity = pEntity;
	}
	CMod_Zombi::CheckMapConditions();
}

void CMod_ZB3ZP::InstallPlayerModStrategy(CBasePlayer* player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3ZP(player, this));
}


float CMod_ZB3ZP::GetAdjustedEntityDamage(CBaseEntity* victim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = CMod_ZombieMod2::GetAdjustedEntityDamage(victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);

	CBasePlayer* pVictim = dynamic_ent_cast<CBasePlayer*>(victim);
	CBasePlayer* pAttacker = dynamic_ent_cast<CBasePlayer*>(pevAttacker);
	m_eventTakeDamage.dispatch(pVictim, pevInflictor, pAttacker, flDamage, bitsDamageType);

	return flDamage;
}

void CMod_ZB3ZP::HumanInfectionByZombie(CBasePlayer* player, CBasePlayer* attacker)
{
	CMod_ZombieMod2::HumanInfectionByZombie(player, attacker);

	if(!strcmp(player->m_ZombieClass, "alien_boss"))
		player->pev->health = player->pev->max_health = 2300.0f;
	else if(!strcmp(player->m_ZombieClass, "source"))
		player->pev->health = player->pev->max_health = 2500.0f;
	else if (!strcmp(player->m_ZombieClass, "licker"))
		player->pev->health = player->pev->max_health = 2000.0f;
	else if (!strcmp(player->m_ZombieClass, "alien_infected"))
		player->pev->health = player->pev->max_health = 1800.0f;
	else if (!strcmp(player->m_ZombieClass, "ap"))
		player->pev->health = player->pev->max_health = 3000.0f;
	else if (!strcmp(player->m_ZombieClass, "hostex"))
		player->pev->health = player->pev->max_health = 2500.0f;
	else if (!strcmp(player->m_ZombieClass, "humanzombie"))
		player->pev->health = player->pev->max_health = 1800.0f;
	else if (!strcmp(player->m_ZombieClass, "nohead"))
		player->pev->health = player->pev->max_health = 1800.0f;
	else if (!strcmp(player->m_ZombieClass, "pyrammid"))
		player->pev->health = player->pev->max_health = 2300.0f;
	else if (!strcmp(player->m_ZombieClass, "zptank"))
		player->pev->health = player->pev->max_health = 2000.0f;
	else if (!strcmp(player->m_ZombieClass, "tf"))
		player->pev->health = player->pev->max_health = 3000.0f;
	else if (!strcmp(player->m_ZombieClass, "siren"))
		player->pev->health = player->pev->max_health = 2000.0f;
}

void CMod_ZB3ZP::RestartRound()
{
	CMod_Zombi::RestartRound();
	m_eventRestartRound.dispatch();

}

CPlayerModStrategy_ZB3ZP::CPlayerModStrategy_ZB3ZP(CBasePlayer* player, CMod_ZB3ZP* mp)
	: CPlayerModStrategy_ZB3(player, static_cast<CMod_ZombieHero*>(mp)),
	m_eventTakeDamageListener(mp->m_eventTakeDamage.subscribe(&CPlayerModStrategy_ZB3ZP::Event_TakeDamage, this)),
	m_RestartRoundListener(mp->m_eventRestartRound.subscribe(&CPlayerModStrategy_ZB3ZP::Event_RestartRound, this))
{
	m_flNextCheckButton = invalid_time_point;
	m_flNextTimeChangeZombie = invalid_time_point;
	m_iKilledTime = 0;
}

void CPlayerModStrategy_ZB3ZP::GiveDefaultItems()
{
	if (!m_pPlayer->m_bIsZombie)
	{
		if (m_pPlayer->IsBot())
		{
			m_pPlayer->RemoveAllItems(FALSE);
			m_pPlayer->m_bHasPrimary = false;

			for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT, KNIFE_SLOT })
			{
				std::vector<MoEWeaponBuyInfo_s> candidate_list;
				std::copy_if(std::begin(g_MoEWeaponBuyInfoLocal), std::end(g_MoEWeaponBuyInfoLocal), std::back_inserter(candidate_list), [iSlot](const MoEWeaponBuyInfo_s& info) { return info.iSlot == iSlot; });

				std::shuffle(candidate_list.begin(), candidate_list.end(), std::random_device());
				const MoEWeaponBuyInfo_s& info = candidate_list.front();
				if (iSlot == KNIFE_SLOT) {
					m_pPlayer->m_iKnifeID = AliasToKnifeType(info.pszClassName);
				}
				m_pPlayer->GiveNamedItem(info.pszClassName);
				GiveSlotAmmo(m_pPlayer, iSlot);
			}
			m_pPlayer->GiveNamedItem("weapon_hegrenade");
		}
		else
			return CPlayerModStrategy_Default::GiveDefaultItems();	
	}
	return CPlayerModStrategy_ZB3::GiveDefaultItems();
}

bool CPlayerModStrategy_ZB3ZP::CanPlayerBuy(bool display)
{
	// is the player alive?
	if (m_pPlayer->pev->deadflag != DEAD_NO)
		return false;

	return !m_pPlayer->m_bIsZombie;
}

std::shared_ptr<CBaseZombieClass_ZB2> CPlayerModStrategy_ZB3ZP::ZombieClassFactory(ZombieLevel lv, const char* name)
{
	auto player = m_pPlayer;

	if (!strcmp(name, "random"))
	{
		switch (RANDOM_LONG(0, 1))
		{
		case 0:
		{
			player->m_ZombieClass = "source";
			return std::make_shared<CZombieClass_Source>(m_pPlayer, lv);
		}
		case 1:
		{
			player->m_ZombieClass = "licker";
			return std::make_shared<CZombieClass_Licker>(m_pPlayer, lv);
		}
		case 2:
		{
			player->m_ZombieClass = "alien_boss";
			return std::make_shared<CZombieClass_Alien_Boss>(m_pPlayer, lv);
		}
		case 3:
		{
			player->m_ZombieClass = "alien_infected";
			return std::make_shared<CZombieClass_Alien_Infected>(m_pPlayer, lv);
		}
		case 4:
		{
			player->m_ZombieClass = "ap";
			return std::make_shared<CZombieClass_AP>(m_pPlayer, lv);
		}
		case 5:
		{
			player->m_ZombieClass = "hostex";
			return std::make_shared<CZombieClass_HostEx>(m_pPlayer, lv);
		}
		case 6:
		{
			player->m_ZombieClass = "humanzombie";
			return std::make_shared<CZombieClass_Human>(m_pPlayer, lv);
		}
		case 7:
		{
			player->m_ZombieClass = "nohead";
			return std::make_shared<CZombieClass_Nohead>(m_pPlayer, lv);
		}
		case 8:
		{
			player->m_ZombieClass = "pyrammid";
			return std::make_shared<CZombieClass_Pyrammid>(m_pPlayer, lv);
		}
		case 9:
		{
			player->m_ZombieClass = "zptank";
			return std::make_shared<CZombieClass_Tank>(m_pPlayer, lv);
		}
		case 10:
		{
			player->m_ZombieClass = "tf";
			return std::make_shared<CZombieClass_TF>(m_pPlayer, lv);
		}
		case 11:
		{
			player->m_ZombieClass = "siren";
			return std::make_shared<CZombieClass_Siren>(m_pPlayer, lv);
		}
		}
	}
	return CPlayerModStrategy_ZB3::ZombieClassFactory(lv, name);
}

void CPlayerModStrategy_ZB3ZP::CmdStart(struct usercmd_s* cmd, unsigned int random_seed)
{
	auto& bitsCurButton = cmd->buttons;
	auto& bitsOldButton = m_pPlayer->pev->oldbuttons;

	if (bitsCurButton & ~bitsOldButton & IN_USE)
	{
		if (m_flNextCheckButton <= gpGlobals->time && m_pPlayer->IsAlive() && m_pPlayer->m_bIsZombie && !m_pPlayer->IsBot())
		{
			if (GetCharacter()->GetSkillStatus(SKILL_SLOT_2) == SKILL_STATUS_USING || GetCharacter()->GetSkillStatus(SKILL_SLOT_1) == SKILL_STATUS_USING)
			{
				ClientPrint(m_pPlayer->pev, HUD_PRINTCENTER, "当前技能使用完之后再使用适应力技能");
				return;
			}

			float flHealth = m_pPlayer->pev->health;
			float flMaxHealth = m_pPlayer->pev->max_health;
			m_flNextCheckButton = gpGlobals->time + 2.0s;
			if (flHealth <= 1000 || m_flNextTimeChangeZombie > gpGlobals->time)
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
			m_pPlayer->pev->health = flHealth - 1000;
			return;
		}
	}

	return CPlayerModStrategy_ZB2::CmdStart(cmd, random_seed);
}

int CPlayerModStrategy_ZB3ZP::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{

	return CPlayerModStrategy_ZB3::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

int CPlayerModStrategy_ZB3ZP::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	int result = CPlayerModStrategy_ZB3::AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);
	if (!player || host == ent)
		return result;

	if (host->v.team != ent->v.team)
		return result;
	if (host->v.flags & FL_SPECTATOR)
		return result;

	state->solid = SOLID_NOT;
	Vector vecDelta = host->v.origin - ent->v.origin;

	if (vecDelta.IsLengthLessThan(70.0f))
	{
		state->rendermode = kRenderTransAlpha;
		state->renderamt = int(vecDelta.Length() * 3);
	}

	return result;
}

void CPlayerModStrategy_ZB3ZP::OnSpawn()
{
	m_pPlayer->pev->playerclass = 2;
	return CPlayerModStrategy_ZB3::OnSpawn();
}

void CPlayerModStrategy_ZB3ZP::Event_TakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType)
{
	if (victim == pevAttacker)
		return;
}

void CPlayerModStrategy_ZB3ZP::Evolution(ZombieLevel iEvolutionLevel)
{
	CPlayerModStrategy_ZB3::Evolution(iEvolutionLevel);

	if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = GetCharacter()->GetZombiMaxHealth(iEvolutionLevel);
		m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
		m_pPlayer->pev->armorvalue = 500.0f;
	}
	else if (iEvolutionLevel == ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = GetCharacter()->GetZombiMaxHealth(iEvolutionLevel);
		m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
		m_pPlayer->pev->armorvalue = 1000.0f;
	}
}

void CPlayerModStrategy_ZB3ZP::OnKilled(entvars_t* pKiller, entvars_t* pInflictor)
{
	CPlayerModStrategy_ZB3::OnKilled(pKiller, pInflictor);
	m_pPlayer->m_bHeadshotKilled = false;
	m_pPlayer->m_LastHitGroup = HITGROUP_GENERIC;
	m_iKilledTime++;
}


void CPlayerModStrategy_ZB3ZP::OnThink()
{
	return CPlayerModStrategy_ZB3::OnThink();
}

void CPlayerModStrategy_ZB3ZP::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	CPlayerModStrategy_ZB3::BecomeZombie(iEvolutionLevel);
	m_pPlayer->pev->max_health = m_pPlayer->pev->health = GetCharacter()->GetZombiMaxHealth(iEvolutionLevel) + m_iKilledTime * 500.0f;

}

void CPlayerModStrategy_ZB3ZP::Event_RestartRound()
{
	m_iKilledTime = 0;
}

}

