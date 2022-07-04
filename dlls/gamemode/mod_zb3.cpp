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

#include "mod_zb3.h"
#include "player/player_zombie.h"
#include "player/player_mod_strategy.h"

#include "zb2/zb2_zclass_speed.h"
#include "zb2/zb2_zclass_resident.h"

#include "util/u_range.hpp"

#include "Reciter.h"

#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <memory>
#include "player/csdm_randomspawn.h"


namespace sv {

constexpr auto MORALE_TYPE_GLOBAL = ZB3_MORALE_STRENGTHEN;

CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp)
	:	CPlayerModStrategy_ZB2(player, static_cast<CMod_ZombieMod2 *>(mp)),
		m_pModZB3(mp),
		m_eventBecomeHeroListener(mp->m_eventBecomeHero.subscribe(&CPlayerModStrategy_ZB3::Event_OnBecomeHero, this)),
		m_eventRoundStartListener(mp->m_eventRoundStart.subscribe(&CPlayerModStrategy_ZB3::Event_OnRoundStart, this)),
		m_eventPickAsZombieListener(mp->m_eventPickAsZombie.subscribe(&CPlayerModStrategy_ZB3::Event_OnPickAsZombie, this)),
		m_flRagePercent(0.0f)
{
	
}

void CPlayerModStrategy_ZB3::OnSpawn()
{
	SetHero(false);
	SetPickAsZombie(false);
	m_pModZB3->HumanMorale().UpdateHUD(m_pPlayer, MORALE_TYPE_GLOBAL);
	return CPlayerModStrategy_ZB2::OnSpawn();
}

void CPlayerModStrategy_ZB3::CheckEvolution()
{
	if (!m_pPlayer->m_bIsZombie)
		return;

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST && m_flRagePercent > 100.0f)
	{
		Evolution(ZOMBIE_LEVEL_ORIGIN);
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN && m_flRagePercent > 100.0f)
	{
		Evolution(ZOMBIE_LEVEL_ORIGIN_LV2);
	}

	if (m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_ORIGIN_LV2)
	{
		m_flRagePercent = 100.0f;
	}

	UpdatePlayerEvolutionHUD();
}

void CPlayerModStrategy_ZB3::Evolution(ZombieLevel iEvolutionLevel)
{
	m_flRagePercent = (m_flRagePercent - 100.0f) * 0.5f;
	CPlayerModStrategy_ZB2::Evolution(iEvolutionLevel);
}

void CPlayerModStrategy_ZB3::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	m_flGodMode = gpGlobals->time + 1.5s;
	SetHero(false);
	SetPickAsZombie(false);
	return CPlayerModStrategy_ZB2::BecomeZombie(iEvolutionLevel);
}

void CPlayerModStrategy_ZB3::Event_OnInfection(CBasePlayer * victim, CBasePlayer * attacker)
{
	if (m_pPlayer != attacker)
		return;

	// infection => rage
	if (IsHero())
		m_flRagePercent += m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 100 : 50;
	else
		m_flRagePercent += m_pPlayer->m_iZombieLevel == ZOMBIE_LEVEL_HOST ? 40 : 20;
	//CheckEvolution();
	return CPlayerModStrategy_ZB2::Event_OnInfection(victim, attacker);
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

	if (m_flGodMode > gpGlobals->time)
	{
		flDamage = 0.0F;
	}

	// morale calc
	CBasePlayer *pPlayerAttacker = dynamic_ent_cast<CBasePlayer *>(pevAttacker);
	if (pPlayerAttacker && pPlayerAttacker->m_pActiveItem && !(bitsDamageType & DMG_NOMORALE))
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

void CPlayerModStrategy_ZB3::OnThink()
{
	if (!m_pModZB3->FInfectionStarted() || m_pModZB3->m_bRoundTerminating)
		return;

	if (!m_pPlayer->IsAlive() && m_pPlayer->m_bIsZombie && m_pPlayer->m_iTeam != TEAM_UNASSIGNED && m_pPlayer->m_iTeam != TEAM_SPECTATOR)
	{
		// respawn after 5 secs
		if (!m_pPlayer->m_bHeadshotKilled && gpGlobals->time > m_flDeadTime + 1.0s)
		{
			const float health = max(m_flBackupMaxHealth, 3000);
			const float armor = m_flBackupArmor;

			m_pPlayer->RoundRespawn();		
			m_pPlayer->pev->armorvalue = armor;
			BecomeZombie(m_pPlayer->m_iZombieLevel);	//set health

			if(m_pPlayer->pev->health < health)
				m_pPlayer->pev->max_health = m_pPlayer->pev->health = health;

			m_pPlayer->SetScoreboardAttributes();

			UTIL_BroadcastStuffText("spk \"zombi_comeback\"\n");
		}
	}

	return CPlayerModStrategy_ZB2::OnThink();
}

void CPlayerModStrategy_ZB3::OnKilled(entvars_t * pKiller, entvars_t * pInflictor)
{
	if (m_pModZB3->isForceRespawn())
	{
		m_pPlayer->m_bHeadshotKilled = false;
		m_pPlayer->m_LastHitGroup = HITGROUP_GENERIC;
	}

	// Human should respawn as a zombie
	if (!m_pPlayer->m_bIsZombie)
	{
		m_pPlayer->m_ZombieClass = "random";
		m_pPlayer->m_bHeadshotKilled = false;
		m_pPlayer->m_LastHitGroup = HITGROUP_GENERIC; // fix headshot cannot respawn (Bad Solution)
		m_pPlayer->m_bIsZombie = true;
		m_pPlayer->m_iZombieLevel = ZOMBIE_LEVEL_HOST;
		m_pPlayer->pev->max_health = 3000.0f;
		m_pPlayer->pev->armorvalue = 1000.0f;
		m_flRagePercent = 0;
		m_pModZB3->TeamCheck();
	}

	m_flDeadTime = gpGlobals->time;
	m_flBackupMaxHealth = m_pPlayer->pev->max_health;
	m_flBackupArmor = m_pPlayer->pev->armorvalue;

	// TODO : Respawn effect

	return CPlayerModStrategy_ZB2::OnKilled(pKiller, pInflictor);
}

void CPlayerModStrategy_ZB3::Event_OnBecomeHero(CBasePlayer * who)
{
	if (m_pPlayer != who)
		return;

	SetHero(true);
	m_pPlayer->SetScoreboardAttributes();
	m_pPlayer->pev->max_health = 10000.0f;
	m_pPlayer->pev->health += 2000.0f;
	m_pPlayer->pev->armorvalue = 1000.0f;
	SET_CLIENT_KEY_VALUE(m_pPlayer->entindex(), GET_INFO_BUFFER(m_pPlayer->edict()), "model", m_pPlayer->m_bIsFemale ? "heroine" : "hero");

	DropPrimary(m_pPlayer);
	int iAmount = 0;
	if (m_pPlayer->m_bIsFemale) {
		m_pPlayer->GiveNamedItem("weapon_qbarrel");
		iAmount = m_pPlayer->m_pModStrategy->ComputeMaxAmmo("buckshot", MAX_AMMO_BUCKSHOT);
		m_pPlayer->GiveAmmo(iAmount, "buckshot", iAmount);
	}
	else {
		m_pPlayer->GiveNamedItem("weapon_svdex");
		iAmount = m_pPlayer->m_pModStrategy->ComputeMaxAmmo("762Nato", MAX_AMMO_556NATO);
		m_pPlayer->GiveAmmo(iAmount, "762Nato", iAmount);
	}

	DropSecondary(m_pPlayer);
	m_pPlayer->GiveNamedItem("weapon_deagled");
	iAmount = m_pPlayer->m_pModStrategy->ComputeMaxAmmo("50AE", MAX_AMMO_50AE);
	m_pPlayer->GiveAmmo(iAmount, "50AE", iAmount);

	UTIL_LogPrintfDetail("\"%s<%i><%s><CT>\" triggered \"Became_Hero\"\n", STRING(m_pPlayer->pev->netname), GETPLAYERUSERID(m_pPlayer->edict()), GETPLAYERAUTHID(m_pPlayer->edict()));
	UTIL_ClientPrintAll(HUD_PRINTTALK, "%s 被选中成为英雄！", STRING(m_pPlayer->pev->netname));
	//UTIL_ClientPrintAll(HUD_PRINTTALK, "%s Become a hero！", STRING(m_pPlayer->pev->netname));
}

void CPlayerModStrategy_ZB3::Event_OnRoundStart()
{
	m_flRagePercent = 0;
}

int CPlayerModStrategy_ZB3::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_pPlayer->m_bIsZombie)
		return 1;

	CBaseEntity* pAttacker = GetClassPtr<CBaseEntity>(pevAttacker);
	if (!pAttacker->IsPlayer())
		return 1;

	CBasePlayer* pAttackerPlayer = GetClassPtr<CBasePlayer>(pevAttacker);
	if (!pAttackerPlayer->m_bIsZombie)
		return 1;

	if (m_pPlayer->m_iBuff & BUFF_HOLYSWORD_AVOID_INFECT)
	{
		UTIL_MakeVectors(m_pPlayer->pev->angles);
		if (DotProduct((m_pPlayer->pev->origin.Make2D() - pevAttacker->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) < 0.8 && !(bitsDamageType & DMG_INFECTION))
		{
			CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[KNIFE_SLOT];

			if (pWeapon)
			{
				if (!Q_strcmp(STRING(pWeapon->pev->classname), "knife_holysword") || !Q_strcmp(STRING(pWeapon->pev->classname), "knife_y22s1holyswordmb"))
					pWeapon->WeaponCallBack(0);
			}

			if (bitsDamageType & DMG_BULLET)
			{
				flDamage = 75.0f;

				if (m_pPlayer->pev->health - flDamage >= 300)
				{
					m_pPlayer->pev->health -= flDamage;
				}
				return 0;
			}
			return 1;
		}	
	}

	if (m_pPlayer->m_iBuff & BUFF_AVOID_INFECT)
	{
		return 0;
	}

	if (m_pPlayer->m_iBuff & BUFF_REVIVE && (bitsDamageType & DMG_BULLET))
	{
		CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)m_pPlayer->m_rgpPlayerItems[PRIMARY_WEAPON_SLOT];

		CSDM_DoRandomSpawn(m_pPlayer);
		m_pPlayer->m_iBuff &= ~BUFF_REVIVE;
		m_pPlayer->m_iReviveGunShotsFired = 0;
		if (pWeapon)
		{
			if (!Q_strcmp(STRING(pWeapon->pev->classname), "weapon_revivegun"))
				pWeapon->WeaponCallBack(1);
		}
		return 0;
	}

	if (IsHero() && m_pPlayer->pev->health - flDamage > 0.0f)
		return 1;

	if (m_pPlayer->m_iBuff & BUFF_EMERGENCYESCAPE)
	{
		if (m_pPlayer->pev->health - flDamage <= 0.0f)
			m_pPlayer->pev->health = 1.0f;
		else
			m_pPlayer->pev->health -= flDamage;

		m_pPlayer->m_iBuff |= BUFF_AVOID_INFECT;
		m_pPlayer->pev->takedamage = DAMAGE_NO;
		m_pPlayer->m_tAvoidInfectBuffTime = gpGlobals->time + 2.0s;
		m_pPlayer->m_iBuff &= ~BUFF_EMERGENCYESCAPE;

		m_pPlayer->pev->velocity.z += 1000;

		Vector vecSrc = m_pPlayer->pev->origin;
		Vector vecEnd;
		TraceResult tr;
		CBaseEntity* pEntity = nullptr;
		while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, 220)) != nullptr) {
			if (pEntity == m_pPlayer)
				continue;

			if (pEntity->IsDormant())
				continue;

			if (!pEntity->IsPlayer())
				continue;

			if (g_pGameRules->PlayerRelationship(m_pPlayer, pEntity) == GR_TEAMMATE)
				continue;

			vecEnd = pEntity->BodyTarget(vecSrc);

			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

			if (tr.flFraction < 1.0 && tr.pHit != ENT(pEntity->pev))
				continue;

			Vector vecDir = (pEntity->pev->origin - m_pPlayer->pev->origin);
			vecDir.z = 0;

			CBasePlayer* player = dynamic_ent_cast<CBasePlayer*>(pEntity);
			if (player)
			{
				ApplyKnockbackData(player, player->Center() - vecSrc, { 1500, 1500, 1500, 1500, 1.0f });
				player->pev->velocity.z = +400.0f;
			}
		}
		return 0;
	}

	/*if (~bitsDamageType & DMG_INFECTION)
		return 1;*/

	m_pModZB3->HumanInfectionByZombie(m_pPlayer, pAttackerPlayer);
	return 0;
}

int CPlayerModStrategy_ZB3::AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet)
{
	if (m_flGodMode > gpGlobals->time)
	{
		if (player)
		{
			state->renderfx = kRenderFxGlowShell;
			state->rendercolor = { 255,255,255 };
			state->renderamt = 1;
		}
	}
	return CPlayerModStrategy_ZB2::AddToFullPack_Post(state, e, ent, host, hostflags, player, pSet);
}

void CPlayerModStrategy_ZB3::Event_OnPickAsZombie(CBasePlayer *who)
{
	if (m_pPlayer != who)
		return;

	SetPickAsZombie(true);

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

void CPlayerModStrategy_ZB3::CmdStart(struct usercmd_s* cmd, unsigned int random_seed)
{
	auto& bitsCurButton = cmd->buttons;
	auto& bitsOldButton = m_pPlayer->pev->oldbuttons;

	if (bitsCurButton & ~bitsOldButton & IN_USE && m_pPlayer->IsAlive() && !m_pPlayer->m_bIsZombie && IsPickAsZombie())
	{
		auto iNumZombies = m_pModZB3->ZombieOriginNum();
		auto iNumPlayers = m_pModZB3->m_iNumTerrorist + m_pModZB3->m_iNumCT;

		m_pPlayer->m_ZombieClass = "random";
		m_pPlayer->pev->sequence = 0;
		m_pPlayer->m_iCustomSequence = -1;
		m_pModZB3->MakeZombie(m_pPlayer, ZOMBIE_LEVEL_ORIGIN);
		m_pPlayer->pev->health = m_pPlayer->pev->max_health = 1000.0f * iNumPlayers / iNumZombies + 1000.0f;
		m_pPlayer->pev->armorvalue = 1100;
		m_pModZB3->TeamCheck();

		// sound effect
		m_pModZB3->InfectionSound();
		m_pModZB3->CheckWinConditions();
	}

	return CPlayerModStrategy_ZB2::CmdStart(cmd, random_seed);
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

// pimpl requires individual definition for dtor
CMod_ZombieHero::CMod_ZombieHero() : m_CountDownDelegate(this)
{
	m_flForceRespawnEnd = invalid_time_point;
	m_Countdown.SetDelegate(&m_CountDownDelegate);
}
CMod_ZombieHero::~CMod_ZombieHero() = default;

void CMod_ZombieHero::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3(player, this));
}

void CMod_ZombieHero::StartForceRespawn()
{
	m_flForceRespawnEnd = gpGlobals->time + m_iRoundTime * 0.25;
	char szHudText[64];
	Q_sprintf(szHudText, "病毒散布中，僵尸被致命击杀后依旧能够复活");

	hudtextparms_t textParms;
	textParms.x = -1.0f;
	textParms.y = 0.3f;
	textParms.effect = 0;
	textParms.r1 = 255;
	textParms.g1 = 155;
	textParms.b1 = 0;
	textParms.r2 = 255;
	textParms.g2 = 255;
	textParms.b2 = 255;
	textParms.fadeinTime = 0.1f;
	textParms.fadeoutTime = 1.0f;
	textParms.holdTime = 5.0f;
	textParms.fxTime = 0.1f;
	textParms.channel = 1;
	UTIL_HudMessageAll(textParms, szHudText);
}

void CMod_ZombieHero::PickZombieOrigin()
{
	UTIL_BroadcastStuffText("mp3 loop sound/Zombi_Ambience.mp3\n");
	StartForceRespawn();

	auto iNumZombies = ZombieOriginNum();
	auto iNumPlayers = this->m_iNumTerrorist + this->m_iNumCT;

	// build alive player list
	moe::range::PlayersList list;
	std::vector<CBasePlayer *> players {list.begin(), list.end()};
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer *player) { return !player->IsAlive() || player->m_iTeam != TEAM_CT || player->m_bIsZombie; }), players.end());

	if (players.empty())
		return;

	// randomize player list
	std::shuffle(players.begin(), players.end(), std::random_device());

	// pick them
	for (size_t i = 0; i < iNumZombies; ++i)
	{
		MakePickAsZombie(players[i]);
	}
	CheckWinConditions();
}

void CMod_ZombieHero::MakeZombieOrigin()
{
	auto iNumZombies = ZombieOriginNum();
	auto iNumPlayers = this->m_iNumTerrorist + this->m_iNumCT;

	// build alive player list
	moe::range::PlayersList list;
	std::vector<CBasePlayer*> players{ list.begin(), list.end() };
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer* player) {
		auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(player->m_pModStrategy.get());
		return !(strategy && strategy->IsPickAsZombie() && player->IsAlive() && !player->m_bIsZombie);
	}), players.end());

	if (players.empty())
	{
		if (iNumPlayers)
		{
			PickZombieOrigin();
			MakeZombieOrigin();
		}
		return;
	}

	// pick them
	for (size_t i = 0; i < players.size(); ++i)
	{
		players[i]->m_ZombieClass = "random";
		players[i]->pev->sequence = 0;
		players[i]->m_iCustomSequence = -1;
		MakeZombie(players[i], ZOMBIE_LEVEL_ORIGIN);
		players[i]->pev->health = players[i]->pev->max_health = 1000.0f * iNumPlayers / iNumZombies + 1000.0f;
		players[i]->pev->armorvalue = 1100;
	}
	TeamCheck();

	// sound effect
	InfectionSound();
	CheckWinConditions();
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
	UTIL_BroadcastStuffText("mp3 stop;stopsound\n");
	m_Morale.ResetLevel();
	m_eventRoundStart.dispatch();
	CMod_ZombieMod2::RestartRound();

	MakeReciter();
}

void CMod_ZombieHero::MakeReciter()
{
	m_bHasAnswered = false;
	m_pCurrentReciter = nullptr;
	try {
		m_pCurrentReciter = std::make_unique<Reciter>(GetRandomReciter());
	}
	catch(...) {}

	if(	m_pCurrentReciter )
	{
		char szText[512];
		snprintf(szText, 512, "^3[柑橘CitruS] ^7抢答活动：：^5%s^7", m_pCurrentReciter->question.c_str());
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
		WRITE_BYTE(0);
		WRITE_STRING(szText);
		MESSAGE_END();
	}
}

bool CMod_ZombieHero::Host_Say(edict_t *pEntity, int teamonly, const char *content)
{
	if(m_pCurrentReciter && content == m_pCurrentReciter->answer)
	{
		if(m_bHasAnswered)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSayText, nullptr, pEntity);
			WRITE_BYTE(0);
			WRITE_STRING("^3[柑橘CitruS] ^7已有其他人回答过问题，你手速慢了嗷！^7");
			MESSAGE_END();
		}
		else
		{
			CBaseEntity *pEntity2 = CBaseEntity::Instance(pEntity);
			if(pEntity2->IsPlayer())
			{
				if (RANDOM_LONG(0, 1))
				{
					char szText[256];
					snprintf(szText, 256, "^3[柑橘CitruS] ^7恭喜^2%s^7成功抢答，本局将成为英雄！^7", STRING(pEntity->v.netname));
					MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
					WRITE_BYTE(ENTINDEX(pEntity));
					WRITE_STRING(szText);
					MESSAGE_END();

					CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity2);
					if (!pPlayer->m_bIsZombie)
						MakeHero(pPlayer);
				}
				else
				{
					char szText[256];
					snprintf(szText, 256, "^3[柑橘CitruS] ^7恭喜^2%s^7成功抢答，获得答题武器炽麟煌焱^7", STRING(pEntity->v.netname));
					MESSAGE_BEGIN(MSG_BROADCAST, gmsgSayText);
					WRITE_BYTE(ENTINDEX(pEntity));
					WRITE_STRING(szText);
					MESSAGE_END();
					CBasePlayer* pPlayer = static_cast<CBasePlayer*>(pEntity2);
					if (!pPlayer->m_bIsZombie)
					{

						DropPrimary(pPlayer);
						//pPlayer->RemovePlayerItem(pPlayer->m_rgpPlayerItems[KNIFE_SLOT]);
	
						pPlayer->GiveNamedItem("weapon_wondercannon");
						int iAmount = pPlayer->m_pModStrategy->ComputeMaxAmmo("wondercannon_ammo", 120);
						pPlayer->GiveAmmo(iAmount, "wondercannon_ammo", iAmount);
						

					}
				}

				m_bHasAnswered = true;
			}
		}
	}
	return CMod_ZombieMod2::Host_Say(pEntity, teamonly, content);
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
	std::random_device rd;
	std::vector<CBasePlayer *> players (list.begin(), list.end());
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer *player) {
		auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(player->m_pModStrategy.get());
		return !player->IsAlive() || player->m_iTeam != TEAM_CT || player->m_bIsZombie || (strategy && strategy->IsHero());
	}), players.end());

	if (players.empty())
		return;

	std::shuffle(players.begin(), players.end(), rd);
	// make heroes
	const auto iNumHeroes = std::min(players.size(), players.size() / 10 + std::uniform_int_distribution<size_t>(0, 1)(rd));
	if(iNumHeroes > 0)
		std::for_each(players.begin(), players.begin() + iNumHeroes, std::bind(&CMod_ZombieHero::MakeHero, this, std::placeholders::_1));
}

void CMod_ZombieHero::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
		return;

	if (!FInfectionStarted() || m_bRoundTerminating)
		return;
	
	moe::range::PlayersList list;
	auto iAliveHuman = std::count_if(list.begin(), list.end(), [](CBasePlayer *player) { return player->m_iTeam == TEAM_CT && !player->m_bIsZombie && player->IsAlive() && !player->IsDormant(); });
	auto iAliveZombie = std::count_if(list.begin(), list.end(), [](CBasePlayer *player) {
		auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(player->m_pModStrategy.get());
		return ((player->m_iTeam == TEAM_TERRORIST && player->m_bIsZombie) || (strategy && strategy->IsPickAsZombie())) && !(!player->IsAlive() && player->m_bHeadshotKilled) && !player->IsDormant();
	});

	//ALERT(at_console, "CheckWinConditions: iAliveHuman:%d iAliveZombie:%d\n", iAliveHuman, iAliveZombie);

	if (!iAliveHuman)
		ZombieWin();
	else if (!iAliveZombie)
		HumanWin();
}

BOOL CMod_ZombieHero::FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker)
{
	int iReturn = FALSE;

	if (!FInfectionStarted() || m_bRoundTerminating)
		return FALSE;

	if (!pAttacker || PlayerRelationship(pPlayer, pAttacker) != GR_TEAMMATE)
	{
		iReturn = TRUE;
	}

	if (CVAR_GET_FLOAT("mp_friendlyfire") != 0 || pAttacker == pPlayer)
	{
		iReturn = TRUE;
	}

	return iReturn;
}

void CMod_ZombieHero::Think()
{
	/*if (m_flCheckWinConditions <= gpGlobals->time)
	{
		m_flCheckWinConditions = gpGlobals->time + 5s;
		CheckWinConditions();
	}*/
	if (m_flForceRespawnEnd != invalid_time_point && m_flForceRespawnEnd <= gpGlobals->time)
	{
		m_flForceRespawnEnd = invalid_time_point;
		char szHudText[64];
		Q_sprintf(szHudText, "病毒已驱散，僵尸被致命击败后无法再次复活");

		hudtextparms_t textParms;
		textParms.x = -1.0f;
		textParms.y = 0.3f;
		textParms.effect = 0;
		textParms.r1 = 255;
		textParms.g1 = 155;
		textParms.b1 = 0;
		textParms.r2 = 255;
		textParms.g2 = 255;
		textParms.b2 = 255;
		textParms.fadeinTime = 0.1f;
		textParms.fadeoutTime = 1.0f;
		textParms.holdTime = 5.0f;
		textParms.fxTime = 0.1f;
		textParms.channel = 1;
		UTIL_HudMessageAll(textParms, szHudText);
	}
	return CMod_ZombieMod2::Think();
}

int CMod_ZombieHero::IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled)
{
	if (!pAttacker->m_bIsZombie && pKilled->m_bIsZombie)
	{
		if (pKilled->m_bHeadshotKilled)
			return 3;

		return 0;
	}

	return 1;
}

BOOL CMod_ZombieHero::FInfectionStarted()
{
	return !m_Countdown.IsFreezePeriod() && (m_Countdown.GetCountdown() <= 7 || m_Countdown.IsExpired());
}

CZB3CountdownDelegate::CZB3CountdownDelegate(CMod_ZombieHero *mod) : CZB1CountdownDelegate(mod) {}

void CZB3CountdownDelegate::OnCountdownStart()
{
	UTIL_BroadcastStuffText("spk \"zombi_start\"\n");
}

void CZB3CountdownDelegate::OnCountdownChanged(int iCurrentCount)
{
	static const char* s_szCountDownSound[11] = {
		"", "one", "two", "three", "four", "five", "six",
		"seven", "eight", "nine", "ten"
	};

	if (iCurrentCount > 7)
	{
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "[😷]生化病毒正在寻找宿主， %s 秒后选定感染目标", UTIL_dtos1(iCurrentCount - 7));

		if (iCurrentCount - 7 <= 10)
		{
			UTIL_BroadcastStuffText("spk \"%s\"\n", s_szCountDownSound[iCurrentCount - 7]);
		}
	}
	else if (iCurrentCount <= 7)
	{
		if (iCurrentCount == 7)
			m_pMod->PickZombieOrigin();

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "[💉]生化病毒已找到宿主，母体僵尸将在 %s 秒内出现", UTIL_dtos1(iCurrentCount));
	}
}

void CZB3CountdownDelegate::OnCountdownEnd()
{
	// select zombie
	((CMod_ZombieHero*)m_pMod)->MakeZombieOrigin();
}

void CMod_ZB3F2P::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3F2P(player, this));
}

void CMod_ZB3F2P::W_Precache()
{
    UTIL_PrecacheOtherWeapon("weapon_cannon");
    UTIL_PrecacheOther("ammo_cannon");
    UTIL_PrecacheOtherWeapon("z4b_cannontiger");
    UTIL_PrecacheOtherWeapon("z4b_cannonplasma");
    UTIL_PrecacheOtherWeapon("weapon_infinityex2");
    UTIL_PrecacheOtherWeapon("z4b_infinityx");
    UTIL_PrecacheOtherWeapon("knife_skullaxe");
    UTIL_PrecacheOtherWeapon("knife_skullt9");
    UTIL_PrecacheOther("ammo_skullt9");
    UTIL_PrecacheOtherWeapon("weapon_chainsaw");
    UTIL_PrecacheOther("ammo_chainsaw");
    UTIL_PrecacheOtherWeapon("knife_dragonsword");
}

void CPlayerModStrategy_ZB3F2P::GiveDefaultItems()
{
	if (!m_pPlayer->m_bIsZombie)
	{
		m_pPlayer->RemoveAllItems(FALSE);
		m_pPlayer->m_bHasPrimary = false;
		if(!RANDOM_LONG(0, 10))
		{
			// 电锯套 10%
			m_pPlayer->GiveNamedItem("weapon_chainsaw");
			m_pPlayer->GiveNamedItem("weapon_infinityex2");
			m_pPlayer->GiveNamedItem("knife_dragonsword");
			m_pPlayer->GiveNamedItem("weapon_hegrenade");
		}
		else
		{
			// 龙炮套 90%
			if(!RANDOM_LONG(0, 5))
				m_pPlayer->GiveNamedItem("weapon_cannonex");
			else
				m_pPlayer->GiveNamedItem("weapon_cannon");

			if(!RANDOM_LONG(0, 5))
				m_pPlayer->GiveNamedItem("z4b_infinityx");
			else
				m_pPlayer->GiveNamedItem("weapon_infinityex2");

			if(!RANDOM_LONG(0, 5))
				m_pPlayer->GiveNamedItem("knife_skullt9");
			else
				m_pPlayer->GiveNamedItem("knife_skullaxe");

			
			m_pPlayer->GiveNamedItem("weapon_hegrenade");
		}
		for (auto iSlot : { PRIMARY_WEAPON_SLOT, PISTOL_SLOT, KNIFE_SLOT })
			GiveSlotAmmo(m_pPlayer, iSlot);
		return;
	}
	return CPlayerModStrategy_ZB3::GiveDefaultItems();
}

std::shared_ptr<CBaseZombieClass_ZB2> CPlayerModStrategy_ZB3F2P::ZombieClassFactory(ZombieLevel lv, const char *name)
{
	if(!strcmp(name, "random"))
	{
		if(!RANDOM_LONG(0, 3))
		{
			// 长手 25%
			return std::make_shared<CZombieClass_Resident>(m_pPlayer, lv);
		}
		else
		{
			// 芭比 75%
			return std::make_shared<CZombieClass_Speed>(m_pPlayer, lv);
		}
	}
	return CPlayerModStrategy_ZB3::ZombieClassFactory(lv, name);
}

}
