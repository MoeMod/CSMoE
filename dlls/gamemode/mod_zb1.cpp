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
	PRECACHE_GENERIC("sound/Zombi_Ambience.mp3");

	CVAR_SET_FLOAT("sv_maxspeed", 390);
}

void CMod_Zombi::CheckMapConditions()
{
	IBaseMod_RemoveObjects::CheckMapConditions();
	CVAR_SET_STRING("sv_skyname", "hk"); // it should work, but...
//	CVAR_SET_FLOAT("sv_skycolor_r", 150);
//	CVAR_SET_FLOAT("sv_skycolor_g", 150);
//	CVAR_SET_FLOAT("sv_skycolor_b", 150);

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
	LIGHT_STYLE(0, "g"); // previous one is "f"
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

BOOL CMod_Zombi::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason)
{
	CLIENT_COMMAND(pEntity, "mp3 loop sound/Zombi_Ambience.mp3\n");

	return IBaseMod::ClientConnected(pEntity, pszName, pszAddress, szRejectReason);
}

void CMod_Zombi::ClientDisconnected(edict_t *pClient)
{
	CLIENT_COMMAND(pClient, "mp3 stop\n");

	IBaseMod::ClientDisconnected(pClient);
}

void CMod_Zombi::Think()
{
	//IBaseMod::Think();

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
			PickZombieOrigin();
		}
		TeamCheck();
	}

	if (CheckGameOver())   // someone else quit the game already
		return;

	if (CheckTimeLimit())
		return;

	if (IsFreezePeriod())
	{
		CheckFreezePeriodExpired();
	}

	if (m_fTeamCount != 0.0f && m_fTeamCount <= gpGlobals->time)
	{
		RestartRound();
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		CheckRestartRound();
		m_tmNextPeriodicThink = gpGlobals->time + 1.0f;

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

	if (TimeRemaining() <= 0 && !m_bRoundTerminating)
		HumanWin();
}

void CMod_Zombi::HumanWin()
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

void CMod_Zombi::ZombieWin()
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

void CMod_Zombi::CheckWinConditions()
{
	// If a winner has already been determined and game of started.. then get the heck out of here
	if (m_bFirstConnected && m_iRoundWinStatus != WINNER_NONE)
	{
		return;
	}

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!FInfectionStarted())
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
	int iCountDown = gpGlobals->time - m_fRoundCount;
	if (iCountDown <= 20)
		return false;

	return true;
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

int CMod_Zombi::ComputeMaxAmmo(CBasePlayer *player, const char *szAmmoClassName, int iOriginalMax)
{
	int ret = iOriginalMax * 2;

	// do not *2 for machine-guns.
	if (Q_strstr(szAmmoClassName, "box"))
		ret = iOriginalMax;

	return ret;
}

int CMod_Zombi::ZombieOriginNum()
{
	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (!NumAliveCT)
		return 0;
	return NumAliveCT / 10 + 1;
}

void CMod_Zombi::PickZombieOrigin()
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
		MakeZombie(players[i], ZOMBIE_LEVEL_ORIGIN);
		players[i]->pev->health = players[i]->pev->max_health = 1000 * iNumPlayers / iNumZombies + 1000;
		players[i]->pev->armorvalue = 1100;
	}

	// sound effect
	InfectionSound();
	CheckWinConditions();
}

void CMod_Zombi::HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker)
{
	MakeZombie(player, ZOMBIE_LEVEL_HOST);
	player->pev->health = player->pev->max_health = std::max(1000, static_cast<int>(attacker->pev->health * 0.5f));
	player->pev->armorvalue = std::max(100, static_cast<int>(attacker->pev->armorvalue * 0.5f));

	InfectionSound();
	PRECACHE_SOUND("zombi/human_death_01.wav");
	PRECACHE_SOUND("zombi/human_death_02.wav");
	EMIT_SOUND(ENT(player->pev), CHAN_BODY, RANDOM_LONG(0, 1) ? "zombi/human_death_01.wav" : "zombi/human_death_02.wav", VOL_NORM, ATTN_NORM);


	DeathNotice(player, attacker->pev, attacker->pev);
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
	pPlayer->m_bIsZombie = false;
	pPlayer->m_bNotKilled = false;
	IBaseMod::PlayerSpawn(pPlayer);
	pPlayer->AddAccount(16000);

	// Give Armor
	pPlayer->pev->health = pPlayer->pev->max_health= 1000;
	//pPlayer->pev->gravity = 0.86f;
	pPlayer->m_iKevlar = ARMOR_TYPE_HELMET;
	pPlayer->pev->armorvalue = 100;

	// Open buy menu on spawn
	ShowVGUIMenu(pPlayer, VGUI_Menu_Buy, (MENU_KEY_1 | MENU_KEY_2 | MENU_KEY_3 | MENU_KEY_4 | MENU_KEY_5 | MENU_KEY_6 | MENU_KEY_7 | MENU_KEY_8 | MENU_KEY_0), "#Buy");
	pPlayer->m_iMenu = Menu_Buy;
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

void CMod_Zombi::MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel)
{
	return player->MakeZombie(iEvolutionLevel);
}
