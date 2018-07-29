#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "game.h"
#include "client.h"
#include "bmodels.h"

#include "mod_zb1.h"

#include <algorithm>
#include <vector>

#include "bot_include.h"

CMod_Zombi::CMod_Zombi() // precache
{
	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
}

void CMod_Zombi::CheckMapConditions()
{
	IBaseMod_RemoveObjects::CheckMapConditions();
	CVAR_SET_STRING("sv_skyname", "hk"); // it should work, but...
	CVAR_SET_FLOAT("sv_skycolor_r", 150);
	CVAR_SET_FLOAT("sv_skycolor_g", 150);
	CVAR_SET_FLOAT("sv_skycolor_b", 150);

	// create fog, however it doesnt work...
	CBaseEntity *fog = nullptr;
	while ((fog = UTIL_FindEntityByClassname(fog, "env_fog")) != nullptr)
	{
		REMOVE_ENTITY(fog->edict());
	}
	CClientFog *newfog = GetClassPtr<CClientFog>(NULL);
	MAKE_STRING_CLASS("env_fog", newfog->pev);
	newfog->Spawn();
	newfog->m_fDensity = 0.0016f;
	newfog->pev->rendercolor = { 0,0,0 };

	// light
	LIGHT_STYLE(0, "f");
}

void CMod_Zombi::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(MOD_ZB1);
	WRITE_BYTE(0); // Reserved. (weapon restriction? )
	WRITE_BYTE(maxrounds.value); // MaxRound (mp_roundlimit)
	WRITE_BYTE(0); // Reserved. (MaxTime?)

	MESSAGE_END();
}

bool CMod_Zombi::CanPlayerBuy(CBasePlayer *player, bool display)
{
	// is the player alive?
	if (player->pev->deadflag != DEAD_NO)
		return false;

	if (player->m_bIsZombie)
		return false;

	return true;
}

void CMod_Zombi::Think()
{
	IBaseMod::Think();

	static int iLastCountDown = -1;
	int iCountDown = gpGlobals->time - m_fRoundCount;

	if (iCountDown != iLastCountDown)
	{
		iLastCountDown = iCountDown;
		if (iCountDown > 0 && iCountDown < 20 && !m_bFreezePeriod)
		{

			UTIL_ClientPrintAll(HUD_PRINTCENTER, "Time Remaining for Zombie Selection: %s1 Sec", UTIL_dtos1(20 - iCountDown)); // #CSO_ZombiSelectCount

			static const char *szCountDownSound[11] = {
				"", "one", "two", "three", "four", "five", "six",
				"seven", "eight", "nine", "ten"
			};

			if (iCountDown == 1)
			{
				for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
				{
					CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
					if (!entity)
						continue;
					CLIENT_COMMAND(entity->edict(), "spk zombi_start\n");
				}
			}
			else if (iCountDown >= 10)
			{
				for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
				{
					CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
					if (!entity)
						continue;
					CLIENT_COMMAND(entity->edict(), "spk %s\n", szCountDownSound[20 - iCountDown]);
				}
			}
		}
		else if (iCountDown == 20)
		{
			// select zombie
			MakeZombieOrigin();
		}
		TeamCheck();
	}
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

	int iCountDown = gpGlobals->time - m_fRoundCount;
	if (iCountDown <= 20)
		return;

	if (!NumAliveTerrorist || TimeRemaining() < 0.0f)
	{
		//Broadcast("ctwin");
		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;
			CLIENT_COMMAND(entity->edict(), "spk win_human\n");
		}
		EndRoundMessage("HumanWin", ROUND_CTS_WIN);
		TerminateRound(5, WINSTATUS_CTS);
		RoundEndScore(WINSTATUS_CTS);

		++m_iNumCTWins;
		UpdateTeamScores();
	}
	else if (!NumAliveCT)
	{
		//Broadcast("terwin");
		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
			if (!entity)
				continue;
			CLIENT_COMMAND(entity->edict(), "spk win_zombi\n");
		}
		EndRoundMessage("Zombie Win", ROUND_TERRORISTS_WIN);
		TerminateRound(5, WINSTATUS_TERRORISTS);
		RoundEndScore(WINSTATUS_TERRORISTS);

		++m_iNumTerroristWins;
		UpdateTeamScores();
	}

}

void CMod_Zombi::RoundEndScore(int iWinStatus)
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);
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
	return IBaseMod::PlayerKilled(pVictim, pKiller, pInflictor);
}

int CMod_Zombi::ZombieOriginNum()
{
	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!NumAliveCT)
		return 0;
	return NumAliveCT / 10 + 1;
}

void CMod_Zombi::MakeZombieOrigin()
{
	int iNumZombies = ZombieOriginNum();
	int iNumPlayers = this->m_iNumTerrorist + this->m_iNumCT;

	std::vector<CBasePlayer *> players;
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);
		if (!player->IsAlive())
			continue;
		if (player->m_iTeam == TEAM_UNASSIGNED || player->m_iTeam == TEAM_SPECTATOR)
			continue;

		players.push_back(player);
	}

	// randomize player list
	std::random_shuffle(players.begin(), players.end());

	// pick them
	for (int i = 0; i < iNumZombies; ++i)
	{
		players[i]->MakeZombie(ZOMBIE_LEVEL_ORIGIN);
		players[i]->pev->health = 1000 * iNumPlayers / iNumZombies + 1000;
		players[i]->pev->armorvalue = 1100;
	}

	// sound effect
	InfectionSound();
	CheckWinConditions();
}

void CMod_Zombi::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	player->MakeZombie(ZOMBIE_LEVEL_HOST);
	player->pev->health = std::max(1000, static_cast<int>(attacker->pev->health * 0.7f)) + 1000;
	player->pev->armorvalue = std::max(100, static_cast<int>(attacker->pev->armorvalue * 0.5f)) + 100;

	InfectionSound();
	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
	EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/human_death_01.wav" : "zombi/human_death_02.wav", VOL_NORM, ATTN_NORM);


	DeathNotice(player, attacker->pev, attacker->m_pActiveItem->pev);
	SetScoreAttrib(player, player);
	TeamCheck();
	CheckWinConditions();

	player->m_iDeaths += 1;
	player->AddPoints(0, FALSE);
	attacker->AddPoints(1, FALSE);
}

void CMod_Zombi::InfectionSound()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CLIENT_COMMAND(entity->edict(), "spk zombi_coming_%d\n", RANDOM_LONG(1, 2));
	}
}

void CMod_Zombi::RestartRound()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);
		player->m_bIsZombie = false;
	}

	TeamCheck();

	CVAR_SET_FLOAT("mp_autoteambalance", 0.0f);
	
	IBaseMod::RestartRound();
	m_bTCantBuy = false;
}

void CMod_Zombi::TeamCheck()
{
	for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
	{
		CBaseEntity *entity = UTIL_PlayerByIndex(iIndex);
		if (!entity)
			continue;
		CBasePlayer *player = static_cast<CBasePlayer *>(entity);
		
		if ((player->m_bIsZombie && player->m_iTeam != TERRORIST) || (!player->m_bIsZombie && player->m_iTeam != CT))
		{
			player->m_iTeam = player->m_bIsZombie ? TERRORIST :CT;
			TeamChangeUpdate(player, player->m_iTeam);

			TheBots->OnEvent(EVENT_PLAYER_CHANGED_TEAM, player);
		}
	}
}

void CMod_Zombi::PlayerSpawn(CBasePlayer *pPlayer)
{
	pPlayer->m_bNotKilled = false;
	IBaseMod::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Give Armor
	pPlayer->pev->health = 1000;
	pPlayer->pev->gravity = 0.86f;
	pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	pPlayer->pev->armorvalue = 100;
	pPlayer->m_bIsZombie = false;

	pPlayer->m_signals.Signal(SIGNAL_BUY);
}

BOOL CMod_Zombi::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	int iReturn = FALSE;

	if (!pAttacker || PlayerRelationship(pPlayer, pAttacker) != GR_TEAMMATE)
	{
		iReturn = TRUE;
	}

	if (CVAR_GET_FLOAT("mp_friendlyfire") != 0 || pAttacker == pPlayer)
	{
		iReturn = TRUE;
	}

	if (pAttacker->IsPlayer())
	{
		CBasePlayer *pAttacker2 = static_cast<CBasePlayer *>(pAttacker);
		if (pAttacker2->m_bIsZombie && !pPlayer->m_bIsZombie)
		{
			HumanInfectionByZombie(pPlayer, pAttacker2);
			iReturn = false;
		}
	}
	

	return iReturn;
}