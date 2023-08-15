
#ifndef BOSSCLASS_BIG01_H
#define BOSSCLASS_BIG01_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"

namespace sv {


class CBossClass_Big01 : public CBaseMonsterClass
{
public:
	CBossClass_Big01(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
};

class CBossSkill_Big01 : public CBaseMonsterSkill
{

public:
	CBossSkill_Big01(CMonster* p) : CBaseMonsterSkill(p)
	{ 
		m_ioc_enabled = true;
	}

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void OnTouch(CBaseEntity* other) override;
	void ActivateSkill(CBaseEntity* target);

	void Dash(const BossData& sk, CBaseEntity* target);
	void ShockWave(const BossData& sk);

protected:
	boost::asio::awaitable<void> Task_ZBS_SHOCKWAVE();
	boost::asio::awaitable<void> Task_ZBS_SWING();
	boost::asio::awaitable<void> Task_ZBS_DASH();

	time_point_t m_flForceAttackTime;
	BossData m_CurSkillData;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;
};


}

#endif
