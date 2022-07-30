
#include "extdll.h"
#include "common.h"
#include "util.h"
#include "cbase.h"

#include <string>
#include <sstream>
#include "game_shared/steam_util.h"
//#include <algorithm>
//#include "game_shared/bot/nav_area.h"
#include <unordered_map>

namespace sv {

unsigned int MapMgr_GetRandomAreaID();

struct MapData
{
	int map_id;
	std::string map_name;
	char map_filename[32];
	/*int map_original;
	int map_deathmode;
	int map_teamdeathmode;
	int map_bot_original;
	int map_bot_deathmode;
	int map_bot_teamdeathmod;
	int official;
	int official_tiebreak;
	int map_zombi;
	int weapon_restrict_deault;
	int weapon_restrict_none;
	int weapon_restrict_noawp;
	int weapon_resctrict_classic;
	int weapon_restrict_sniper;
	int weapon_restrict_pistol;
	int weapon_restrict_knife;
	int weapon_restrict_noflash;
	int weapon_restrict_awp;*/
	char bright[32];
	char skybox[32];
	float density;
	float density_r;
	float density_g;
	float density_b;
	/*int server_limit;
	int max_player;
	char d3d_bright;
	char c4pos[32];
	int map_zombi_expand;
	int ExpLimit;
	int PointLimit;
	int event_map;
	int weapon_restrict_fragonly;
	int map_zombi_team;
	char ZBTbright;
	char ZBTskybox[32];
	int ZBTdensity;
	int ZBTdensity_r;
	int ZBTdensity_g;
	int ZBTdensity_b;
	char ZBTd3d_bright;
	int map_challenge;
	int map_zombi_team_ann;
	int map_zombi_3;
	int map_zombie_survival;
	int Scen_CountType_1ST;
	int Scen_CountType_2ND;
	int Scen_CountType_3RD;
	int Scen_Rank_Clear;
	int map_sort;
	int ExtraInfo;
	int map_soccer;
	int ball_max;
	int ball_default;
	int map_human_scenario;
	int SubType;
	int PVPVE;
	int Orginal_Mission;
	int Weapon_Tag;
	int map_FunMode;
	int Fun_Subtype;
	int map_GDM;
	int map_Basic;
	int map_BZM;
	int film_script;
	int Vulnerability_analyzer;
	int map_BattleRush;
	int map_ZombieShelter;
	int map_ZombieEscape;
	int map_Snowman;
	int Snowman_Subtype;
	int map_shelterteam;
	int Zombie_PVE_recovery_rate_applicable_map;
	int map_Zombie4;
	int map_matching;
	int map_ZombieGiant;
	int map_Zombie_Exterminate;
	int map_matching_Zombie_Exterminate;
	int map_standalone;
	int Single_PVE_achievement_rate_target;
	int weapon_restrict_shotgun;
	int Zombieofficial;
	int Zombieofficial_tiebreak;
	int ZombieTag;
	int ModeEventMap;
	int allstar;
	int playroom;
	int season_original;
	int season_zombie_ex;
	int season_zombie_hero;
	int map_zombi_3z;
	int playroom_modeID;
	int map_zombietouchdown;
	int season_touchdown;
	int prop_hunt;
	int map_partner;
	int map_zhe;
	int vxlzshelter;
	int zombi_5;
	int ZSmaxDifficulty;
	int scenariotx;*/
	int randomrespawn;
};
// same map_filename but different map_id
std::unordered_map<std::string, MapData> g_MapData;

struct MapSafeSectorData
{
	unsigned int ratio;
	std::vector<unsigned int> area;
};
typedef std::unordered_map<unsigned int, MapSafeSectorData> MapSafeSector;
std::unordered_map<int, MapSafeSector> g_MapSafeArea;

int MapMgr_GetMapID(const char *szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return -1;

	return g_MapData[std::string(szMapName)].map_id;
}

char *MapMgr_GetMapBright(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return NULL;

	return g_MapData[std::string(szMapName)].bright;
}

char* MapMgr_GetMapSkybox(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return NULL;

	return g_MapData[std::string(szMapName)].skybox;
}

float MapMgr_GetMapDensity(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return 0.00006;

	return g_MapData[std::string(szMapName)].density;
}

float MapMgr_GetMapDensity_r(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return 0;

	return g_MapData[std::string(szMapName)].density_r;
}

float MapMgr_GetMapDensity_g(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return 0;

	return g_MapData[std::string(szMapName)].density_g;
}

float MapMgr_GetMapDensity_b(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return 0;

	return g_MapData[std::string(szMapName)].density_b;
}

int MapMgr_GetMapRandomRespawn(const char* szMapName)
{
	if (g_MapData.find(std::string(szMapName)) == g_MapData.end())
		return 0;

	return g_MapData[std::string(szMapName)].randomrespawn;
}

void MapMgr_LoadMapSafeArea()
{
	SteamFile file("resource/zombi/zombie_map_safearea.csv");

	if (!file.IsValid())
		return;

	auto readline = [](SteamFile& sf) {
		std::string ret;
		char ch = '\0';
		while (sf.Read((void*)&ch, sizeof(char)))
		{
			if (ch == '\n')
				return std::make_pair(true, ret);
			ret.push_back(ch);
		}
		return std::make_pair(false, ret);
	};

	bool bFirstLine = true;
	std::pair<bool, std::string> linedata;
	while ((linedata = readline(file)).first)
	{
		if (bFirstLine) {
			bFirstLine = false;
			continue;
		}
		std::istringstream ss(linedata.second);
		std::string data;

		std::getline(ss, data, ',');
		int map_id = atoi(data.c_str());

		std::getline(ss, data, ',');
		unsigned int area = atoi(data.c_str());

		std::getline(ss, data, ',');
		unsigned int sector = atoi(data.c_str());

		std::getline(ss, data, ',');
		unsigned int ratio = atoi(data.c_str());

		if (g_MapSafeArea.find(map_id) == g_MapSafeArea.end())
		{
			MapSafeSector mapList;
			g_MapSafeArea[map_id] = mapList;
		}
		if (g_MapSafeArea[map_id].find(sector) == g_MapSafeArea[map_id].end())
		{
			MapSafeSectorData sectorData;
			g_MapSafeArea[map_id][sector] = sectorData;
		}

		g_MapSafeArea[map_id][sector].area.emplace_back(area);
		g_MapSafeArea[map_id][sector].ratio = ratio;
	}
}

unsigned int MapMgr_GetRandomAreaID()
{
	int map_id = MapMgr_GetMapID(STRING(gpGlobals->mapname));
	if (g_MapSafeArea.find(map_id) == g_MapSafeArea.end())
		return -2;
	
	std::vector<std::pair<unsigned int, MapSafeSectorData>> list{ g_MapSafeArea[map_id].begin(), g_MapSafeArea[map_id].end() };

	std::random_device rd;
	std::shuffle(list.begin(), list.end(), rd);
	std::shuffle(list.front().second.area.begin(), list.front().second.area.end(), rd);

	return list.front().second.area.front();
	/*auto iter = std::find_if(list.front().second.area.begin(), list.front().second.area.end(),
		[](const unsigned int id) {
			return TheNavAreaGrid.GetNavAreaByID(id) != NULL;
		});
	if (iter == list.front().second.area.end())
		return -1;

	return *iter;*/
}

void MapMgr_LoadMapList()
{
	SteamFile file("resource/MapList.csv");

	if (!file.IsValid())
		return;

	auto readline = [](SteamFile& sf) {
		std::string ret;
		char ch = '\0';
		while (sf.Read((void*)&ch, sizeof(char)))
		{
			if (ch == '\n')
				return std::make_pair(true, ret);
			ret.push_back(ch);
		}
		return std::make_pair(false, ret);
	};

	bool bFirstLine = true;
	std::pair<bool, std::string> linedata;
	while ((linedata = readline(file)).first)
	{
		if (bFirstLine) {
			bFirstLine = false;
			continue;
		}
		MapData map;
		std::istringstream ss(linedata.second);
		std::string data;

		std::getline(ss, data, ',');
		map.map_id = atoi(data.c_str());

		std::getline(ss, data, ',');
		map.map_name = data;

		std::getline(ss, data, ',');
		Q_strncpy(map.map_filename, data.c_str(), sizeof(map.map_filename));

		std::getline(ss, data, ',');
		Q_strncpy(map.bright, data.c_str(), sizeof(map.bright));

		std::getline(ss, data, ',');
		Q_strncpy(map.skybox, data.c_str(), sizeof(map.skybox));

		std::getline(ss, data, ',');
		map.density = atof(data.c_str());

		std::getline(ss, data, ',');
		map.density_r = atof(data.c_str());

		std::getline(ss, data, ',');
		map.density_g = atof(data.c_str());

		std::getline(ss, data, ',');
		map.density_b = atof(data.c_str());

		std::getline(ss, data, ',');
		map.randomrespawn = atoi(data.c_str());

		data = map.map_filename;
		if (g_MapData.find(data) != g_MapData.end())
			continue;

		g_MapData[data] = map;
		// TODO
		/*for (int i = 0; i < 101; i++)
		{
			std::getline(ss, data, ',');
		}*/
	}
}
}
