
#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "player.h"
#include "client.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "hltv.h"
#include "game.h"
#include "trains.h"
#include "vehicle.h"
#include "globals.h"

#include "debug.h"

#include "pm_shared.h"
#include "utllinkedlist.h"

// CSBOT and Nav
#include "game_shared/GameEvent.h"		// Game event enum used by career mode, tutor system, and bots
#include "game_shared/bot/bot_util.h"
#include "game_shared/bot/simple_state_machine.h"

#include "game_shared/steam_util.h"

#include "game_shared/bot/bot_manager.h"
#include "game_shared/bot/bot_constants.h"
#include "game_shared/bot/bot.h"

#include "game_shared/shared_util.h"
#include "game_shared/bot/bot_profile.h"

#include "game_shared/bot/nav.h"
#include "game_shared/bot/improv.h"
#include "game_shared/bot/nav_node.h"
#include "game_shared/bot/nav_area.h"
#include "game_shared/bot/nav_file.h"
#include "game_shared/bot/nav_path.h"

#include "airtank.h"
#include "h_ai.h"
#include "h_cycler.h"
#include "h_battery.h"

// Hostage
#include "hostage/hostage.h"
#include "hostage/hostage_localnav.h"

#include "bot/cs_bot.h"

// Tutor
#include "tutor.h"
#include "tutor_base_states.h"
#include "tutor_base_tutor.h"
#include "tutor_cs_states.h"
#include "tutor_cs_tutor.h"

#include "gamerules.h"
#include "career_tasks.h"
#include "maprules.h"

/*
* Globals initialization
*/
CBotManager *TheBots = NULL;

float CCSBotManager::m_flNextCVarCheck = 0.0f;
bool CCSBotManager::m_isMapDataLoaded = false;
bool CCSBotManager::m_isLearningMap = false;
bool CCSBotManager::m_isAnalysisRequested = false;
NavEditCmdType CCSBotManager::m_editCmd = EDIT_NONE;


CCSBotManager::CCSBotManager()
{
	m_flNextCVarCheck = 0.0f;

	m_zoneCount = 0;
	SetLooseBomb(NULL);

	m_isBombPlanted = false;
	m_bombDefuser = NULL;

	m_isLearningMap = false;
	m_isAnalysisRequested = false;
	m_editCmd = EDIT_NONE;

	m_navPlace = false;
	m_roundStartTimestamp = 0.0f;

	m_bServerActive = false;

	TheBotPhrases = new BotPhraseManager;
	// load the database of bot radio chatter
	TheBotPhrases->Initialize("BotChatter.db", 0);

	TheBotProfiles = new BotProfileManager;
	// make sure default voice bank is first
	TheBotProfiles->FindVoiceBankIndex("BotChatter.db");

	const char *filename;
	if (IS_CAREER_MATCH())
	{
		filename = "MissionPacks/BotPackList.db";
	}
	else
	{
		filename = "BotPackList.db";
	}

	// read in the list of bot profile DBs
	int dataLength;
	char *dataPointer = (char *)LOAD_FILE_FOR_ME((char *)filename, &dataLength);

	if (dataPointer == NULL)
	{
		TheBotProfiles->Init("BotProfile.db");
	}
	else
	{
		const char *dataFile = SharedParse(dataPointer);
		const char *token;

		while (dataFile != NULL)
		{
			token = SharedGetToken();
			char *clone = CloneString(token);
			TheBotProfiles->Init(clone);
			delete[] clone;
			dataFile = SharedParse(dataFile);
		}

		FREE_FILE(dataPointer);
	}

	// Now that we've parsed all the profiles, we have a list of the voice banks they're using.
	// Go back and parse the custom voice speakables.
	const BotProfileManager::VoiceBankList *pVoiceBanks = TheBotProfiles->GetVoiceBanks();
	for (int i = 1; i < pVoiceBanks->Count(); ++i)
	{
		TheBotPhrases->Initialize((*pVoiceBanks)[i], i);
	}
}

// Invoked when a new round begins

void CCSBotManager::RestartRound()
{
	// extend
	CBotManager::RestartRound();

	SetLooseBomb(NULL);
	m_isBombPlanted = false;
	m_earliestBombPlantTimestamp = gpGlobals->time + RANDOM_FLOAT(10.0f, 30.0f);
	m_bombDefuser = NULL;

	m_editCmd = EDIT_NONE;

	ResetRadioMessageTimestamps();

	m_lastSeenEnemyTimestamp = -9999.9f;

	m_roundStartTimestamp = gpGlobals->time + CVAR_GET_FLOAT("mp_freezetime");

	// randomly decide if defensive team wants to "rush" as a whole
	const float defenseRushChance = 33.3f;	// 25.0f;
	m_isDefenseRushing = (RANDOM_FLOAT(0.0f, 100.0f) <= defenseRushChance) ? true : false;

	TheBotPhrases->OnRoundRestart();

	m_isRoundOver = false;
	m_isRespawnStarted = false;
	m_canRespawn = true;
}

void UTIL_DrawBox(Extent *extent, int lifetime, int red, int green, int blue)
{
	Vector v[8];
	v[0].x = extent->lo.x; v[0].y = extent->lo.y; v[0].z = extent->lo.z;
	v[1].x = extent->hi.x; v[1].y = extent->lo.y; v[1].z = extent->lo.z;
	v[2].x = extent->hi.x; v[2].y = extent->hi.y; v[2].z = extent->lo.z;
	v[3].x = extent->lo.x; v[3].y = extent->hi.y; v[3].z = extent->lo.z;
	v[4].x = extent->lo.x; v[4].y = extent->lo.y; v[4].z = extent->hi.z;
	v[5].x = extent->hi.x; v[5].y = extent->lo.y; v[5].z = extent->hi.z;
	v[6].x = extent->hi.x; v[6].y = extent->hi.y; v[6].z = extent->hi.z;
	v[7].x = extent->lo.x; v[7].y = extent->hi.y; v[7].z = extent->hi.z;

	static int edge[] =
	{
		1, 2, 3, 4, -1,
		5, 6, 7, 8, -5,
		1, -5,
		2, -6,
		3, -7,
		4, -8,
		0	// end iterator
	};

	Vector from, to;
	bool restart = true;

	for (int i = 0; edge[i] != 0; ++i)
	{
		if (restart)
		{
			to = v[ edge[i] - 1 ];
			restart = false;
			continue;
		}

		from = to;

		int index = edge[i];
		if (index < 0)
		{
			restart = true;
			index = -index;
		}

		to = v[ index - 1 ];

		UTIL_DrawBeamPoints(from, to, lifetime, red, green, blue);
		UTIL_DrawBeamPoints(to, from, lifetime, red, green, blue);
	}
}

// Called each frame

void CCSBotManager::StartFrame()
{
	// EXTEND
	CBotManager::StartFrame();
	MonitorBotCVars();

	// debug zone extent visualization
	if (cv_bot_debug.value == 5.0f)
	{
		for (int z = 0; z < m_zoneCount; ++z)
		{
			Zone *zone = &m_zone[z];
			UTIL_DrawBox(&zone->m_extent, 1, 255, 100, 0);
		}
	}
}

// Return true if the bot can use this weapon

bool CCSBotManager::IsWeaponUseable(CBasePlayerItem *item) const
{
	if (item == NULL)
	{
		return false;
	}

	if (item->m_iId == WEAPON_C4)
		return true;

	int weaponClass = WeaponIDToWeaponClass(item->m_iId);

	if ((!AllowShotguns() && weaponClass == WEAPONCLASS_SHOTGUN)
		|| (!AllowMachineGuns() && weaponClass == WEAPONCLASS_MACHINEGUN)
		|| (!AllowRifles() && weaponClass == WEAPONCLASS_RIFLE)
		|| (!AllowSnipers() && weaponClass == WEAPONCLASS_SNIPERRIFLE)
		|| (!AllowSubMachineGuns() && weaponClass == WEAPONCLASS_SUBMACHINEGUN)
		|| (!AllowTacticalShield() && item->m_iId == WEAPON_SHIELDGUN)
		|| (!AllowPistols() && weaponClass == WEAPONCLASS_PISTOL)
		|| (!AllowGrenades() && weaponClass == WEAPONCLASS_GRENADE))
	{
		return false;
	}

	return true;
}

// Return true if this player is on "defense"

bool CCSBotManager::IsOnDefense(CBasePlayer *player) const
{
	switch (GetScenario())
	{
		case SCENARIO_DEFUSE_BOMB:
			return (player->m_iTeam == CT);

		case SCENARIO_RESCUE_HOSTAGES:
			return (player->m_iTeam == TERRORIST);

		case SCENARIO_ESCORT_VIP:
			return (player->m_iTeam == TERRORIST);

		default:
			break;
	}

	return false;
}

// Return true if this player is on "offense"

bool CCSBotManager::IsOnOffense(CBasePlayer *player) const
{
	return !IsOnDefense(player);
}

// Invoked when a map has just been loaded

void CCSBotManager::ServerActivate()
{
	DestroyNavigationMap();
	m_isMapDataLoaded = false;

	m_zoneCount = 0;
	m_gameScenario = SCENARIO_DEATHMATCH;

	ValidateMapData();
	RestartRound();

	m_isLearningMap = false;
	m_isAnalysisRequested = false;

	m_bServerActive = true;
	AddServerCommands();

	TheBotPhrases->OnMapChange();
}

void CCSBotManager::AddServerCommand(const char *cmd)
{
	ADD_SERVER_COMMAND((char *)cmd, Bot_ServerCommand);
}

void CCSBotManager::AddServerCommands()
{
	static bool fFirstTime = true;

	if (!fFirstTime)
		return;

	fFirstTime = false;

	if (g_bEnableCSBot)
	{
		AddServerCommand("bot_about");
		AddServerCommand("bot_add");
		AddServerCommand("bot_add_t");
		AddServerCommand("bot_add_ct");
		AddServerCommand("bot_kill");
		AddServerCommand("bot_kick");
		AddServerCommand("bot_knives_only");
		AddServerCommand("bot_pistols_only");
		AddServerCommand("bot_snipers_only");
		AddServerCommand("bot_all_weapons");
		AddServerCommand("entity_dump");
		AddServerCommand("bot_nav_delete");
		AddServerCommand("bot_nav_split");
		AddServerCommand("bot_nav_merge");
		AddServerCommand("bot_nav_mark");
		AddServerCommand("bot_nav_begin_area");
		AddServerCommand("bot_nav_end_area");
		AddServerCommand("bot_nav_connect");
		AddServerCommand("bot_nav_disconnect");
		AddServerCommand("bot_nav_splice");
		AddServerCommand("bot_nav_crouch");
		AddServerCommand("bot_nav_jump");
		AddServerCommand("bot_nav_precise");
		AddServerCommand("bot_nav_no_jump");
		AddServerCommand("bot_nav_analyze");
		AddServerCommand("bot_nav_strip");
		AddServerCommand("bot_nav_save");
		AddServerCommand("bot_nav_load");
		AddServerCommand("bot_nav_use_place");
		AddServerCommand("bot_nav_place_floodfill");
		AddServerCommand("bot_nav_place_pick");
		AddServerCommand("bot_nav_toggle_place_mode");
		AddServerCommand("bot_nav_toggle_place_painting");
		AddServerCommand("bot_goto_mark");
		AddServerCommand("bot_memory_usage");
		AddServerCommand("bot_nav_mark_unnamed");
		AddServerCommand("bot_nav_warp");
		AddServerCommand("bot_nav_corner_select");
		AddServerCommand("bot_nav_corner_raise");
		AddServerCommand("bot_nav_corner_lower");
		AddServerCommand("bot_nav_check_consistency");
	}
}

void CCSBotManager::ServerDeactivate()
{
	m_bServerActive = false;
}

void CCSBotManager::ClientDisconnect(CBasePlayer *pPlayer)
{
	if (pPlayer != NULL && pPlayer->IsBot())
	{
		entvars_t *temp = VARS(pPlayer->edict());
		CCSBot *pBot = static_cast<CCSBot *>(pPlayer);

		if (pBot != NULL)
		{
			pBot->Disconnect();
		}

		if (!FStringNull(pPlayer->pev->classname))
		{
			RemoveEntityHashValue(pPlayer->pev, STRING(pPlayer->pev->classname), CLASSNAME);
		}

		FREE_PRIVATE(pPlayer->edict());

		CBasePlayer *player = GetClassPtr<CBasePlayer>(temp);
		AddEntityHashValue(player->pev, STRING(player->pev->classname), CLASSNAME);
		player->pev->flags = FL_DORMANT;
	}
}

void PrintAllEntities()
{
	for (int i = 1; i < gpGlobals->maxEntities; ++i)
	{
		edict_t *edict = INDEXENT(i);

		if (!edict || FStringNull(edict->v.classname))
			continue;

		CONSOLE_ECHO("  %s\n", STRING(edict->v.classname));
	}
}

void CCSBotManager::ServerCommand(const char *pcmd)
{
	if (!m_bServerActive || !g_bEnableCSBot)
		return;

	char buffer[512];
	const char *msg = CMD_ARGV(1);

#if 0 // crashes on xash
	if (FStrEq(pcmd, "bot_about"))
	{
		Q_snprintf(buffer, sizeof (buffer), "\n--------------------------------------------------------------------------\nThe Official Counter-Strike Bot V%d.%02d\nCreated by Michael S. Booth\nWeb: www.turtlerockstudios.com\\csbot\nE-mail: csbot@turtlerockstudios.com\n--------------------------------------------------------------------------\n\n", CSBOT_VERSION_MAJOR, CSBOT_VERSION_MINOR);
		g_engfuncs.pfnServerPrint(buffer);
		HintMessageToAllPlayers(buffer);
	}
	else 
#endif
   if (FStrEq(pcmd, "bot_add"))
	{
		BotAddCommand(BOT_TEAM_ANY, FROM_CONSOLE);
	}
	else if (FStrEq(pcmd, "bot_add_t"))
	{
		BotAddCommand(BOT_TEAM_T, FROM_CONSOLE);
	}
	else if (FStrEq(pcmd, "bot_add_ct"))
	{
		BotAddCommand(BOT_TEAM_CT, FROM_CONSOLE);
	}
	else if (FStrEq(pcmd, "bot_kill"))
	{
		bool killThemAll;
		if (CMD_ARGC() == 1 || FStrEq(msg, "all"))
			killThemAll = true;
		else
			killThemAll = false;

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(iIndex));

			if (pPlayer == NULL)
				continue;

			if (FNullEnt(pPlayer->pev))
				continue;

			const char *name = STRING(pPlayer->pev->netname);

			if (FStrEq(name, ""))
				continue;

			if (pPlayer->IsBot())
			{
				if (killThemAll || FStrEq(name, msg))
				{
					pPlayer->TakeDamage(pPlayer->pev, pPlayer->pev, 9999.9f, DMG_CRUSH);
				}
			}
		}
	}
	else if (FStrEq(pcmd, "bot_kick"))
	{
		bool kickThemAll;
		if (CMD_ARGC() == 1 || FStrEq(msg, "all"))
			kickThemAll = true;
		else
			kickThemAll = false;

		for (int iIndex = 1; iIndex <= gpGlobals->maxClients; ++iIndex)
		{
			CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_PlayerByIndex(iIndex));

			if (pPlayer == NULL)
				continue;

			if (FNullEnt(pPlayer->pev))
				continue;

			const char *name = STRING(pPlayer->pev->netname);

			if (FStrEq(name, ""))
				continue;

			if (pPlayer->IsBot())
			{
				if (kickThemAll || FStrEq(name, msg))
				{
					// adjust bot quota so kicked bot is not immediately added back in
					int newQuota = cv_bot_quota.value - 1;
					SERVER_COMMAND(UTIL_VarArgs("kick \"%s\"\n", name));
					if (newQuota > cv_bot_quota.value)
						newQuota = (int)cv_bot_quota.value;
					if (newQuota < 0)
						newQuota = 0;
					//CVAR_SET_FLOAT("bot_quota", clamp(newQuota, 0, (int)cv_bot_quota.value));
					CVAR_SET_FLOAT("bot_quota", newQuota);
				}
			}
		}

		if (kickThemAll || cv_bot_quota.value < 0.0f)
		{
			CVAR_SET_FLOAT("bot_quota", 0);
		}
	}
	else if (FStrEq(pcmd, "bot_knives_only"))
	{
		CVAR_SET_FLOAT("bot_allow_pistols", 0);
		CVAR_SET_FLOAT("bot_allow_shotguns", 0);
		CVAR_SET_FLOAT("bot_allow_sub_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_rifles", 0);
		CVAR_SET_FLOAT("bot_allow_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_grenades", 0);
		CVAR_SET_FLOAT("bot_allow_snipers", 0);
		CVAR_SET_FLOAT("bot_allow_shield", 0);
	}
	else if (FStrEq(pcmd, "bot_pistols_only"))
	{
		CVAR_SET_FLOAT("bot_allow_pistols", 1);
		CVAR_SET_FLOAT("bot_allow_shotguns", 0);
		CVAR_SET_FLOAT("bot_allow_sub_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_rifles", 0);
		CVAR_SET_FLOAT("bot_allow_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_grenades", 0);
		CVAR_SET_FLOAT("bot_allow_snipers", 0);
		CVAR_SET_FLOAT("bot_allow_shield", 0);
	}
	else if (FStrEq(pcmd, "bot_snipers_only"))
	{
		CVAR_SET_FLOAT("bot_allow_pistols", 1);
		CVAR_SET_FLOAT("bot_allow_shotguns", 0);
		CVAR_SET_FLOAT("bot_allow_sub_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_rifles", 0);
		CVAR_SET_FLOAT("bot_allow_machine_guns", 0);
		CVAR_SET_FLOAT("bot_allow_grenades", 0);
		CVAR_SET_FLOAT("bot_allow_snipers", 1);
		CVAR_SET_FLOAT("bot_allow_shield", 0);
	}
	else if (FStrEq(pcmd, "bot_all_weapons"))
	{
		CVAR_SET_FLOAT("bot_allow_pistols", 1);
		CVAR_SET_FLOAT("bot_allow_shotguns", 1);
		CVAR_SET_FLOAT("bot_allow_sub_machine_guns", 1);
		CVAR_SET_FLOAT("bot_allow_rifles", 1);
		CVAR_SET_FLOAT("bot_allow_machine_guns", 1);
		CVAR_SET_FLOAT("bot_allow_grenades", 1);
		CVAR_SET_FLOAT("bot_allow_snipers", 1);
		CVAR_SET_FLOAT("bot_allow_shield", 1);
	}
	else if (FStrEq(pcmd, "entity_dump"))
	{
		PrintAllEntities();
	}
	else if (FStrEq(pcmd, "bot_nav_delete"))
	{
		m_editCmd = EDIT_DELETE;
	}
	else if (FStrEq(pcmd, "bot_nav_split"))
	{
		m_editCmd = EDIT_SPLIT;
	}
	else if (FStrEq(pcmd, "bot_nav_merge"))
	{
		m_editCmd = EDIT_MERGE;
	}
	else if (FStrEq(pcmd, "bot_nav_mark"))
	{
		m_editCmd = EDIT_MARK;
	}
	else if (FStrEq(pcmd, "bot_nav_begin_area"))
	{
		m_editCmd = EDIT_BEGIN_AREA;
	}
	else if (FStrEq(pcmd, "bot_nav_end_area"))
	{
		m_editCmd = EDIT_END_AREA;
	}
	else if (FStrEq(pcmd, "bot_nav_connect"))
	{
		m_editCmd = EDIT_CONNECT;
	}
	else if (FStrEq(pcmd, "bot_nav_disconnect"))
	{
		m_editCmd = EDIT_DISCONNECT;
	}
	else if (FStrEq(pcmd, "bot_nav_splice"))
	{
		m_editCmd = EDIT_SPLICE;
	}
	else if (FStrEq(pcmd, "bot_nav_crouch"))
	{
		m_editCmd = EDIT_ATTRIB_CROUCH;
	}
	else if (FStrEq(pcmd, "bot_nav_jump"))
	{
		m_editCmd = EDIT_ATTRIB_JUMP;
	}
	else if (FStrEq(pcmd, "bot_nav_precise"))
	{
		m_editCmd = EDIT_ATTRIB_PRECISE;
	}
	else if (FStrEq(pcmd, "bot_nav_no_jump"))
	{
		m_editCmd = EDIT_ATTRIB_NO_JUMP;
	}
	else if (FStrEq(pcmd, "bot_nav_analyze"))
	{
		m_isAnalysisRequested = true;
	}
	else if (FStrEq(pcmd, "bot_nav_strip"))
	{
		StripNavigationAreas();
	}
	else if (FStrEq(pcmd, "bot_nav_save"))
	{
		GET_GAME_DIR(buffer);
		Q_strcat(buffer, "\\");
		Q_strcat(buffer, CBotManager::GetNavMapFilename());

		if (SaveNavigationMap(buffer))
			CONSOLE_ECHO("Navigation map '%s' saved.\n", buffer);
		else
			CONSOLE_ECHO("ERROR: Cannot save navigation map '%s'.\n", buffer);
	}
	else if (FStrEq(pcmd, "bot_nav_load"))
	{
		ValidateMapData();
	}
	else if (FStrEq(pcmd, "bot_nav_use_place"))
	{
		if (CMD_ARGC() == 1)
		{
			// no arguments = list all available places
			int i = 0;
			const BotPhraseList *placeList = TheBotPhrases->GetPlaceList();
			FOR_EACH_LL ((*placeList), it)
			{
				const BotPhrase *phrase = (*placeList)[it];

				if (phrase->GetID() == GetNavPlace())
					CONSOLE_ECHO("--> %-26s", phrase->GetName());
				else
					CONSOLE_ECHO("%-30s", phrase->GetName());

				if (!(i % 3))
					CONSOLE_ECHO("\n");
			}
			CONSOLE_ECHO("\n");
		}
		else
		{
			// single argument = set current place
			const BotPhraseList *placeList = TheBotPhrases->GetPlaceList();
			const BotPhrase *found = NULL;
			bool isAmbiguous = false;
			FOR_EACH_LL ((*placeList), it)
			{
				const BotPhrase *phrase = (*placeList)[it];
				if (!Q_strnicmp(phrase->GetName(), msg, Q_strlen(msg)))
				{
					// check for exact match in case of subsets of other strings
					if (!Q_strcmp(phrase->GetName(), msg))
					{
						found = phrase;
						isAmbiguous = false;
						break;
					}

					if (found != NULL)
					{
						isAmbiguous = true;
					}
					else
					{
						found = phrase;
					}
				}
			}

			if (isAmbiguous)
			{
				CONSOLE_ECHO("Ambiguous\n");
			}
			else
			{
				CONSOLE_ECHO("Current place set to '%s'\n", found->GetName());
				m_navPlace = found->GetID();
			}
		}
	}
	else if (FStrEq(pcmd, "bot_nav_toggle_place_mode"))
	{
		m_editCmd = EDIT_TOGGLE_PLACE_MODE;
	}
	else if (FStrEq(pcmd, "bot_nav_place_floodfill"))
	{
		m_editCmd = EDIT_PLACE_FLOODFILL;
	}
	else if (FStrEq(pcmd, "bot_nav_place_pick"))
	{
		m_editCmd = EDIT_PLACE_PICK;
	}
	else if (FStrEq(pcmd, "bot_nav_toggle_place_painting"))
	{
		m_editCmd = EDIT_TOGGLE_PLACE_PAINTING;
	}
	else if (FStrEq(pcmd, "bot_goto_mark"))
	{
		// tell the first bot we find to go to our marked area
		CNavArea *area = GetMarkedArea();
		if (area != NULL)
		{
			CBaseEntity *pEntity = NULL;
			while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
			{
				if (!pEntity->IsPlayer())
					continue;

				if ((pEntity->pev->flags & FL_DORMANT) == FL_DORMANT)
					continue;

				CBasePlayer *playerOrBot = GetClassPtr<CBasePlayer>(pEntity->pev);

				if (playerOrBot->IsBot())
				{
					CCSBot *bot = static_cast<CCSBot *>(playerOrBot);
					if (bot != NULL)
					{
						bot->MoveTo(&area->m_center, FASTEST_ROUTE);
					}

					break;
				}
			}
		}
	}
	else if (FStrEq(pcmd, "bot_memory_usage"))
	{
		CONSOLE_ECHO("Memory usage:\n");
		CONSOLE_ECHO("  %d bytes per bot\b", sizeof(CCSBot));
		CONSOLE_ECHO("  %d Navigation Areas @ %d bytes each = %d bytes\n",
			TheNavAreaGrid.GetNavAreaCount(),
			sizeof(CNavArea),
			TheNavAreaGrid.GetNavAreaCount() * sizeof(CNavArea));
		CONSOLE_ECHO("  %d Hiding Spots @ %d bytes each = %d bytes\n",
			TheHidingSpotList.Count(),
			sizeof(HidingSpot),
			sizeof(HidingSpot) * TheHidingSpotList.Count());

		unsigned int encounterMem = 0;
		FOR_EACH_LL (TheNavAreaList, it)
		{
			CNavArea *area = TheNavAreaList[it];

			FOR_EACH_LL (area->m_spotEncounterList, it2)
			{
				SpotEncounter *se = area->m_spotEncounterList[it2];

				encounterMem += sizeof(SpotEncounter);
				encounterMem += sizeof(SpotOrder) * se->spotList.Count();
			}
		}

		CONSOLE_ECHO("  Encounter Spot data = %d bytes\n", encounterMem);
	}
	else if (FStrEq(pcmd, "bot_nav_mark_unnamed"))
	{
		m_editCmd = EDIT_MARK_UNNAMED;
	}
	else if (FStrEq(pcmd, "bot_nav_warp"))
	{
		m_editCmd = EDIT_WARP_TO_MARK;
	}
	else if (FStrEq(pcmd, "bot_nav_corner_select"))
	{
		m_editCmd = EDIT_SELECT_CORNER;
	}
	else if (FStrEq(pcmd, "bot_nav_corner_raise"))
	{
		m_editCmd = EDIT_RAISE_CORNER;
	}
	else if (FStrEq(pcmd, "bot_nav_corner_lower"))
	{
		m_editCmd = EDIT_LOWER_CORNER;
	}
	else if (FStrEq(pcmd, "bot_nav_check_consistency"))
	{
		if (CMD_ARGC() != 2)
		{
			CONSOLE_ECHO("usage: bot_nav_check_consistency <filename>\n");
			return;
		}

		SanityCheckNavigationMap(msg);
	}
}

BOOL CCSBotManager::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	return FALSE;
}

// Process the "bot_add" console command

bool CCSBotManager::BotAddCommand(BotProfileTeamType team, bool isFromConsole)
{
	// dont allow bots to join if the Navigation Area is being generated
	if (m_isLearningMap)
		return false;

	if (!g_bEnableCSBot)
		return false;

	const BotProfile *profile = NULL;

	if (!isFromConsole || CMD_ARGC() < 2)
	{
		// if team not specified, check cv_bot_join_team cvar for preference
		if (team == BOT_TEAM_ANY)
		{
			if (!Q_stricmp(cv_bot_join_team.string, "T"))
				team = BOT_TEAM_T;

			else if (!Q_stricmp(cv_bot_join_team.string, "CT"))
				team = BOT_TEAM_CT;
			else
			{
				TeamName defaultTeam = SelectDefaultTeam();

				if (defaultTeam == TERRORIST)
					team = BOT_TEAM_T;

				else if (defaultTeam == CT)
					team = BOT_TEAM_CT;
			}
		}

		// try to add a bot by name
		profile = TheBotProfiles->GetRandomProfile(GetDifficultyLevel(), team);

		if (profile == NULL)
		{
			CONSOLE_ECHO("All bot profiles at this difficulty level are in use.\n");
			return true;
		}
	}
	else
	{
		// in career, ignore humans
		bool ignoreHumans = false;
		CHalfLifeMultiplay *mp = g_pGameRules;

		if (mp != NULL && mp->IsCareer())
			ignoreHumans = true;

		if (UTIL_IsNameTaken(CMD_ARGV(1), ignoreHumans))
		{
			CONSOLE_ECHO("Error - %s is already in the game.\n", CMD_ARGV(1));
			return true;
		}

		profile = TheBotProfiles->GetProfile(CMD_ARGV(1), team);
		if (profile == NULL)
		{
			CONSOLE_ECHO("Error - no profile for '%s' exists.\n", CMD_ARGV(1));
			return true;
		}
	}

	// create the bot
	if (AddBot(profile, team))
	{
		if (isFromConsole)
		{
			// increase the bot quota to account for manually added bot
			CVAR_SET_FLOAT("bot_quota", cv_bot_quota.value + 1);
		}
	}

	return true;
}

// Keep a minimum quota of bots in the game

void CCSBotManager::MaintainBotQuota()
{
	if (m_isLearningMap)
		return;

	CHalfLifeMultiplay *mp = g_pGameRules;
	int totalHumansInGame = UTIL_HumansInGame();
	int humanPlayersInGame = UTIL_HumansInGame(IGNORE_SPECTATORS);

	// don't add bots until local player has been registered, to make sure he's player ID #1
	if (!IS_DEDICATED_SERVER() && totalHumansInGame == 0)
		return;

	int desiredBotCount = (int)cv_bot_quota.value;
	int botsInGame = UTIL_BotsInGame();

	if (cv_bot_quota_match.value > 0.0)
	{
		desiredBotCount = (int)(humanPlayersInGame * cv_bot_quota_match.value);
	}

	// wait for a player to join, if necessary
	if (cv_bot_join_after_player.value > 0.0)
	{
		if (humanPlayersInGame == 0)
			desiredBotCount = 0;
	}

	// if bots will auto-vacate, we need to keep one slot open to allow players to join
	if (cv_bot_auto_vacate.value > 0.0)
		desiredBotCount = Q_min(desiredBotCount, gpGlobals->maxClients - (totalHumansInGame + 1));
	else
		desiredBotCount = Q_min(desiredBotCount, gpGlobals->maxClients - totalHumansInGame);

	// add bots if necessary
	if (desiredBotCount > botsInGame)
	{
		// don't try to add a bot if all teams are full
		if (!mp->TeamFull(TERRORIST) || !mp->TeamFull(CT))
			BotAddCommand(BOT_TEAM_ANY);
	}
	else if (desiredBotCount < botsInGame)
	{
		// kick a bot to maintain quota

		// first remove any unassigned bots
		if (UTIL_KickBotFromTeam(UNASSIGNED))
			return;

		TeamName kickTeam;

		// remove from the team that has more players
		if (mp->m_iNumTerrorist > mp->m_iNumCT)
		{
			kickTeam = TERRORIST;
		}
		else if (mp->m_iNumTerrorist < mp->m_iNumCT)
		{
			kickTeam = CT;
		}
		// remove from the team that's winning
		else if (mp->m_iNumTerroristWins > mp->m_iNumCTWins)
		{
			kickTeam = TERRORIST;
		}
		else if (mp->m_iNumCTWins > mp->m_iNumTerroristWins)
		{
			kickTeam = CT;
		}
		else
		{
			// teams and scores are equal, pick a team at random
			kickTeam = (RANDOM_LONG(0, 1) == 0) ? CT : TERRORIST;
		}

		// attempt to kick a bot from the given team
		if (UTIL_KickBotFromTeam(kickTeam))
			return;

		// if there were no bots on the team, kick a bot from the other team
		if (kickTeam == TERRORIST)
			UTIL_KickBotFromTeam(CT);
		else
			UTIL_KickBotFromTeam(TERRORIST);
	}
	else
	{
		if (mp != NULL && !mp->IsCareer())
			return;

		bool humansAreCTs = (Q_strcmp(humans_join_team.string, "CT") == 0);

		if (humansAreCTs)
		{
			if (mp->m_iNumCT <= 6)
				return;

			UTIL_KickBotFromTeam(CT);
		}
		else
		{
			if (mp->m_iNumTerrorist <= 6)
				return;

			UTIL_KickBotFromTeam(TERRORIST);
		}

		CVAR_SET_FLOAT("bot_quota", cv_bot_quota.value - 1.0f);
	}
}

void CCSBotManager::MonitorBotCVars()
{
	if (cv_bot_nav_edit.value != 0.0f)
	{
		EditNavAreas(m_editCmd);
		m_editCmd = EDIT_NONE;
	}

	if (gpGlobals->time >= m_flNextCVarCheck)
	{
		if (cv_bot_show_danger.value != 0.0f)
			DrawDanger();

		MaintainBotQuota();
		m_flNextCVarCheck = gpGlobals->time + 0.3f;
	}
}

// Collect all nav areas that overlap the given zone
class CollectOverlappingAreas
{
public:
	CollectOverlappingAreas(CCSBotManager::Zone *zone)
	{
		m_zone = zone;
		zone->m_areaCount = 0;
	}
	bool operator()(CNavArea *area)
	{
		const Extent *areaExtent = area->GetExtent();

		if (areaExtent->hi.x >= m_zone->m_extent.lo.x && areaExtent->lo.x <= m_zone->m_extent.hi.x
			&& areaExtent->hi.y >= m_zone->m_extent.lo.y && areaExtent->lo.y <= m_zone->m_extent.hi.y
			&& areaExtent->hi.z >= m_zone->m_extent.lo.z && areaExtent->lo.z <= m_zone->m_extent.hi.z)
		{
			// area overlaps m_zone
			m_zone->m_area[ m_zone->m_areaCount++ ] = area;
			if (m_zone->m_areaCount == CCSBotManager::MAX_ZONE_NAV_AREAS)
			{
				return false;
			}
		}

		return true;
	}

private:
	CCSBotManager::Zone *m_zone;
};

// Search the map entities to determine the game scenario and define important zones.

void CCSBotManager::ValidateMapData()
{
	if (m_isMapDataLoaded || !g_bEnableCSBot)
		return;

	m_isMapDataLoaded = true;

	if (LoadNavigationMap())
	{
		CONSOLE_ECHO("Failed to load navigation map.\n");
		return;
	}

	CONSOLE_ECHO("Navigation map loaded.\n");

	m_zoneCount = 0;
	m_gameScenario = SCENARIO_DEATHMATCH;

	// Search all entities in the map and set the game type and
	// store all zones (bomb target, etc).

	CBaseEntity *entity = NULL;
	int i;
	for (i = 1; i < gpGlobals->maxEntities; ++i)
	{
		entity = CBaseEntity::Instance(INDEXENT(i));

		if (entity == NULL)
			continue;

		bool found = false;
		bool isLegacy = false;

		if (FClassnameIs(entity->pev, "func_bomb_target"))
		{
			m_gameScenario = SCENARIO_DEFUSE_BOMB;
			found = true;
			isLegacy = false;
		}
		else if (FClassnameIs(entity->pev, "info_bomb_target"))
		{
			m_gameScenario = SCENARIO_DEFUSE_BOMB;
			found = true;
			isLegacy = true;
		}
		else if (FClassnameIs(entity->pev, "func_hostage_rescue"))
		{
			m_gameScenario = SCENARIO_RESCUE_HOSTAGES;
			found = true;
			isLegacy = false;
		}
		else if (FClassnameIs(entity->pev, "info_hostage_rescue"))
		{
			m_gameScenario = SCENARIO_RESCUE_HOSTAGES;
			found = true;
			isLegacy = true;
		}
		else if (FClassnameIs(entity->pev, "hostage_entity"))
		{
			// some very old maps (ie: cs_assault) use info_player_start
			// as rescue zones, so set the scenario if there are hostages
			// in the map
			m_gameScenario = SCENARIO_RESCUE_HOSTAGES;
		}
		else if (FClassnameIs(entity->pev, "func_vip_safetyzone"))
		{
			m_gameScenario = SCENARIO_ESCORT_VIP;
			found = true;
			isLegacy = false;
		}

		if (found)
		{
			if (m_zoneCount < MAX_ZONES)
			{
				m_zone[ m_zoneCount ].m_center = (isLegacy) ? entity->pev->origin : (entity->pev->absmax + entity->pev->absmin) / 2.0f;
				m_zone[ m_zoneCount ].m_isLegacy = isLegacy;
				m_zone[ m_zoneCount ].m_index = m_zoneCount;
				m_zone[ m_zoneCount++ ].m_entity = entity;
			}
			else
				CONSOLE_ECHO("Warning: Too many zones, some will be ignored.\n");
		}
	}

	// If there are no zones and the scenario is hostage rescue,
	// use the info_player_start entities as rescue zones.
	if (m_zoneCount == 0 && m_gameScenario == SCENARIO_RESCUE_HOSTAGES)
	{
		entity = NULL;

		while ((entity = UTIL_FindEntityByClassname(entity, "info_player_start")) != NULL)
		{
			if (FNullEnt(entity->edict()))
				break;

			if (m_zoneCount < MAX_ZONES)
			{
				m_zone[ m_zoneCount ].m_center = entity->pev->origin;
				m_zone[ m_zoneCount ].m_isLegacy = true;
				m_zone[ m_zoneCount ].m_index = m_zoneCount;
				m_zone[ m_zoneCount++ ].m_entity = entity;
			}
			else
				CONSOLE_ECHO("Warning: Too many zones, some will be ignored.\n");
		}
	}

	// Collect nav areas that overlap each zone
	for (i = 0; i < m_zoneCount; ++i)
	{
		Zone *zone = &m_zone[i];

		if (zone->m_isLegacy)
		{
			const float legacyRange = 256.0f;
			zone->m_extent.lo.x = zone->m_center.x - legacyRange;
			zone->m_extent.lo.y = zone->m_center.y - legacyRange;
			zone->m_extent.lo.z = zone->m_center.z - legacyRange;
			zone->m_extent.hi.x = zone->m_center.x + legacyRange;
			zone->m_extent.hi.y = zone->m_center.y + legacyRange;
			zone->m_extent.hi.z = zone->m_center.z + legacyRange;
		}
		else
		{
			zone->m_extent.lo = zone->m_entity->pev->absmin;
			zone->m_extent.hi = zone->m_entity->pev->absmax;
		}

		// ensure Z overlap
		const float zFudge = 50.0f;
		zone->m_extent.lo.z -= zFudge;
		zone->m_extent.hi.z += zFudge;

		// build a list of nav areas that overlap this zone
		CollectOverlappingAreas collector(zone);
		ForAllAreas(collector);
	}
}

bool CCSBotManager::AddBot(const BotProfile *profile, BotProfileTeamType team)
{
	if (!g_bEnableCSBot)
		return false;

	CHalfLifeMultiplay *mp = g_pGameRules;

	int nTeamSlot = UNASSIGNED;

	if (team == BOT_TEAM_ANY)
	{
		// if team not specified, check cv_bot_join_team cvar for preference
		if (!Q_stricmp(cv_bot_join_team.string, "T"))
			nTeamSlot = TERRORIST;

		else if (!Q_stricmp(cv_bot_join_team.string, "CT"))
			nTeamSlot = CT;
	}
	else if (team == BOT_TEAM_CT)
		nTeamSlot = CT;

	else if (team == BOT_TEAM_T)
		nTeamSlot = TERRORIST;

	if (nTeamSlot == UNASSIGNED)
	{
		nTeamSlot = SelectDefaultTeam();
	}

	if (nTeamSlot == UNASSIGNED || mp->TeamFull(nTeamSlot))
	{
		CONSOLE_ECHO("Could not add bot to the game: Team is full\n");
		return false;
	}

	if (mp->TeamStacked(nTeamSlot, UNASSIGNED))
	{
		CONSOLE_ECHO("Could not add bot to the game: Team is stacked (to disable this check, set mp_limitteams and mp_autoteambalance to zero and restart the round).\n");
		return false;
	}

	CCSBot *pBot = CreateBot<CCSBot>(profile);
	if (pBot == NULL)
	{
		return false;
	}

	//int nJoinedTeam;
	ClientPutInServer(pBot->edict());
	SET_CLIENT_KEY_VALUE(pBot->entindex(), GET_INFO_BUFFER(pBot->edict()), "*bot", "1");

	pBot->m_iMenu = Menu_ChooseTeam;
	pBot->m_iJoiningState = PICKINGTEAM;

	if (HandleMenu_ChooseTeam(pBot, nTeamSlot))
	{
		int skin = profile->GetSkin();

		if (!skin)
			skin = 6;// MODEL_GIGN?

		HandleMenu_ChooseAppearance(pBot, skin);

		if (IS_DEDICATED_SERVER())
		{
			UTIL_DPrintf("Added bot %s to server\n", STRING(pBot->pev->netname));
		}

		return true;
	}

	SERVER_COMMAND(UTIL_VarArgs("kick \"%s\"\n", STRING(pBot->pev->netname)));
	CONSOLE_ECHO("Could not add bot to the game.\n");

	return false;
}

// Return the zone that contains the given position

const CCSBotManager::Zone *CCSBotManager::GetZone(const Vector *pos) const
{
	for (int z = 0; z < m_zoneCount; ++z)
	{
		if (m_zone[z].m_extent.Contains(pos))
		{
			return &m_zone[z];
		}
	}

	return NULL;
}

// Return the closest zone to the given position

const CCSBotManager::Zone *CCSBotManager::GetClosestZone(const Vector *pos) const
{
	const Zone *close = NULL;
	float closeRangeSq = 1e9f;

	for (int z = 0; z < m_zoneCount; ++z)
	{
		float rangeSq = (m_zone[z].m_center - (*pos)).LengthSquared();

		if (rangeSq < closeRangeSq)
		{
			closeRangeSq = rangeSq;
			close = &m_zone[z];
		}
	}

	return close;
}

// Return a random position inside the given zone

const Vector *CCSBotManager::GetRandomPositionInZone(const Zone *zone) const
{
	static Vector pos;

	if (zone == NULL)
		return NULL;

	if (zone->m_areaCount == 0)
		return NULL;

	// pick a random overlapping area
	CNavArea *area = GetRandomAreaInZone(zone);

	// pick a location inside both the nav area and the zone
	// TODO: Randomize this
	if (zone->m_isLegacy)
	{
		// TODO: It is possible that the radius might not overlap this area at all...
		area->GetClosestPointOnArea(&zone->m_center, &pos);
	}
	else
	{
		const Extent &areaExtent = *area->GetExtent();
		Extent overlap;
		overlap.lo.x = Q_max(areaExtent.lo.x, zone->m_extent.lo.x);
		overlap.lo.y = Q_max(areaExtent.lo.y, zone->m_extent.lo.y);
		overlap.hi.x = Q_min(areaExtent.hi.x, zone->m_extent.hi.x);
		overlap.hi.y = Q_min(areaExtent.hi.y, zone->m_extent.hi.y);

		pos.x = (overlap.lo.x + overlap.hi.x) / 2.0f;
		pos.y = (overlap.lo.y + overlap.hi.y) / 2.0f;
		pos.z = area->GetZ(&pos);
	}

	return &pos;
}

// Return a random area inside the given zone

CNavArea *CCSBotManager::GetRandomAreaInZone(const Zone *zone) const
{
	// TODO: improvement is needed
	if (!zone->m_areaCount)
		return NULL;

	return zone->m_area[ RANDOM_LONG(0, zone->m_areaCount - 1) ];
}

void CCSBotManager::OnEvent(GameEventType event, CBaseEntity *entity, CBaseEntity *other)
{
	switch (event)
	{
	case EVENT_BOMB_PLANTED:
		m_isBombPlanted = true;
		m_bombPlantTimestamp = gpGlobals->time;
		break;

	case EVENT_BOMB_DEFUSING:
		m_bombDefuser = (CBasePlayer *)entity;
		break;

	case EVENT_BOMB_DEFUSE_ABORTED:
		m_bombDefuser = NULL;
		break;

	case EVENT_BOMB_DEFUSED:
		m_isBombPlanted = false;
		m_bombDefuser = NULL;
		break;

	case EVENT_TERRORISTS_WIN:
	case EVENT_CTS_WIN:
	case EVENT_ROUND_DRAW:
		m_isRoundOver = true;
		break;

	case EVENT_RADIO_ENEMY_SPOTTED:
		m_lastSeenEnemyTimestamp = gpGlobals->time;
		SetLastSeenEnemyTimestamp();
		break;

	default:
		break;
	}

	CBotManager::OnEvent(event, entity, other);
}

// Get the time remaining before the planted bomb explodes

float CCSBotManager::GetBombTimeLeft() const
{
	return (g_pGameRules->m_iC4Timer - (gpGlobals->time - m_bombPlantTimestamp));
}

void CCSBotManager::SetLooseBomb(CBaseEntity *bomb)
{
	m_looseBomb = bomb;

	if (bomb)
	{
		m_looseBombArea = TheNavAreaGrid.GetNearestNavArea(&bomb->pev->origin);
	}
	else
	{
		m_looseBombArea = NULL;
	}
}

// Return true if player is important to scenario (VIP, bomb carrier, etc)

bool CCSBotManager::IsImportantPlayer(CBasePlayer *player) const
{
	switch (GetScenario())
	{
		case SCENARIO_DEFUSE_BOMB:
		{
			if (player->m_iTeam == TERRORIST && player->IsBombGuy())
				return true;

			// TODO: CT's defusing the bomb are important
			return false;
		}
		case SCENARIO_ESCORT_VIP:
		{
			if (player->m_iTeam == CT && player->m_bIsVIP)
				return true;

			return false;
		}
		case SCENARIO_RESCUE_HOSTAGES:
		{
			// TODO: CT's escorting hostages are important
			return false;
		}
		default:
			break;
	}

	// everyone is equally important in a deathmatch
	return false;
}

// Return priority of player (0 = max pri)

unsigned int CCSBotManager::GetPlayerPriority(CBasePlayer *player) const
{
	const unsigned int lowestPriority = 0xFFFFFFFF;

	if (!player->IsPlayer())
		return lowestPriority;

	// human players have highest priority
	if (!player->IsBot())
		return 0;

	CCSBot *bot = dynamic_cast<CCSBot *>(player);

	if (!bot)
		return 0;

	// bots doing something important for the current scenario have high priority
	switch (GetScenario())
	{
		case SCENARIO_DEFUSE_BOMB:
		{
			// the bomb carrier has high priority
			if (bot->m_iTeam == TERRORIST && bot->m_bHasC4)
				return 1;

			break;
		}
		case SCENARIO_ESCORT_VIP:
		{
			// the VIP has high priority
			if (bot->m_iTeam == CT && bot->m_bIsVIP)
				return 1;

			break;
		}
		case SCENARIO_RESCUE_HOSTAGES:
		{
			// CT's rescuing hostages have high priority
			if (bot->m_iTeam == CT && bot->GetHostageEscortCount())
				return 1;

			break;
		}
		default:
			break;
	}

	// everyone else is ranked by their unique ID (which cannot be zero)
	return 1 + bot->GetID();
}

// Return the last time the given radio message was sent for given team
// 'teamID' can be CT or TERRORIST

float CCSBotManager::GetRadioMessageTimestamp(GameEventType event, int teamID) const
{
	if (event <= EVENT_START_RADIO_1 || event >= EVENT_END_RADIO)
		return 0.0f;

	int i = (teamID == TERRORIST) ? 0 : 1;
	return m_radioMsgTimestamp[ event - EVENT_START_RADIO_1 ][ i ];
}

// Return the interval since the last time this message was sent

float CCSBotManager::GetRadioMessageInterval(GameEventType event, int teamID) const
{
	if (event <= EVENT_START_RADIO_1 || event >= EVENT_END_RADIO)
		return 99999999.9f;

	int i = (teamID == TERRORIST) ? 0 : 1;
	return gpGlobals->time - m_radioMsgTimestamp[ event - EVENT_START_RADIO_1 ][ i ];
}

// Set the given radio message timestamp.
// 'teamID' can be CT or TERRORIST

void CCSBotManager::SetRadioMessageTimestamp(GameEventType event, int teamID)
{
	if (event <= EVENT_START_RADIO_1 || event >= EVENT_END_RADIO)
		return;

	int i = (teamID == TERRORIST) ? 0 : 1;
	m_radioMsgTimestamp[ event - 1 ][ i ] = gpGlobals->time;
}

// Reset all radio message timestamps

void CCSBotManager::ResetRadioMessageTimestamps()
{
	for (size_t t = 0; t < ARRAYSIZE(m_radioMsgTimestamp[0]); ++t)
	{
		for (size_t m = 0; m < ARRAYSIZE(m_radioMsgTimestamp); ++m)
		{
			m_radioMsgTimestamp[m][t] = 0.0f;
		}
	}
}
