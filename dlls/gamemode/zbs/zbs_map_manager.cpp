
#include "extdll.h"
#include "common.h"
#include "util.h"
#include "cbase.h"

#include <string>
#include <sstream>
#include "game_shared/steam_util.h"
#include "zbs_map_manager.h"

namespace sv {

std::vector<SpawnData> g_ZBSSpawnData;
std::vector<WaveData> g_ZBSWaveData;
std::vector<ZombiTypeData> g_ZBSZombiTypeData;
ZBSData g_ZBSData;
std::vector<ZBSRoundData> g_ZBSRoundData;
std::vector<TriggerData> g_ZBSTriggerData;
std::vector<BossData> g_BossData;

void ZBS_LoadSpawnArea()
{
	g_ZBSSpawnData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_sp_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

	std::pair<bool, std::string> linedata;
	while ((linedata = readline(file)).first)
	{
		std::istringstream ss(linedata.second);
		std::string data;

		SpawnData spawn;

		std::getline(ss, data, ',');
		spawn.type = data;

		std::getline(ss, data, ',');
		spawn.spawnPoint = atoi(data.c_str());

		std::getline(ss, data, ',');
		spawn.origin.x = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.origin.y = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.origin.z = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.angle.x = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.angle.y = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.angle.z = atof(data.c_str());

		std::getline(ss, data, ',');
		spawn.team = atoi(data.c_str());

		g_ZBSSpawnData.push_back(spawn);
	}
}

void ZBS_LoadWaveData()
{
	g_ZBSWaveData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_wave_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

		WaveData wave;

		std::getline(ss, data, ',');
		wave.waveID = atoi(data.c_str());

		std::getline(ss, data, ',');
		wave.spawnPoint = atoi(data.c_str());

		std::getline(ss, data, ',');
		wave.startTime = atof(data.c_str()) * 1.0s;

		std::getline(ss, data, ',');
		wave.endTime = atof(data.c_str()) * 1.0s;

		std::getline(ss, data, ',');
		wave.intervalTime = atof(data.c_str()) * 1.0s;

		std::getline(ss, data, ',');
		std::string zombi = data;

		std::getline(ss, data, ',');
		std::string zombiRatio = data;

		std::istringstream sszombi(zombi);
		std::istringstream ssratio(zombiRatio);

		int zombitype;
		int zombiratio;

		while (sszombi >> zombitype)
		{
			ssratio >> zombiratio;
			wave.vecZombi.insert(std::pair(zombitype, zombiratio));
		}

		std::getline(ss, data, ',');
		wave.eventmessage = data;
		wave.wavestate = 0;

		g_ZBSWaveData.push_back(wave);
	}
}

void ZBS_LoadZombiTypeData()
{
	g_ZBSZombiTypeData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_zt_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

		ZombiTypeData zt;

		std::getline(ss, data, ',');
		zt.id = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.appearance = data;

		std::getline(ss, data, ',');
		zt.attackDamage = atof(data.c_str());

		std::getline(ss, data, ',');
		zt.attackRange = atof(data.c_str());

		std::getline(ss, data, ',');
		zt.attackInterval = atof(data.c_str()) * 1.0s;

		std::getline(ss, data, ',');
		zt.health = atof(data.c_str());


		std::getline(ss, data, ',');
		zt.armor = atof(data.c_str());

		std::getline(ss, data, ',');
		zt.speed = atof(data.c_str());

		std::getline(ss, data, ',');
		zt.bonusMoney = atoi(data.c_str());
		
		std::getline(ss, data, ',');
		zt.priority = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.skill = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.skill = atoi(data.c_str());

		std::string str = data;
		std::istringstream is(str);
		int i;
		while (is >> i)
		{
			zt.color.push_back(i);
		}

		std::getline(ss, data, ',');
		zt.Killtype = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.EventClass = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.propensity = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.clearpoint = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.dropItem = atoi(data.c_str());

		std::getline(ss, data, ',');
		zt.exp = atoi(data.c_str());

		g_ZBSZombiTypeData.push_back(zt);
	}
}

void ZBS_LoadZBSRoundData()
{
	g_ZBSRoundData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

	std::pair<bool, std::string> linedata;
	while ((linedata = readline(file)).first)
	{
		std::istringstream ss(linedata.second);
		std::string data;

		std::getline(ss, data, ',');
		if (!strcmp(data.c_str(), "gametype"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.gametype = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "roundTime"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.roundTime = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "wallBonusDmg"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.wallBonusDmg = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "wallBonusPoint"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.wallBonusPoint = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "wallexp"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.wallexp = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "RewardRound"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.RewardRound = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "wall") ||
				 !strcmp(data.c_str(), "wallexo"))
		{
			std::getline(ss, data, ',');
			g_ZBSData.wallBonusDmg = g_ZBSData.wallBonusPoint = g_ZBSData.wallexp = atoi(data.c_str());
			continue;
		}
		else if (!strcmp(data.c_str(), "round"))
			continue;

		ZBSRoundData rounddata;
		rounddata.round = atoi(data.c_str());


		std::getline(ss, data, ',');
		std::string str = data;
		std::istringstream is(str);
		int i;
		while (is >> i)
		{
			rounddata.waveTypes.push_back(i);
		}

		std::getline(ss, data, ',');
		rounddata.GoalKillCount = atoi(data.c_str());

		std::getline(ss, data, ',');
		rounddata.damageAdd = atof(data.c_str());

		std::getline(ss, data, ',');
		rounddata.attackRangeAdd = atof(data.c_str());

		std::getline(ss, data, ',');
		rounddata.healthAdd = atof(data.c_str());

		std::getline(ss, data, ',');
		rounddata.armorAdd = atof(data.c_str());

		std::getline(ss, data, ',');
		rounddata.speedAdd = atof(data.c_str());

		std::getline(ss, data, ',');
		rounddata.IntervalAdd = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		rounddata.moneyAdd = atoi(data.c_str());

		std::getline(ss, data, ',');
		rounddata.BGM = atoi(data.c_str());

		std::getline(ss, data, ',');
		rounddata.PlayerSpawn = atoi(data.c_str());

		g_ZBSRoundData.push_back(rounddata);


		g_ZBSData.maxround = g_ZBSRoundData.size();
	}
}

void ZBS_LoadTriggerData()
{
	g_ZBSTriggerData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_trg_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

		TriggerData trg;

		std::getline(ss, data, ',');
		trg.id = atoi(data.c_str());

		std::getline(ss, data, ',');
		trg.type = data;

		std::getline(ss, data, ',');
		trg.map_triggerid = atoi(data.c_str());

		std::getline(ss, data, ',');
		trg.waveid = atoi(data.c_str());

		std::getline(ss, data, ',');
		trg.cooltime = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		trg.destratio = atof(data.c_str());

		std::getline(ss, data, ',');
		trg.endwave = atoi(data.c_str());

		std::getline(ss, data, ',');
		trg.hostCrashReset = atoi(data.c_str());

		std::getline(ss, data, ',');
		trg.roundTimeBase = atoi(data.c_str());

		g_ZBSTriggerData.push_back(trg);

	}
}


void ZBS_LoadBossData()
{
	g_BossData.clear();

	static char filename[256];
	Q_sprintf(filename, "maps\\zbs_sk_%s.csv", STRING(gpGlobals->mapname));

	SteamFile file(filename);

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

		BossData sk;
		//temp useless
		std::getline(ss, data, ',');

		std::getline(ss, data, ',');
		sk.id = atoi(data.c_str());

		std::getline(ss, data, ',');
		sk.type = atoi(data.c_str());

		std::getline(ss, data, ',');
		sk.ratio = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.cooltime = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		sk.aniTime = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		sk.aniSeq = data;

		std::getline(ss, data, ',');
		sk.damageDelay = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		sk.damage = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.damageDegree = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.damageHeight = atof(data.c_str()) * 39.37;

		std::getline(ss, data, ',');
		sk.damageDist = atof(data.c_str()) * 39.37;

		std::getline(ss, data, ',');
		sk.forceAttackTime = atof(data.c_str()) * 1s;

		std::getline(ss, data, ',');
		sk.startDistMin = atof(data.c_str()) * 39.37;

		std::getline(ss, data, ',');
		sk.startDistMax = atof(data.c_str()) * 39.37;

		std::getline(ss, data, ',');
		sk.startHealthMin = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.startHealthMax = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.approachDist = atof(data.c_str()) * 39.37;

		std::getline(ss, data, ',');
		sk.startMsg = atoi(data.c_str());

		std::getline(ss, data, ',');
		sk.kickSide = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.kickUp = atof(data.c_str());

		std::getline(ss, data, ',');
		sk.rushPower = atof(data.c_str());

		g_BossData.push_back(sk);
	}
}
}
