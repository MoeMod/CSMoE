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

void CMod_ZombieHero::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3(player, this));
}

void CMod_ZombieHero::PickZombieOrigin()
{
	CMod_Zombi::PickZombieOrigin();
	PickHero();
}

void CMod_ZombieHero::PickHero()
{
	int iNumHeroes = HeroNum();

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
