
#ifndef BOSSCLASS_BIG00_H
#define BOSSCLASS_BIG00_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"

namespace sv {


class CBossClass_Big00 : public CBaseMonsterClass
{
public:
	CBossClass_Big00(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
};

class CBossSkill_Big00 : public CBaseMonsterSkill
{
	enum BossSkillStatus : byte
	{
		NONE,
		SWING,
		SWING_DMG,
		SHOCKWAVE,
		SHOCKWAVE_DMG,
		DASH,
		DASH_ACTION,
		DASH_END
	};

public:
	CBossSkill_Big00(CMonster* p) : CBaseMonsterSkill(p) , m_iBossSkillStatus(NONE)
	{ 	}

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void OnTouch(CBaseEntity* other) override;

	void Dash(const BossData& sk, CBaseEntity* target);
	void ShockWave(const BossData& sk);

	time_point_t m_flTimeTriggerZombieSkill;

protected:
	BossData m_CurSkillData;
	BossSkillStatus m_iBossSkillStatus;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;
};


}

#endif
