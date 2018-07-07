// Translated from
// https://github.com/MoeMod/BTE-AMXX/blob/master/BTE%20Codename%20Z4E/z4e_random_spawn.sma
// or random CSDM .sma, IDK where this code came from...
// MoeMod 2018/7/1

#include "extdll.h"
#include "common.h"
#include "util.h"
#include "cbase.h"

#include "game_shared/steam_util.h"

#include <string>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>
#include <array>
#include <functional>

#define SPAWN_DATA_ORIGIN_X 0
#define SPAWN_DATA_ORIGIN_Y 1
#define SPAWN_DATA_ORIGIN_Z 2
#define SPAWN_DATA_ANGLES_X 3
#define SPAWN_DATA_ANGLES_Y 4
#define SPAWN_DATA_ANGLES_Z 5
#define SPAWN_DATA_V_ANGLES_X 6
#define SPAWN_DATA_V_ANGLES_Y 7
#define SPAWN_DATA_V_ANGLES_Z 8

struct SpawnPointData
{
	Vector origin;
	Vector angles;
	Vector v_angle;
};
std::vector<SpawnPointData> g_vecSpawnCSDM;

template<class Arr, size_t...Vals>
inline Vector PackVector_impl(const Arr &arr, size_t N, std::index_sequence<Vals...>)
{
	return { arr[N * 3 + Vals]... };
}
template<class Arr>
inline Vector PackVector(const Arr &arr, size_t N)
{
	return PackVector_impl(arr, N, std::make_index_sequence<3>());
}
SpawnPointData MakeSpawnPointData(const std::array<float, 9> &arr)
{
	return { PackVector(arr, 0), PackVector(arr, 1), PackVector(arr, 2) };
}

BOOL CSDM_IsSpawnPointValid(CBaseEntity *pPlayer, const SpawnPointData &data)
{
	CBaseEntity *ent = NULL;

	while ((ent = UTIL_FindEntityInSphere(ent, data.origin, 64)) != NULL)
	{
		// if ent is a client, don't spawn on 'em
		if (ent->IsPlayer() && ent != pPlayer)
			return FALSE;
	}

	return TRUE;
}

void CSDM_ApplyRandomSpawnPoint(CBaseEntity *pEntity, const SpawnPointData & data)
{
	pEntity->pev->origin = data.origin + Vector(0, 0, 1);
	pEntity->pev->velocity = g_vecZero;
	pEntity->pev->angles = data.angles;
	pEntity->pev->v_angle = data.v_angle;
}

bool CSDM_DoRandomSpawn(CBaseEntity *pEntity)
{
	if (g_vecSpawnCSDM.empty())
		return false;
	// randomize those fucking spawn points
	std::random_shuffle(g_vecSpawnCSDM.begin(), g_vecSpawnCSDM.end());
	using namespace std::placeholders;
	// find the first availble one
	auto iter = std::find_if(g_vecSpawnCSDM.begin(), g_vecSpawnCSDM.end(), std::bind(CSDM_IsSpawnPointValid, pEntity, _1));
	if (iter == g_vecSpawnCSDM.end())
		return false;

	// sets these item
	CSDM_ApplyRandomSpawnPoint(pEntity, *iter);
	return true;
}

void CSDM_LoadSpawnPoints()
{
	g_vecSpawnCSDM.clear();
	// Check for CSDM spawns of the current map
	char filename[256];
	Q_sprintf(filename, "addons/amxmodx/configs/csdm/%s.spawns.cfg", STRING(gpGlobals->mapname));

	SteamFile csdmFile(filename);

	if (!csdmFile.IsValid())
		return;

	auto readline = [](SteamFile &sf) {
		std::string ret;
		char ch = '\0';
		while (sf.Read((void *)&ch, sizeof(char)))
		{
			if (ch == '\n')
				return std::make_pair(true, ret);
			ret.push_back(ch);
		}
		return std::make_pair(false, ret);
	};

	std::pair<bool, std::string> linedata;
	while ((linedata = readline(csdmFile)).first)
	{
		std::array<float, 9> arr;
		std::copy_n(std::istream_iterator<float>(std::istringstream(linedata.second)), 9, std::begin(arr));
		g_vecSpawnCSDM.emplace_back(MakeSpawnPointData(arr));
	}
}

/*
stock load_spawns()
{
    // Check for CSDM spawns of the current map
    new cfgdir[32], mapname[32], filepath[100], linedata[64]
    get_configsdir(cfgdir, charsmax(cfgdir))
    get_mapname(mapname, charsmax(mapname))
    formatex(filepath, charsmax(filepath), "%s/csdm/%s.spawns.cfg", cfgdir, mapname)
    
    // Load CSDM spawns if present
    if (file_exists(filepath))
    {
        new csdmdata[10][6], file = fopen(filepath,"rt")
        
        while (file && !feof(file))
        {
            fgets(file, linedata, charsmax(linedata))
            
            // invalid spawn
            if(!linedata[0] || str_count(linedata,' ') < 2) continue;
            
            // get spawn point data
            parse(linedata,csdmdata[0],5,csdmdata[1],5,csdmdata[2],5,csdmdata[3],5,csdmdata[4],5,csdmdata[5],5,csdmdata[6],5,csdmdata[7],5,csdmdata[8],5,csdmdata[9],5)
            
            // origin
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ORIGIN_X] = floatstr(csdmdata[0])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ORIGIN_Y] = floatstr(csdmdata[1])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ORIGIN_Z] = floatstr(csdmdata[2])
            
            // angles
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ANGLES_X] = floatstr(csdmdata[3])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ANGLES_Y] = floatstr(csdmdata[4])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_ANGLES_Z] = floatstr(csdmdata[5])
            
            // view angles
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_V_ANGLES_X] = floatstr(csdmdata[7])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_V_ANGLES_Y] = floatstr(csdmdata[8])
            g_vecSpawnCSDM[g_iSpawnCountCSDM][SPAWN_DATA_V_ANGLES_Z] = floatstr(csdmdata[9])
            
            // increase spawn count
            g_iSpawnCountCSDM++
            if (g_iSpawnCountCSDM >= sizeof g_vecSpawnCSDM) break;
        }
        if (file) fclose(file)
    }
    else
    {
        // Collect regular spawns
        collect_spawns_ent("info_player_start")
        collect_spawns_ent("info_player_deathmatch")
    }
}
*/