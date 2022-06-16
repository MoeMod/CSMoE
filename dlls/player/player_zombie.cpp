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
	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity = 0.8f;
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

	const char *szModel = iEvolutionLevel ? "zombi_origin" : "zombi_host";
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", const_cast<char *>(szModel));

	static char szModelPath[64];
	Q_snprintf(szModelPath, sizeof(szModelPath), "models/player/%s/%s.mdl", szModel, szModel);
	m_pPlayer->SetNewPlayerModel(szModelPath);

	UTIL_LogPrintfDetail("\"%s<%i><%s><CT>\" triggered \"Became_ZOMBIE\"\n", STRING(m_pPlayer->pev->netname), GETPLAYERUSERID(m_pPlayer->edict()), GETPLAYERAUTHID(m_pPlayer->edict()));

	// remove guns & give nvg
	m_pPlayer->GiveDefaultItems();
	m_pPlayer->m_bNightVisionOn = false;
	//m_pPlayer->ClientCommand("nightvision");

	// set default property
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 2000;
	m_pPlayer->pev->armortype = ARMOR_TYPE_HELMET;
	m_pPlayer->pev->armorvalue = 200;
	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity = 0.83f;
	m_pPlayer->ResetMaxSpeed();

}

void CZombie_ZB1::ResetMaxSpeed() const
{
	m_pPlayer->pev->maxspeed = 290;
	m_pPlayer->m_flDefaultGravity = m_pPlayer->pev->gravity = 0.8f;

	if (m_tStunGravityTime > gpGlobals->time)
	{
		m_pPlayer->pev->gravity = 1.0f;
	}

	if (m_tStunSpeedTime > gpGlobals->time)
	{
		m_pPlayer->pev->maxspeed = 100.0f;
	}
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
	PRECACHE_SOUND("zombi/zombi_death_teleport_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_teleport_2.wav");
	PRECACHE_SOUND("zombi/boosterzombie_death1.wav");
	PRECACHE_SOUND("zombi/boosterzombie_death2.wav");
	PRECACHE_SOUND("zombi/zombi_death_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_2.wav");
	PRECACHE_SOUND("zombi/zombi_chinese_death.wav");
	PRECACHE_SOUND("zombi/passzombie_death1.wav");
	PRECACHE_SOUND("zombi/passzombie_death2.wav");
	PRECACHE_SOUND("zombi/flyzombie_death1.wav");
	PRECACHE_SOUND("zombi/flyzombie_death2.wav");
	PRECACHE_SOUND("zombi/zombi_revival_death1.wav");
	PRECACHE_SOUND("zombi/zombi_revival_death2.wav");
	PRECACHE_SOUND("zombi/zombi_death_stamper_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_stamper_2.wav");
	PRECACHE_SOUND("zombi/resident_death.wav");
	PRECACHE_SOUND("zombi/zombi_death_banshee_1.wav");
	PRECACHE_SOUND("zombi/zombi_death_banshee_2.wav");
	PRECACHE_SOUND("zombi/zombie_die4.wav");
	PRECACHE_SOUND("zombi/zombie_die3.wav");
	PRECACHE_SOUND("zombi/akshazombie_death1.wav");
	PRECACHE_SOUND("zombi/akshazombie_death2.wav");
	PRECACHE_SOUND("zombi/boomer_death.wav");
	PRECACHE_SOUND("zombi/spider_death1.wav");
	PRECACHE_SOUND("zombi/spider_death2.wav");
	PRECACHE_SOUND("zombi/deathknight_death1.wav");
	PRECACHE_SOUND("zombi/deathknight_death2.wav");
	PRECACHE_SOUND("zombi/meatwallzombie_death1.wav");
	PRECACHE_SOUND("zombi/meatwallzombie_death2.wav");

	PRECACHE_SOUND("zombi/zombi_hurt_01.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_02.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_female_1.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_female_2.wav");
	PRECACHE_SOUND("zombi/boosterzombie_hurt1.wav");
	PRECACHE_SOUND("zombi/boosterzombie_hurt2.wav");
	PRECACHE_SOUND("zombi/zombi_chinese_hurt.wav");
	PRECACHE_SOUND("zombi/passzombie_hurt1.wav");
	PRECACHE_SOUND("zombi/passzombie_hurt2.wav");
	PRECACHE_SOUND("zombi/flyzombie_hurt1.wav");
	PRECACHE_SOUND("zombi/flyzombie_hurt2.wav");
	PRECACHE_SOUND("zombi/zombi_revival_hurt.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_stamper_1.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_stamper_2.wav");
	PRECACHE_SOUND("zombi/resident_hurt1.wav");
	PRECACHE_SOUND("zombi/resident_hurt2.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_banshee_1.wav");
	PRECACHE_SOUND("zombi/zombi_hurt_banshee_2.wav");
	PRECACHE_SOUND("zombi/nemesis_pain1.wav");
	PRECACHE_SOUND("zombi/nemesis_pain2.wav");
	PRECACHE_SOUND("zombi/akshazombie_hurt1.wav");
	PRECACHE_SOUND("zombi/akshazombie_hurt2.wav");
	PRECACHE_SOUND("zombi/boomer_hurt1.wav");
	PRECACHE_SOUND("zombi/boomer_hurt2.wav");
	PRECACHE_SOUND("zombi/spider_hurt1.wav");
	PRECACHE_SOUND("zombi/spider_hurt2.wav");
	PRECACHE_SOUND("zombi/deathknight_hurt1.wav");
	PRECACHE_SOUND("zombi/deathknight_hurt2.wav");
	PRECACHE_SOUND("zombi/meatwallzombie_hurt1.wav");
	PRECACHE_SOUND("zombi/meatwallzombie_hurt2.wav");

	PRECACHE_SOUND("zombi/zombi_heal.wav");
	PRECACHE_SOUND("zombi/zombi_heal_female.wav");
	PRECACHE_SOUND("zombi/zombi_heal_teleport.wav");
	PRECACHE_SOUND("zombi/zombi_heal_revival.wav");

	PRECACHE_MODEL("models/v_knife_zombis.mdl");
	PRECACHE_MODEL("models/v_knife_zombideimos2.mdl");
	PRECACHE_MODEL("models/v_knife_zombideimos.mdl");
	PRECACHE_MODEL("models/v_knife_zombideimos_host.mdl");
	PRECACHE_MODEL("models/v_knife_zombih.mdl");
	PRECACHE_MODEL("models/v_knife_zombihealer.mdl");
	PRECACHE_MODEL("models/v_knife_zombipc.mdl");
	PRECACHE_MODEL("models/v_knife_teleport_zombi.mdl");
	PRECACHE_MODEL("models/v_knife_teleport_zombi_host.mdl");
	PRECACHE_MODEL("models/v_knife_booster_zombi.mdl");
	PRECACHE_MODEL("models/v_knife_booster_zombi_host.mdl");
	PRECACHE_MODEL("models/v_knife_zombichina.mdl");
	PRECACHE_MODEL("models/v_knife_pass_zombi.mdl");
	PRECACHE_MODEL("models/v_knife_pass_zombi_host.mdl");
	PRECACHE_MODEL("models/v_knife_pass_zombi.mdl");
	PRECACHE_MODEL("models/v_knife_flyingzombi_host.mdl");
	PRECACHE_MODEL("models/v_knife_flyingzombi.mdl");
	PRECACHE_MODEL("models/v_knife_zombirevival_host.mdl");
	PRECACHE_MODEL("models/v_knife_zombirevival.mdl");
	PRECACHE_MODEL("models/v_knife_zombiundertaker.mdl");
	PRECACHE_MODEL("models/v_knife_zombieresident.mdl");
	PRECACHE_MODEL("models/v_knife_zombiewitch.mdl");
	PRECACHE_MODEL("models/v_knife_zombiaksha_host.mdl");
	PRECACHE_MODEL("models/v_knife_zombiaksha.mdl");
	PRECACHE_MODEL("models/v_knife_zombiboomer.mdl");
	PRECACHE_MODEL("models/v_knife_zombinemesis.mdl");
	PRECACHE_MODEL("models/v_knife_zombispider.mdl");
	PRECACHE_MODEL("models/v_knife_zombideathknight.mdl");
	PRECACHE_MODEL("models/v_knife_zombimeatwall.mdl");
	PRECACHE_MODEL("models/v_knife_zombisiren.mdl");

	PRECACHE_MODEL("models/v_zombibomb-deimos2.mdl");
	PRECACHE_MODEL("models/v_zombibomb-deimos.mdl");
	PRECACHE_MODEL("models/v_zombibomb-deimos_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb-s.mdl");
	PRECACHE_MODEL("models/v_zombibombpc.mdl");
	PRECACHE_MODEL("models/v_zombibomb-heal.mdl");
	PRECACHE_MODEL("models/v_zombibomb-h.mdl");
	PRECACHE_MODEL("models/v_zombibomb_teleport_zombi.mdl");
	PRECACHE_MODEL("models/v_zombibomb_teleport_zombi_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_booster_zombi.mdl");
	PRECACHE_MODEL("models/v_zombibomb_booster_zombi_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_china.mdl");	
	PRECACHE_MODEL("models/v_zombibomb_pass_zombi.mdl");
	PRECACHE_MODEL("models/v_zombibomb_pass_zombi_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_flyingzombi.mdl");
	PRECACHE_MODEL("models/v_zombibomb_flyingzombi_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_revival.mdl");
	PRECACHE_MODEL("models/v_zombibomb_revival_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_undertaker.mdl");
	PRECACHE_MODEL("models/v_zombibomb_resident.mdl");
	PRECACHE_MODEL("models/v_zombibomb_witch.mdl");
	PRECACHE_MODEL("models/v_zombibomb_aksha_host.mdl");
	PRECACHE_MODEL("models/v_zombibomb_aksha.mdl");
	PRECACHE_MODEL("models/v_zombibomb_boomer.mdl");
	PRECACHE_MODEL("models/v_zombibomb_spider.mdl");
	PRECACHE_MODEL("models/v_zombibomb_deathknight.mdl");
	PRECACHE_MODEL("models/v_zombibomb_meatwall.mdl");

	PRECACHE_MODEL("models/@frogvari_v_zombibomb-deimos2.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb-deimos.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb-deimos_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb-s.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibombpc.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb-heal.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb-h.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_teleport_zombi.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_teleport_zombi_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_booster_zombi.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_booster_zombi_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_china.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_pass_zombi.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_pass_zombi_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_flyingzombi.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_flyingzombi_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_revival.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_revival_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_undertaker.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_resident.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_witch.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_aksha_host.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_aksha.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_boomer.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_spider.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_deathknight.mdl");
	PRECACHE_MODEL("models/@frogvari_v_zombibomb_meatwall.mdl");

	PRECACHE_MODEL("models/player/zombi_origin/zombi_origin.mdl");
	PRECACHE_MODEL("models/player/zombi_host/zombi_host.mdl");
	PRECACHE_MODEL("models/player/speed_zombi_origin/speed_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/speed_zombi_host/speed_zombi_host.mdl");
	PRECACHE_MODEL("models/player/deimos_zombi_host/deimos_zombi_host.mdl");
	PRECACHE_MODEL("models/player/deimos2_zombi_origin/deimos2_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/deimos_zombi_origin/deimos_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/heavy_zombi_host/heavy_zombi_host.mdl");
	PRECACHE_MODEL("models/player/heavy_zombi_origin/heavy_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/pc_zombi_origin/pc_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/pc_zombi_host/pc_zombi_host.mdl");
	PRECACHE_MODEL("models/player/heal_zombi_host/heal_zombi_host.mdl");
	PRECACHE_MODEL("models/player/heal_zombi_origin/heal_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/teleport_zombi_host/teleport_zombi_host.mdl");
	PRECACHE_MODEL("models/player/teleport_zombi_origin/teleport_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/booster_zombi_host/booster_zombi_host.mdl");
	PRECACHE_MODEL("models/player/booster_zombi_origin/booster_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/china_zombi_host/china_zombi_host.mdl");
	PRECACHE_MODEL("models/player/china_zombi_origin/china_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/china_zombi_host/china_zombi_host.mdl");
	PRECACHE_MODEL("models/player/china_zombi_origin/china_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/pass_zombi_origin/pass_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/pass_zombi_host/pass_zombi_host.mdl");
	PRECACHE_MODEL("models/player/flyingzombi_origin/flyingzombi_origin.mdl");
	PRECACHE_MODEL("models/player/flyingzombi_host/flyingzombi_host.mdl");
	PRECACHE_MODEL("models/player/revival_zombi_origin/revival_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/revival_zombi_host/revival_zombi_host.mdl");
	PRECACHE_MODEL("models/player/stamper_zombi_origin/stamper_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/stamper_zombi_host/stamper_zombi_host.mdl");
	PRECACHE_MODEL("models/player/resident_zombi_origin/resident_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/resident_zombi_host/resident_zombi_host.mdl");
	PRECACHE_MODEL("models/player/witch_zombi_origin/witch_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/witch_zombi_host/witch_zombi_host.mdl");
	PRECACHE_MODEL("models/player/boomer_zombi_origin/boomer_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/boomer_zombi_host/boomer_zombi_host.mdl");
	PRECACHE_MODEL("models/player/witch_zombi_origin/witch_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/witch_zombi_host/witch_zombi_host.mdl");
	PRECACHE_MODEL("models/player/csmoe_nemesis/csmoe_nemesis.mdl");
	PRECACHE_MODEL("models/player/akshazombi_host/akshazombi_host.mdl");
	PRECACHE_MODEL("models/player/akshazombi_origin/akshazombi_origin.mdl");
	PRECACHE_MODEL("models/player/boomer_zombi_host/boomer_zombi_host.mdl");
	PRECACHE_MODEL("models/player/boomer_zombi_origin/boomer_zombi_origin.mdl");
	PRECACHE_MODEL("models/player/zbs_deimos2_zombi/zbs_deimos2_zombi.mdl");
	PRECACHE_MODEL("models/player/zombi_spider/zombi_spider.mdl");
	PRECACHE_MODEL("models/player/zombi_deathknight/zombi_deathknight.mdl");
	PRECACHE_MODEL("models/player/zombi_meatwall/zombi_meatwall.mdl");
	PRECACHE_MODEL("models/player/zombi_siren/zombi_siren.mdl");
}

}
