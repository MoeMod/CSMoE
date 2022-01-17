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

#include "mod_zb3.h"
#include "player/player_zombie.h"
#include "player/player_mod_strategy.h"

#include "zb2/zb2_zclass_speed.h"
#include "zb2/zb2_zclass_resident.h"

#include "util/u_range.hpp"

#include <vector>
#include <algorithm>
#include <functional>
#include <random>
#include <memory>

namespace sv {

constexpr auto MORALE_TYPE_GLOBAL = ZB3_MORALE_STRENGTHEN;

CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp)
	:	CPlayerModStrategy_ZB2(player, static_cast<CMod_ZombieMod2 *>(mp)),
		m_pModZB3(mp),
		m_eventBecomeHeroListener(mp->m_eventBecomeHero.subscribe(&CPlayerModStrategy_ZB3::Event_OnBecomeHero, this)),
		m_eventRoundStartListener(mp->m_eventRoundStart.subscribe(&CPlayerModStrategy_ZB3::Event_OnRoundStart, this)),
		m_flRagePercent(0.0f)
{
	
}

void CPlayerModStrategy_ZB3::OnSpawn()
{
	SetHero(false);
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
	m_flGodMode = gpGlobals->time + 1s;
	SetHero(false);
	return CPlayerModStrategy_ZB2::BecomeZombie(iEvolutionLevel);
}

void CPlayerModStrategy_ZB3::Event_OnInfection(CBasePlayer * victim, CBasePlayer * attacker)
{
	if (m_pPlayer != attacker)
		return;

	// infection => rage
	auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(victim->m_pModStrategy.get());
	if (strategy && strategy->IsHero())
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

void CPlayerModStrategy_ZB3::OnThink()
{
	if (!m_pModZB3->FInfectionStarted() || m_pModZB3->m_bRoundTerminating)
		return;

	if (!m_pPlayer->IsAlive() && m_pPlayer->m_bIsZombie && m_pPlayer->m_iTeam != TEAM_UNASSIGNED && m_pPlayer->m_iTeam != TEAM_SPECTATOR)
	{
		// respawn after 5 secs
		if (!m_pPlayer->m_bHeadshotKilled && gpGlobals->time > m_flDeadTime + 1.0s)
		{
			const float health = max(m_flBackupMaxHealth, 1000);
			const float armor = m_flBackupArmor;

			m_pPlayer->RoundRespawn();
			BecomeZombie(m_pPlayer->m_iZombieLevel);

			m_pPlayer->pev->max_health = m_pPlayer->pev->health = health;
			m_pPlayer->pev->armorvalue = armor;
			m_pPlayer->SetScoreboardAttributes();

			UTIL_BroadcastStuffText("spk \"zombi_comeback\"\n");
		}
	}

	return CPlayerModStrategy_ZB2::OnThink();
}

void CPlayerModStrategy_ZB3::OnKilled(entvars_t * pKiller, entvars_t * pInflictor)
{
	// Human should respawn as a zombie
	if (!m_pPlayer->m_bIsZombie)
	{
		m_pPlayer->m_ZombieClass = "random";
		m_pPlayer->m_bHeadshotKilled = false;
		m_pPlayer->m_LastHitGroup = HITGROUP_GENERIC; // fix headshot cannot respawn (Bad Solution)
		m_pPlayer->m_bIsZombie = true;
		m_pPlayer->pev->max_health = 2000.0f;
		m_pPlayer->pev->armorvalue = 200.0f;
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
	m_pPlayer->pev->health = m_pPlayer->pev->max_health = 3000.0f;
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
		iAmount = m_pPlayer->m_pModStrategy->ComputeMaxAmmo("556Nato", MAX_AMMO_556NATO);
		m_pPlayer->GiveAmmo(iAmount, "556Nato", iAmount);
	}

	DropSecondary(m_pPlayer);
	m_pPlayer->GiveNamedItem("weapon_deagled");
	iAmount = m_pPlayer->m_pModStrategy->ComputeMaxAmmo("50AE", MAX_AMMO_50AE);
	m_pPlayer->GiveAmmo(iAmount, "50AE", iAmount);

	UTIL_LogPrintf("\"%s<%i><%s><CT>\" triggered \"Became_Hero\"\n", STRING(m_pPlayer->pev->netname), GETPLAYERUSERID(m_pPlayer->edict()), GETPLAYERAUTHID(m_pPlayer->edict()));
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

	if (m_pPlayer->iuser1 == 1 && m_pPlayer->m_pActiveItem && !Q_strcmp(STRING(m_pPlayer->m_pActiveItem->pev->classname), "knife_dgaxe") && m_pPlayer->pev->health - flDamage > 0.0f)
		return 1;

	if (IsHero() && m_pPlayer->pev->health - flDamage > 0.0f)
		return 1;

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
	m_eventRoundStart.dispatch();
	return CMod_ZombieMod2::RestartRound();
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
	players.erase(std::remove_if(players.begin(), players.end(), [](CBasePlayer *player) { return !player->IsAlive() || player->m_iTeam != TEAM_CT || player->m_bIsZombie; }), players.end());
	std::shuffle(players.begin(), players.end(), rd);
	// make heroes
	const auto iNumHeroes = std::min(players.size(), players.size() / 10 + std::uniform_int_distribution<size_t>(0, 1)(rd));
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
	auto iAliveZombie = std::count_if(list.begin(), list.end(), [](CBasePlayer *player) { return player->m_iTeam == TEAM_TERRORIST && player->m_bIsZombie && !(!player->IsAlive() && player->m_bHeadshotKilled) && !player->IsDormant(); });

	ALERT(at_console, "CheckWinConditions: iAliveHuman:%d iAliveZombie:%d\n", iAliveHuman, iAliveZombie);

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

	/*CBasePlayer* pAttackerPlayer = dynamic_ent_cast<CBasePlayer*>(pAttacker);
	do
	{
		if (!pAttackerPlayer) break;
		if (!pAttackerPlayer->m_bIsZombie || pPlayer->m_bIsZombie) break;
		if (pPlayer->iuser1 == 1 && pPlayer->m_pActiveItem && !Q_strcmp(STRING(pPlayer->m_pActiveItem->pev->classname), "knife_dgaxe"))
		{
			iReturn = FALSE;
			break;
		}

		auto strategy = dynamic_cast<CPlayerModStrategy_ZB3*>(pPlayer->m_pModStrategy.get());
		if (strategy && strategy->IsHero())
		{
			iReturn = TRUE;
			break;
		}
		iReturn = FALSE;
		HumanInfectionByZombie(pPlayer, pAttackerPlayer);
	} while (false);*/

	return iReturn;
}

void CMod_ZombieHero::Think()
{
	if (m_flCheckWinConditions <= gpGlobals->time)
	{
		m_flCheckWinConditions = gpGlobals->time + 5s;
		CheckWinConditions();
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

void CMod_ZB3F2P::InstallPlayerModStrategy(CBasePlayer *player)
{
	player->m_pModStrategy.reset(new CPlayerModStrategy_ZB3F2P(player, this));
}

void CPlayerModStrategy_ZB3F2P::GiveDefaultItems()
{
	if (!m_pPlayer->m_bIsZombie)
	{
		m_pPlayer->RemoveAllItems(FALSE);
		m_pPlayer->m_bHasPrimary = false;
		if(!RANDOM_LONG(0, 3))
		{
			// 电锯套 25%
			m_pPlayer->GiveNamedItem("weapon_chainsaw");
			m_pPlayer->GiveNamedItem("weapon_infinityex2");
			m_pPlayer->GiveNamedItem("knife_dragonsword");
			m_pPlayer->GiveNamedItem("weapon_hegrenade");
		}
		else
		{
			// 龙炮套 75%
			m_pPlayer->GiveNamedItem("weapon_cannon");
			m_pPlayer->GiveNamedItem("weapon_infinityex2");
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
