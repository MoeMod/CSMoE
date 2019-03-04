/*
mod_gd.cpp - CSMoE Gameplay server : GunDeath
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
#include "globals.h"

#include "mod_gd.h"

void CMod_GunDeath::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_GD);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxkills.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)
	MESSAGE_END();
}

void CMod_GunDeath::InstallPlayerModStrategy(CBasePlayer *player)
{
	class MyPlayerModStrategy : public CPlayerModStrategy_Default
	{
	public:
		explicit MyPlayerModStrategy(CBasePlayer *player) : CPlayerModStrategy_Default(player) {}
		bool CanPlayerBuy(bool display) override { return false; }
	};

	std::unique_ptr<MyPlayerModStrategy> up(new MyPlayerModStrategy(player));
	player->m_pModStrategy = std::move(up);
}