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

#include "mod_zb3.h"

#include "util/u_range.hpp"

#include <vector>
#include <algorithm>
#include <random>

constexpr auto MORALE_TYPE_GLOBAL = ZB3_MORALE_DEFAULT;

CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp)
	: CPlayerModStrategy_ZB2(player, static_cast<CMod_ZombieMod2 *>(mp)),
	  m_pModZB3(mp)
{

}

void CPlayerModStrategy_ZB3::OnSpawn()
{
	m_flRagePercent = 0;
	return CPlayerModStrategy_ZB2::OnSpawn();
}

void CPlayerModStrategy_ZB3::CheckEvolution()
{
	float flLastRagePercent = m_flRagePercent;
	auto iLastLevel = m_pPlayer->m_iZombieLevel;
	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST && m_flRagePercent > 100.0f)
	{
		m_pModZB3->MakeZombie(m_pPlayer, ZOMBIE_LEVEL_ORIGIN);
		m_flRagePercent = (flLastRagePercent - 100.0f) * 0.5f;

		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 7000.0f;
		m_pPlayer->pev->armorvalue = 500.0f;
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN && m_flRagePercent > 100.0f)
	{
		m_pModZB3->MakeZombie(m_pPlayer, ZOMBIE_LEVEL_ORIGIN_LV2);
		m_flRagePercent = (flLastRagePercent - 100.0f) * 0.5f;

		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 14000.0f;
		m_pPlayer->pev->armorvalue = 1000.0f;
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		m_flRagePercent = 100.0f;
	}

	UpdatePlayerEvolutionHUD();
}

void CPlayerModStrategy_ZB3::Event_OnInfection(CBasePlayer * victim, CBasePlayer * attacker)
{
	if (m_pPlayer != attacker)
		return;

	// infection => rage
	m_flRagePercent+=m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 40 : 20;
	CheckEvolution();

}

void CPlayerModStrategy_ZB3::UpdatePlayerEvolutionHUD()
{
	MESSAGE_BEGIN(MSG_ONE, gmsgZB3Msg, nullptr, m_pPlayer->edict());
	WRITE_BYTE(ZB3_MESSAGE_RAGE); // type, reserved.
	WRITE_BYTE(m_pPlayer->m_iZombieLevel);
	WRITE_BYTE(static_cast<int>(m_flRagePercent));
	MESSAGE_END();
}

float CPlayerModStrategy_ZB3::AdjustDamageTaken(entvars_t * pevInflictor, entvars_t * pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = CPlayerModStrategy_ZB2::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	// morale calc
	CBasePlayer *pPlayerAttacker = dynamic_ent_cast<CBasePlayer *>(pevAttacker);
	if (pPlayerAttacker && pPlayerAttacker->m_pActiveItem)
	{
		// knife doesn't have extra damage
		if(pPlayerAttacker->m_pActiveItem->m_iId != WEAPON_KNIFE)
			flDamage *= m_pModZB3->HumanMorale().DamageModifier(MORALE_TYPE_GLOBAL);
	}

	// damage => rage
	m_flRagePercent += flDamage * (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 0.01f : 0.005f);
	CheckEvolution();

	return flDamage;
}

void CZB3HumanMorale::UpdateHUD(CBasePlayer *player, ZB3HumanMoraleType_e type) const
{
	if(player)
		MESSAGE_BEGIN(MSG_ONE, gmsgZB3Msg, nullptr, player->edict());
	else
		MESSAGE_BEGIN(MSG_ALL, gmsgZB3Msg);
	WRITE_BYTE(ZB3_MESSAGE_MORALE); // type, reserved.
	WRITE_BYTE(type);
	WRITE_BYTE(GetMoraleLevel());
	MESSAGE_END();
}

CMod_ZombieHero::CMod_ZombieHero()
{

}

void CMod_ZombieHero::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3(player, this));
}

void CMod_ZombieHero::PickZombieOrigin()
{
	CMod_Zombi::PickZombieOrigin();
	PickHero();
}

void CMod_ZombieHero::UpdateGameMode(CBasePlayer * pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZB3);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

void CMod_ZombieHero::RestartRound()
{
	m_Morale.ResetLevel();
	return CMod_ZombieMod2::RestartRound();
}

void CMod_ZombieHero::PlayerSpawn(CBasePlayer * pPlayer)
{
	m_Morale.UpdateHUD(pPlayer, MORALE_TYPE_GLOBAL);
	return CMod_ZombieMod2::PlayerSpawn(pPlayer);
}

void CMod_ZombieHero::PlayerKilled(CBasePlayer * pVictim, entvars_t * pKiller, entvars_t * pInflictor)
{
	if (pVictim->m_bIsZombie)
		if (m_Morale.LevelUp())
			m_Morale.UpdateHUD(nullptr, MORALE_TYPE_GLOBAL);
	return CMod_ZombieMod2::PlayerKilled(pVictim, pKiller, pInflictor);
}

void CMod_ZombieHero::PickHero()
{
	// randomize player list
	moe::range::PlayersList list;
	std::vector<CBasePlayer *> players (list.begin(), list.end());
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer *player) { return !player->IsAlive() || player->m_iTeam != TEAM_CT || player->m_bIsZombie; }), players.end());
	std::shuffle(players.begin(), players.end(), std::random_device());
	
	auto iNumHeroes = std::distance(list.begin(), list.end()) / 10 + std::uniform_int_distribution<size_t>(0, 1)(std::random_device());
	for(int i = 0; i < iNumHeroes; ++i)
	{
		CBasePlayer *hero = players[i];
		MakeHero(hero);
	}
}

void CMod_ZombieHero::MakeHero(CBasePlayer *player)
{
	// TODO : make hero

}
