#ifndef ZBS_MAP_MANAGER_H
#define ZBS_MAP_MANAGER_H
#ifdef _WIN32
#pragma once
#endif

namespace sv {

	struct SpawnData
	{
		std::string type;	//P OR Z
		int spawnPoint;
		Vector origin;
		Vector angle;
		int team;
	};


	struct WaveData
	{
		int waveID;
		int spawnPoint;
		duration_t startTime;
		duration_t endTime;
		duration_t intervalTime;

		std::map<int, int> vecZombi;
		std::string eventmessage;

		int wavestate;
		time_point_t m_flNextWave;
		time_point_t m_flEndWave;
	};

	struct ZombiTypeData
	{
		int id;
		std::string appearance;
		float attackDamage;
		float attackRange;
		duration_t attackInterval;
		float health;
		float armor;
		float speed;
		int bonusMoney;
		int priority;
		int skill;
		std::vector<int> color;
		int Killtype;
		int EventClass;
		int propensity;
		int clearpoint;
		int dropItem;
		int exp;
	};

	struct ZBSData
	{
		int gametype;
		int roundTime;
		int wallBonusDmg;
		int wallBonusPoint;
		int wallexp;
		int RewardRound;

		int maxround;
	};

	struct ZBSRoundData
	{
		int round;
		std::vector<int> waveTypes;
		int GoalKillCount;
		float damageAdd;
		float attackRangeAdd;
		float healthAdd;
		float armorAdd;
		float speedAdd;
		duration_t IntervalAdd;
		int moneyAdd;
		int BGM;
		int PlayerSpawn;
	};

	struct TriggerData
	{
		int id;
		std::string type;
		int map_triggerid;
		int waveid;
		duration_t cooltime;
		float destratio;
		int endwave;
		int hostCrashReset;
		int roundTimeBase;
	};

	struct BossData
	{
		int id;
		int type;
		float ratio;
		duration_t cooltime;
		duration_t aniTime;
		std::string aniSeq;
		duration_t damageDelay;
		float damage;
		float damageDegree;
		float damageHeight;
		float damageDist;
		duration_t forceAttackTime;
		float startDistMin;
		float startDistMax;
		float startHealthMin;
		float startHealthMax;
		float approachDist;
		int startMsg;
		float kickSide;
		float kickUp;
		float rushPower;

		time_point_t m_flTimeSkillEnd;
	};


	void ZBS_LoadSpawnArea();
	void ZBS_LoadWaveData();
	void ZBS_LoadZombiTypeData();
	void ZBS_LoadZBSRoundData();
	void ZBS_LoadTriggerData();
	void ZBS_LoadBossData();
}

#endif