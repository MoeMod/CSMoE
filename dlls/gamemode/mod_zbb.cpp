/*
mob_zbb.cpp - CSMoE Gameplay server : Zombie Mod Base-builder
Copyright (C) 2018 Moemod Hyakuya

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

#include "mod_zbb.h"

CMod_ZombieBaseBuilder::CMod_ZombieBaseBuilder() // precache
{

}

void CMod_ZombieBaseBuilder::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

void CMod_ZombieBaseBuilder::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZBB> up(new CPlayerModStrategy_ZBB(player, this));
	player->m_pModStrategy = std::move(up);
}