#ifndef PROJECT_ZSH_CONSTS_H
#define PROJECT_ZSH_CONSTS_H
#ifdef _WIN32
#pragma once
#endif

namespace zsh {
	constexpr char LIGHTSTYLE[7] = { 's', 'p', 'm', 'j', 'g', 'd', 'b' };
	constexpr float SKY_COLOR[7] = { 0.7, 0.6, 0.6, 0.5, 0.5, 0.4, 0.4 };
	constexpr const char* FOG_DENSITY[7] = { "0.0001", "0.00015", "0.0002", "0.00025", "0.00025", "0.0003", "0.0003" };
	constexpr const char* FOG_COLOR[7] = { "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0", "0 0 0" };
	constexpr const char SOUND_DAY[] = "shelterteam/skill_bonus.wav";
	constexpr const char SOUND_NIGHT[] = "shelterteam/zombies_amb.wav";
	constexpr const char SOUND_BUILDING_SELECT[] = "shelterteam/object_select.wav";
	constexpr const char SOUND_BUILDING_SETTLE[] = "shelterteam/build2.wav";
	constexpr const char SOUND_BUILDING_COMPLETE[] = "shelterteam/build_complete.wav";
	constexpr const char SOUND_BUILDING_CRASH[] = "shelterteam/crash.wav";
	constexpr const char SOUND_BUILDING_NO[] = "shelterteam/build_impossible.wav";
	constexpr const char SOUND_LEVELUP[] = "shelterteam/skill_button.wav";
	constexpr const char SOUND_VITALITY_WARNING[] = "shelterteam/warning.wav";
	constexpr const float ENERGY_EFFICIENCY[] = { 0.2f, 0.45f, 0.75f, 1.0f, 1.3f }; // 能量转换率
	constexpr const float DAMAGE_RATIO[] = { 0.05f, 0.15f, 0.35f, 1.0f, 1.0f }; // 建筑损坏概率
	constexpr const unsigned int WEATHER_LEVEL_WEIGHT[] = { 50, 100, 20, 10, 1 }; // 天气随机权重
	constexpr const unsigned int VITALITY_DAY_COST = 1;			// 白天体力扣除点数
	constexpr const unsigned int VITALITY_NIGHT_COST = 2;		// 夜晚体力扣除点数
	constexpr const duration_t VITALITY_INTERVAL = 1s;		// 体力每多少秒扣除一次
	constexpr const duration_t HURT_INTERVAL = 5s;			// 无体力后每隔多少秒扣一次血

	enum DayNightStatus_e {
		DNS_DAY,
		DNS_NIGHT
	};

	enum WeatherType_e
	{
		/* 多云 */
		WT_CLOUDY,
		/* 晴天 */
		WT_SUNNY,
		/* 雨天 */
		WT_RAINY,
		/* 风天 */
		WT_GUSTINESS,
		/* 雪天 */
		WT_SNOWY,
		WT_MAX
	};

	enum WeatherLevel_e
	{
		WL_NONE,
		WL_BLUE,
		WL_YELLOW,
		WL_ORANGE,
		WL_RED,
		WL_MAX
	};

	enum ResourceType_e
	{
		/* 木 */
		RT_WOOD,
		/* 铁 */
		RT_STEEL,
		/* 电 */
		RT_EPOWER,
		RT_MAX
	};

	enum ZSHBuildingClass
	{
		BUILDING_GENERATOR_DIESEL,
		BUILDING_GENERATOR_WIND,
		BUILDING_GENERATOR_SOLAR,
		BUILDING_WALL_1,
		BUILDING_WALL_2,
		BUILDING_WALL_3
	};

	struct ZSHBuildingCost_s
	{
		unsigned int wood;
		unsigned int steel;
		unsigned int power;
	};
}

#endif //PROJECT_ZSH_CONSTS_H
