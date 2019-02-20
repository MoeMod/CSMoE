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

CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp)
	: CPlayerModStrategy_ZB2(player, static_cast<CMod_ZombieMod2 *>(mp)),
	  m_pModZB3(mp)
{

}

bool CPlayerModStrategy_ZB3::CanUseZombieSkill()
{
	return true;
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

constexpr auto MORALE_TYPE_GLOBAL = ZB3_MORALE_STRENGTHEN;

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

float CMod_ZombieHero::GetAdjustedEntityDamage(CBaseEntity * victim, entvars_t * pevInflictor, entvars_t * pevAttacker, float flDamage, int bitsDamageType)
{
	flDamage = CMod_ZombieMod2::GetAdjustedEntityDamage(victim, pevInflictor, pevAttacker, flDamage, bitsDamageType);

	flDamage *= m_Morale.DamageModifier(MORALE_TYPE_GLOBAL);

	return flDamage;
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
	auto iNumHeroes = HeroNum();

	moe::range::PlayersList list;
	std::vector<CBasePlayer *> players (list.begin(), list.end());
	players.erase(std::remove_if(players.begin(), players.end(),
				[](CBasePlayer *player){ return player->IsAlive() && player->m_iTeam == TEAM_CT && !player->m_bIsZombie; }),
				players.end());

	// randomize player list
	std::shuffle(players.begin(), players.end(), std::random_device());

	for(int i = 0; i < iNumHeroes; ++i)
	{
		CBasePlayer *hero = players[i];
		MakeHero(hero);
	}
}

size_t CMod_ZombieHero::HeroNum()
{
	moe::range::PlayersList list;
	return std::distance(list.begin(), list.end()) / 10 + RANDOM_LONG(0, 1);
}

void CMod_ZombieHero::MakeHero(CBasePlayer *player)
{
	// TODO : make hero

}
