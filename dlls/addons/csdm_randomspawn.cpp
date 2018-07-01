// Translated from
// https://github.com/MoeMod/BTE-AMXX/blob/master/BTE%20Codename%20Z4E/z4e_random_spawn.sma
// or random CSDM .sma, IDK where this code came from...
// MoeMod 2018/7/1

#include "extdll.h"
#include "common.h"
#include "util.h"

#include "game_shared/steam_util.h"

#define MAX_CSDM_SPAWNS 128
#define SPAWN_DATA_ORIGIN_X 0
#define SPAWN_DATA_ORIGIN_Y 1
#define SPAWN_DATA_ORIGIN_Z 2
#define SPAWN_DATA_ANGLES_X 3
#define SPAWN_DATA_ANGLES_Y 4
#define SPAWN_DATA_ANGLES_Z 5
#define SPAWN_DATA_V_ANGLES_X 6
#define SPAWN_DATA_V_ANGLES_Y 7
#define SPAWN_DATA_V_ANGLES_Z 8
float g_vecSpawnCSDM[MAX_CSDM_SPAWNS][SPAWN_DATA_V_ANGLES_Z + 1], g_vecSpawn[MAX_CSDM_SPAWNS][SPAWN_DATA_V_ANGLES_Z + 1];
int g_iSpawnCountCSDM = 0, g_iSpawnCount = 0;

void load_spawns()
{
	// Check for CSDM spawns of the current map
	char filename[256];
	Q_sprintf(filename, "addons/amxmodx/configs/csdm/%s.spawns.cfg", STRING(gpGlobals->mapname));


	SteamFile csdmFile(filename);
	
	auto readline = [](SteamFile &sf) {
		std::string ret;
		char ch = '\0';
		while (sf.Read(&ch, 1))
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
		// ...

	}

	/*
		get_configsdir(cfgdir, charsmax(cfgdir))
		get_mapname(mapname, charsmax(mapname))
		formatex(filepath, charsmax(filepath), "%s/csdm/%s.spawns.cfg", cfgdir, mapname)

		// Load CSDM spawns if present
		if (file_exists(filepath))
		{
			new csdmdata[10][6], file = fopen(filepath, "rt")

				while (file && !feof(file))
				{
					fgets(file, linedata, charsmax(linedata))

						// invalid spawn
						if (!linedata[0] || str_count(linedata, ' ') < 2) continue;

					// get spawn point data
					parse(linedata, csdmdata[0], 5, csdmdata[1], 5, csdmdata[2], 5, csdmdata[3], 5, csdmdata[4], 5, csdmdata[5], 5, csdmdata[6], 5, csdmdata[7], 5, csdmdata[8], 5, csdmdata[9], 5)

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
		}*/
}

