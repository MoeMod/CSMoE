/*
mod_zb1.cpp - CSMoE Gameplay server : Zombie Mod
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
#include "client.h"
#include "bmodels.h"
#include "triggers.h"
#include "entity_state.h"

#include "mod_zb1.h"
#include "map_manager.h"

#include <algorithm>
#include <vector>
#include <random>

#include "bot_include.h"
#include "player/player_zombie.h"
#include "player/csdm_randomspawn.h"

#include "util/u_range.hpp"


namespace sv {

CMod_Zombi::CMod_Zombi() // precache
	: m_Countdown (this, &m_CountdownDelegate )
{
	m_Countdown.SetCounts(20);

	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
	PRECACHE_SOUND("zombi/human_death_female_01.wav");
	PRECACHE_SOUND("zombi/human_death_female_02.wav");
	PRECACHE_SOUND("zombi/KILLHUMANBOSSCOUNTSND.WAV");
	PRECACHE_GENERIC("sound/Zombi_Ambience.mp3");

	CVAR_SET_FLOAT("sv_maxspeed", 390);
}

void CMod_Zombi::CheckMapConditions()
{
	Base::CheckMapConditions();
	
	const char* skybox = MapMgr_GetMapSkybox(STRING(gpGlobals->mapname));
	if (skybox == NULL)
		CVAR_SET_STRING("sv_skyname", "hk"); // it should work, but...
	else
		CVAR_SET_STRING("sv_skyname", skybox);
	
//	CVAR_SET_FLOAT("sv_skycolor_r", 150);
//	CVAR_SET_FLOAT("sv_skycolor_g", 150);
//	CVAR_SET_FLOAT("sv_skycolor_b", 150);

	/*
	CBaseEntity *fog = nullptr;
	while ((fog = UTIL_FindEntityByClassname(fog, "env_fog")) != nullptr)
	{
		REMOVE_ENTITY(fog->edict());
	}

	edict_t* pent = CREATE_NAMED_ENTITY(MAKE_STRING("env_fog"));
	if (!FNullEnt(pent))
	{
		CClientFog* pEntity = static_cast<CClientFog*>(CBaseEntity::Instance(pent));
		DispatchSpawn(pEntity->edict());
		float fDenity = MapMgr_GetMapDensity(STRING(gpGlobals->mapname));
		pEntity->m_fDensity = fDenity;
		float iDensity_r = MapMgr_GetMapDensity_r(STRING(gpGlobals->mapname));
		float iDensity_g = MapMgr_GetMapDensity_g(STRING(gpGlobals->mapname));
		float iDensity_b = MapMgr_GetMapDensity_b(STRING(gpGlobals->mapname));
		pEntity->pev->rendercolor = { iDensity_r, iDensity_g, iDensity_b };
		m_pFogEntity = pEntity;
	}
	*/
	// light
	const char* light = MapMgr_GetMapBright(STRING(gpGlobals->mapname));
	if (light == NULL)
		LIGHT_STYLE(0, "l"); // previous one is "f"
	else
		LIGHT_STYLE(0, light);
}

void CMod_Zombi::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, nullptr, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(static_cast<int>(maxrounds.value)); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

BOOL CMod_Zombi::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	CLIENT_COMMAND(pEntity, "mp3 loop sound/\"win_human%d\"\n", RANDOM_LONG(1, 2));

	return CHalfLifeMultiplay::ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void CMod_Zombi::ClientDisconnected(edict_t *pClient)
{
	CLIENT_COMMAND(pClient, "mp3 stop\n");

	return CHalfLifeMultiplay::ClientDisconnected(pClient);
}

void CMod_Zombi::Think()
{
	m_Countdown.Think();
	if(!m_Countdown.IsExpired())
		TeamCheck();

	if (CheckGameOver())   // someone else quit the game already
		return;

	if (CheckTimeLimit())
		return;

	// have we hit the max rounds?
	if (CheckMaxRounds())
		return;

	if (CheckWinLimit())
		return;

	if (IsFreezePeriod())
	{
		CheckFreezePeriodExpired();
	}

	if (m_fTeamCount != invalid_time_point && m_fTeamCount <= gpGlobals->time)
	{
		RestartRound();
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0s;

		if (g_psv_accelerate->value != 5.0f)
		{
			CVAR_SET_FLOAT("sv_accelerate", 5.0);
		}

		if (g_psv_friction->value != 4.0f)
		{
			CVAR_SET_FLOAT("sv_friction", 4.0);
		}

		if (g_psv_stopspeed->value != 75.0f)
		{
			CVAR_SET_FLOAT("sv_stopspeed", 75.0);
		}

		m_iMaxRounds = (int)maxrounds.value;

		if (m_iMaxRounds < 0)
		{
			m_iMaxRounds = 0;
			CVAR_SET_FLOAT("mp_maxrounds", 0);
		}

		m_iMaxRoundsWon = (int)winlimit.value;

		if (m_iMaxRoundsWon < 0)
		{
			m_iMaxRoundsWon = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}
	}

	if (!IsCareer())
	{
		// have we hit the max rounds?
		if (CheckMaxRounds())
			return;

		if (CheckWinLimit())
			return;
	}

    CheckTimeoutWin();
}

void CMod_Zombi::CheckTimeoutWin()
{
    if (TimeRemaining() <= 0s && !m_bRoundTerminating && !m_bFreezePeriod)
        HumanWin();
}

void CMod_Zombi::HumanWin()
{
	//Broadcast("ctwin");
	//for(CBasePlayer *player : moe::range::PlayersList())
		//CLIENT_COMMAND(player->edict(), "spk win_human\n");

	UTIL_BroadcastStuffText("mp3 stop;spk \"win_human%d\"\n", RANDOM_LONG(1, 2));

	/*MESSAGE_BEGIN(MSG_ALL, gmsgShowWin);
	WRITE_BYTE(2);
	WRITE_BYTE(5);
	MESSAGE_END();*/

	EndRoundMessage("Human Win", ROUND_CTS_WIN);
	TerminateRound(5s, WINSTATUS_CTS);
	RoundEndScore(WINSTATUS_CTS);

	++m_iNumCTWins;
	UpdateTeamScores();
}

void CMod_Zombi::ZombieWin()
{
	//Broadcast("terwin");
	//for(CBasePlayer *player : moe::range::PlayersList())
		//CLIENT_COMMAND(player->edict(), "spk win_zombi\n");

	UTIL_BroadcastStuffText("mp3 stop;spk \"win_zombie%d\"\n", RANDOM_LONG(1, 3));

	/*MESSAGE_BEGIN(MSG_ALL, gmsgShowWin);
	WRITE_BYTE(3);
	WRITE_BYTE(5);
	MESSAGE_END();*/

	EndRoundMessage("Zombie Win", ROUND_TERRORISTS_WIN);
	TerminateRound(5s, WINSTATUS_TERRORISTS);
	RoundEndScore(WINSTATUS_TERRORISTS);

	++m_iNumTerroristWins;
	UpdateTeamScores();
}

void CMod_Zombi::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!FInfectionStarted() || m_bRoundTerminating)
		return;
	
	if (!NumAliveTerrorist)
	{
		HumanWin();
	}
	else if (!NumAliveCT)
	{
		ZombieWin();
	}

}

BOOL CMod_Zombi::FInfectionStarted()
{
	return m_Countdown.IsExpired();
}

void CMod_Zombi::RoundEndScore(int iWinStatus)
{
	for(CBasePlayer *player : moe::range::PlayersList())
	{
		if (player->m_iTeam == TEAM_UNASSIGNED || player->m_iTeam == TEAM_SPECTATOR)
			continue;

		if (iWinStatus == WINSTATUS_CTS)
		{
			if (player->IsAlive() && !player->m_bIsZombie)
			{
				player->pev->frags += 3;

				MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(player->edict()));
				WRITE_SHORT((int)player->pev->frags);
				WRITE_SHORT(player->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(player->m_iTeam);
				MESSAGE_END();
			}
		}
		else if (iWinStatus == WINSTATUS_TERRORISTS)
		{
			if (player->m_bIsZombie)
			{
				player->pev->frags += 1;

				MESSAGE_BEGIN(MSG_BROADCAST, gmsgScoreInfo);
				WRITE_BYTE(ENTINDEX(player->edict()));
				WRITE_SHORT((int)player->pev->frags);
				WRITE_SHORT(player->m_iDeaths);
				WRITE_SHORT(0);
				WRITE_SHORT(player->m_iTeam);
				MESSAGE_END();
			}
		}

		
	}

}

int CMod_Zombi::IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled)
{
	if (!pAttacker->m_bIsZombie && pKilled->m_bIsZombie)
		return 3;

	return 0;
}

void CMod_Zombi::PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor)
{
	// additional death when zombie being killed.
	if (pVictim->m_bIsZombie)
	{
		pVictim->m_iDeaths++;
	}
	return CHalfLifeMultiplay::PlayerKilled(pVictim, pKiller, pInflictor);
}

void CMod_Zombi::InstallPlayerModStrategy(CBasePlayer *player)
{
	std::unique_ptr<CPlayerModStrategy_ZB1> up(new CPlayerModStrategy_ZB1(player, this));
	player->m_pModStrategy = std::move(up);
}

int CPlayerModStrategy_ZB1::ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax)
{
	int ret;
	// do not *2 for machine-guns.

	ret = iOriginalMax * 1.5;

	if (iOriginalMax < 200)
		ret = iOriginalMax *= 2;

	if (!strcmp(szAmmoClassName, "bowarrow"))
		ret = 60;
	if (!strcmp(szAmmoClassName, "pianogun_energycell"))
		ret = 360;
	else if (!strcmp(szAmmoClassName, "crossbowex21"))
		ret = 500;
	else if (!strcmp(szAmmoClassName, "lockongun"))
		ret = 500;
	else if (!strcmp(szAmmoClassName, "CannonAmmo"))
		ret = 20;
	else if (!strcmp(szAmmoClassName, "laserfist"))
		ret = 1500;
	else if (!strcmp(szAmmoClassName, "CannonEXAmmo"))
		ret = 30;
	else if (!strcmp(szAmmoClassName, "guillotineAmmo"))
		ret = 10;
	else if (!strcmp(szAmmoClassName, "guillotineexAmmo"))
		ret = 50;
	else if (!strcmp(szAmmoClassName, "408CheyTac"))
		ret = 30;
	else if (!strcmp(szAmmoClassName, "SpearAmmo"))
		ret = 30;
	else if (!strcmp(szAmmoClassName, "RailBeam"))
		ret = 35;
	else if (!strcmp(szAmmoClassName, "HEGrenade"))
		ret = 2;
	else if (!strcmp(szAmmoClassName, "Flashbang"))
		ret = 2;
	else if (!strcmp(szAmmoClassName, "SmokeGrenade"))
		ret = 1;

	return ret;
}

void CPlayerModStrategy_ZB1::OnSpawn()
{
	BecomeHuman();

	return CPlayerModStrategy_Default::OnSpawn();
}

void CPlayerModStrategy_ZB1::Event_OnBecomeZombie(CBasePlayer *who, ZombieLevel iEvolutionLevel)
{
	if (m_pPlayer != who)
		return;

	BecomeZombie(iEvolutionLevel);
	m_pPlayer->OnBecomeZombie(iEvolutionLevel);
}

void CPlayerModStrategy_ZB1::BecomeZombie(ZombieLevel iEvolutionLevel)
{
	m_pPlayer->pev->flags &= ~FL_IMMUNE_LAVA;
	m_pCharacter = std::make_shared<CZombie_ZB1>(m_pPlayer, iEvolutionLevel);
	m_pPlayer->m_iTeam = TERRORIST;
	m_pPlayer->pev->team = TERRORIST;
}

void CPlayerModStrategy_ZB1::BecomeHuman()
{
	m_pCharacter = std::make_shared<CHuman_ZB1>(m_pPlayer);
	m_pPlayer->m_iTeam = CT;
	m_pPlayer->pev->team = CT;
}

CPlayerModStrategy_ZB1::CPlayerModStrategy_ZB1(CBasePlayer *player, CMod_Zombi *mp)
	:   CPlayerModStrategy_Zombie(player),
		m_pModZB1(mp),
	    m_eventBecomeZombieListener(mp->m_eventBecomeZombie.subscribe(&CPlayerModStrategy_ZB1::Event_OnBecomeZombie, this))
{

}

float CPlayerModStrategy_ZB1::AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if(m_pCharacter)
		flDamage = m_pCharacter->AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	return CPlayerModStrategy_Zombie::AdjustDamageTaken(pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

void CPlayerModStrategy_ZB1::Pain(int m_LastHitGroup, bool HasArmour)
{
	if(m_pPlayer->m_bIsZombie && m_pCharacter)
		return m_pCharacter->Pain_Zombie(m_LastHitGroup, HasArmour);
	return CPlayerModStrategy_Zombie::Pain(m_LastHitGroup, HasArmour);
}

void CPlayerModStrategy_ZB1::DeathSound()
{
	if(m_pPlayer->m_bIsZombie && m_pCharacter)
		return m_pCharacter->DeathSound_Zombie();
	return CPlayerModStrategy_Zombie::DeathSound();
}

void CPlayerModStrategy_ZB1::UpdateClientData(int sendweapons, struct clientdata_s* cd, entvars_t* pevOrg)
{
	CPlayerModStrategy_Default::UpdateClientData(sendweapons, cd, pevOrg);

	CBasePlayer* pl = m_pPlayer;
	entvars_t* pev = pl->pev;

	cd->health = (int)pev->health % 512 == 0 ? pev->health + 1 : pev->health;
}

int CPlayerModStrategy_ZB1::DeathCallBack(CBasePlayer* pVictim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_pPlayer->m_iBuff & BUFF_AVOID_INFECT)
	{
		return 0;
	}

	return CPlayerModStrategy_Default::DeathCallBack(pVictim, pevInflictor, pevAttacker, flDamage, bitsDamageType);
}

int CPlayerModStrategy_ZB1::TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_pPlayer->m_bIsZombie)
		return 1;

	CBaseEntity* pAttacker = GetClassPtr<CBaseEntity>(pevAttacker);
	if (!pAttacker->IsPlayer())
		return 1;

	CBasePlayer* pAttackerPlayer = GetClassPtr<CBasePlayer>(pevAttacker);
	if (!pAttackerPlayer->m_bIsZombie)
		return 1;

	if (DeathCallBack(m_pPlayer, pevInflictor, pevAttacker, flDamage, bitsDamageType))
		return 0;

	m_pModZB1->HumanInfectionByZombie(m_pPlayer, pAttackerPlayer);
	return 0;
}

size_t CMod_Zombi::ZombieOriginNum()
{
	moe::range::PlayersList list;
	return static_cast<size_t>(std::distance(list.begin(), list.end()) / 10 + 1);
}

void CMod_Zombi::PickZombieOrigin()
{
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
		players[i]->m_ZombieClass = "random";
		players[i]->pev->sequence = 0;
		players[i]->m_iCustomSequence = -1;
		MakeZombie(players[i], ZOMBIE_LEVEL_ORIGIN);
		players[i]->pev->health = players[i]->pev->max_health = 1000.0f * iNumPlayers / iNumZombies + 1000.0f;
		players[i]->pev->armorvalue = 1100;
	}
	TeamCheck();

	// sound effect
	UTIL_BroadcastStuffText("mp3 stop;spk \"r_start\"\n");
	CheckWinConditions();
}

void CMod_Zombi::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	player->m_ZombieClass = "random";
	player->pev->sequence = 0;
	player->m_tLockBuffTime = invalid_time_point;
	player->m_flLockSpeed = 0.0;
	player->m_flLockGravity = 0.0;
	player->m_iCustomSequence = -1;
	MakeZombie(player, ZOMBIE_LEVEL_HOST);
	player->pev->health = player->pev->max_health = std::max(3000, static_cast<int>(attacker->pev->health * 1.0f)) + 1000.0f;
	player->m_iKevlar = ARMOR_TYPE_HELMET;
	player->pev->armorvalue = std::max(100, static_cast<int>(attacker->pev->armorvalue * 1.0f)) + 100.0f;

	InfectionSound();
	EmitHomoSound(player, attacker);


	DeathNotice(player, attacker->pev, attacker->pev);
	SetScoreAttrib(player, player);
	TeamCheck();
	CheckWinConditions();

	player->m_iDeaths += 1;
	player->AddPoints(0, FALSE);
	attacker->m_iRoundInfect++;
	attacker->AddPoints(1, FALSE);
}

void CMod_Zombi::InfectionSound()
{
	UTIL_BroadcastStuffText("spk \"zombi_coming_%d\"\n", RANDOM_LONG(1, 2));
}

void CMod_Zombi::EmitHomoSound(CBasePlayer *player, CBasePlayer *attacker)
{
	if (player->m_iBuff & BUFF_GHOSTHUNTER)
	{
		UTIL_BroadcastStuffText("spk \"zombi/KILLHUMANBOSSCOUNTSND\"\n");
		return;
	}
		
	if(!player->m_bIsFemale)
		EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/human_death_01.wav" : "zombi/human_death_02.wav", VOL_NORM, ATTN_NORM);
	else
		EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/human_death_female_01.wav" : "zombi/human_death_female_02.wav", VOL_NORM, ATTN_NORM);
}

void CMod_Zombi::RestartRound()
{
	for(CBasePlayer *player : moe::range::PlayersList())
		player->m_bIsZombie = false;

	TeamCheck();

	CVAR_SET_FLOAT("mp_autoteambalance", 0.0f);

	CHalfLifeMultiplay::RestartRound();
	m_bTCantBuy = false;
}

void CMod_Zombi::TeamCheck()
{
	for(CBasePlayer *player : moe::range::PlayersList())
	{
		if (player->m_iTeam == UNASSIGNED || player->m_iTeam == SPECTATOR)
			continue;

		if ((player->m_bIsZombie && player->m_iTeam != TERRORIST) || (!player->m_bIsZombie && player->m_iTeam != CT))
		{
			player->pev->team = player->m_bIsZombie ? TERRORIST : CT;
			player->m_iTeam = player->m_bIsZombie ? TERRORIST :CT;
			TeamChangeUpdate(player, player->m_iTeam);

			TheBots->OnEvent(EVENT_PLAYER_CHANGED_TEAM, player);
		}
	}
}

void CMod_Zombi::PlayerSpawn(CBasePlayer *pPlayer)
{
	pPlayer->m_bIsZombie = false;
	pPlayer->m_bNotKilled = false;
	CHalfLifeMultiplay::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Open buy menu on spawn
	//ShowVGUIMenu(pPlayer, VGUI_Menu_Buy, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_3 | MENU_KEY_4 | MENU_KEY_5 | MENU_KEY_6 | MENU_KEY_7 | MENU_KEY_8 | MENU_KEY_0), "#Buy");
	//pPlayer->m_iMenu = Menu_Buy;
}

BOOL CMod_Zombi::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
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


	CBasePlayer *pAttackerPlayer = dynamic_ent_cast<CBasePlayer *>(pAttacker);
	if (pAttackerPlayer)
	{
		if (pAttackerPlayer->m_bIsZombie && !pPlayer->m_bIsZombie)
		{
			if (pPlayer->m_iBuff & BUFF_HOLYSWORD_AVOID_INFECT)
			{
				UTIL_MakeVectors(pPlayer->pev->angles);
				if (DotProduct((pPlayer->pev->origin.Make2D() - pAttackerPlayer->pev->origin.Make2D()).Normalize(), gpGlobals->v_forward.Make2D()) > 0.8)
					return 0;

				CBasePlayerWeapon* pWeapon = (CBasePlayerWeapon*)pPlayer->m_rgpPlayerItems[KNIFE_SLOT];

				if (pWeapon)
				{
					if (!Q_strcmp(STRING(pWeapon->pev->classname), "knife_holysword") || !Q_strcmp(STRING(pWeapon->pev->classname), "knife_y22s1holyswordmb"))
						pWeapon->WeaponCallBack(0);
				}
				return 1;
			}

			if (pPlayer->m_iBuff & BUFF_AVOID_INFECT)
			{
				iReturn = false;
				return iReturn;
			}

			HumanInfectionByZombie(pPlayer, pAttackerPlayer);
			iReturn = false;
		}
	}
	

	return iReturn;
}

void CZB1CountdownDelegate::OnCountdownStart()
{
	UTIL_BroadcastStuffText("spk \"zombi_start%d\"\n", RANDOM_LONG(1, 2));
}

inline void CZB1CountdownDelegate::OnCountdownChanged(int iCurrentCount)
{
	static const char* s_szCountDownSound[11] = {
			"", "one", "two", "three", "four", "five", "six",
			"seven", "eight", "nine", "ten"
	};

	UTIL_ClientPrintAll(HUD_PRINTCENTER, "#CSO_ZombiSelectCount", UTIL_dtos1(iCurrentCount)); // #CSO_ZombiSelectCount
	if (iCurrentCount <= 10)
	{
		UTIL_BroadcastStuffText("spk \"%s\"\n", s_szCountDownSound[iCurrentCount]);
	}
}

inline void CZB1CountdownDelegate::OnCountdownEnd()
{
	// select zombie
	m_pMod->PickZombieOrigin();
}

enum CCSBotManager::GameScenarioType CMod_Zombi::OverrideGameScenario(enum CCSBotManager::GameScenarioType type)
{
	return CCSBotManager::SCENARIO_ZOMBIEMOD_INFETCION;
}

bool CMod_Zombi::CanChangeTeam(CBasePlayer *player, int newTeam_id, int curTeam_id)
{
	return ((curTeam_id == UNASSIGNED || curTeam_id == SPECTATOR) || newTeam_id == SPECTATOR) || player->IsBot();
}

}
