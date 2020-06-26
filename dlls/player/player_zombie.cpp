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
#include "gamemode/zb1/zb1_zclass.h"

namespace sv {

CHuman_ZB1::CHuman_ZB1(CBasePlayer *player) : BasePlayerExtra(player)
{
	m_pPlayer->m_bIsZombie = false;
	// Give Armor
	m_pPlayer->pev->health = m_pPlayer->pev->max_health= 1000;
	//pPlayer->pev->gravity = 0.86f;
	m_pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 100;
}

//void CBasePlayer::MakeZombie(ZombieLevel iEvolutionLevel)
CZombie_ZB1::CZombie_ZB1(CBasePlayer *player, ZombieLevel iEvolutionLevel) : BasePlayerExtra(player)
{
	m_pPlayer->m_bIsZombie = true;
	m_pPlayer->m_bNotKilled = false;
	m_pPlayer->m_iZombieLevel = iEvolutionLevel;

	m_pPlayer->pev->body = 0;
	m_pPlayer->m_iModelName = iEvolutionLevel ? MODEL_ZOMBIE_ORIGIN : MODEL_ZOMBIE_HOST;

	const char *szModel = iEvolutionLevel ? "zombi_origin" : "zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", const_cast<char *>(szModel));

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);

	UTIL_LogPrintf("\"%s<%i><%s><CT>\" triggered \"Became_ZOMBIE\"\n", STRING(m_pPlayer->pev->netname), GETPLAYERUSERID(m_pPlayer->edict()), GETPLAYERAUTHID(m_pPlayer->edict()));

	// remove guns & give nvg
	m_pPlayer->GiveDefaultItems();
	m_pPlayer->m_bNightVisionOn = false;
	m_pPlayer->ClientCommand("nightvision");

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->pev->gravity = 0.83f;
	m_pPlayer->ResetMaxSpeed();

}

void CZombie_ZB1::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 290;
}

void CZombie_ZB1::DeathSound_Zombie()
{
	// temporarily using pain sounds for death sounds
	switch (RANDOM_LONG(1, 2))
	{
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_death_2.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void CZombie_ZB1::Pain_Zombie(int m_LastHitGroup, bool HasArmour)
{
	switch (RANDOM_LONG(0, 1))
	{
		case 0: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_01.wav", VOL_NORM, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_VOICE, "zombi/zombi_hurt_02.wav", VOL_NORM, ATTN_NORM); break;
		default:break;
	}
}

void PlayerZombie_Precache()
{
	PRECACHE_SOUND("zombi/zombi_death_female_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_female_2.wav");
	PRECACHE_SOUND("zombi/zombi_death_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_female_1.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_female_2.wav");

	PRECACHE_SOUND("zombi/zombi_heal.wav");

	PRECACHE_MODEL("models/v_knife_zombi.mdl");
	PRECACHE_MODEL("models/v_knife_zombis.mdl");
	PRECACHE_MODEL("models/v_knife_zombideimos2.mdl");
	PRECACHE_MODEL("models/v_knife_zombideimos_host.mdl");

	PRECACHE_MODEL("models/v_zombibomb-deimos2.mdl");
	PRECACHE_MODEL("models/v_zombibomb-deimos_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb-s.mdl");

	PRECACHE_MODEL("models/player/zombi_origin/zombi_origin.mdl");
	PRECACHE_MODEL("models/player/zombi_host/zombi_host.mdl");
	PRECACHE_MODEL("models/player/speed_zombi_origin/speed_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/speed_zombi_host/speed_zombi_host.mdl");
	PRECACHE_MODEL("models/player/deimos_zombi_host/deimos_zombi_host.mdl");
	PRECACHE_MODEL("models/player/deimos2_zombi_origin/deimos2_zombi_origin.mdl");
}

}
