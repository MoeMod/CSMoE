
#ifndef BOSSCLASS_BIG00_H
#define BOSSCLASS_BIG00_H
#ifdef _WIN32
#pragma once
#endif

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
public:
	CBossSkill_Big00(CMonster* p) : CBaseMonsterSkill(p) { m_flTimeTriggerZombieSkill = invalid_time_point; }

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void OnTouch(CBaseEntity* other) override;
	void OnSkillEnd() override;
	void FireZombieSkill();
	float GetDamage();
	time_point_t m_flTimeTriggerZombieSkill;

};


}

#endif
