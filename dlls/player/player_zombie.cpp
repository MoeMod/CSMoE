/*
player_zombie.cpp - CSMoE Gameplay server : CBasePlayer impl for zombies
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
#include "gamerules.h"
#include "client.h"

#include "gamemode/mods.h"
#include "gamemode/zb2/zb2_const.h"

void CBasePlayer::MakeZombie(ZombieLevel iEvolutionLevel)
{
	m_bIsZombie = true;
	m_bNotKilled = false;
	m_iZombieLevel = iEvolutionLevel;

	pev->body = 0;
	m_iModelName = iEvolutionLevel ? MODEL_ZOMBIE_ORIGIN : MODEL_ZOMBIE_HOST;

	const char *szModel = iEvolutionLevel ? "zombi_origin" : "zombi_host";
	SET_CLIENT_KEY_VALUE(entindex(), GET_INFO_BUFFER(edict()), "model", const_cast<char *>(szModel));

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	SetNewPlayerModel(szModelPath);


	UTIL_LogPrintf("\"%s<%i><%s><CT>\" triggered \"Became_ZOMBIE\"\n", STRING(pev->netname), GETPLAYERUSERID(edict()), GETPLAYERAUTHID(edict()));

	// remove guns & give nvg
	GiveDefaultItems();
	m_bNightVisionOn = false;
	ClientCommand("nightvision");

	// set default property
	pev->health = pev->max_health = 2000;
	pev->armortype = ARMOR_TYPE_HELMET;
	pev->armorvalue = 200;
	pev->gravity = 0.83f;
	ResetMaxSpeed();

}

void PlayerZombie_Precache()
{
	PRECACHE_SOUND("zombi/zombi_death_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");

	PRECACHE_SOUND("zombi/zombi_heal.wav");
}