
#ifndef BOSSCLASS_BIG02_H
#define BOSSCLASS_BIG02_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"

namespace sv {

	class CZBSBig02Bomb : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT BombTouch(CBaseEntity* pOther);

		float m_flDamage;
		float m_flRadius;
	};

class CBossClass_Big02 : public CBaseMonsterClass
{
public:
	CBossClass_Big02(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
};

class CBossSkill_Big02 : public CBaseMonsterSkill
{
public:
	CBossSkill_Big02(CMonster* p) : CBaseMonsterSkill(p)
	{
		m_iHoleIndex = PRECACHE_MODEL("models/ef_hole.mdl");
		m_iKnifeIndex = PRECACHE_MODEL("models/ef_knife.mdl");
		m_ioc_enabled = true;
	}

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void ActivateSkill(CBaseEntity* target);

protected:
	BossData m_CurSkillData;
	time_point_t m_flHoleSkillTime;
	time_point_t m_flForceAttackTime;
	int m_iKnifeIndex;
	int m_iHoleIndex;
	int m_iBombSkillCount;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;

	boost::asio::awaitable<void> Task_ZBS_ATTACK1();
	boost::asio::awaitable<void> Task_ZBS_ATTACK2();
	boost::asio::awaitable<void> Task_ZBS_ATTACK3();
	boost::asio::awaitable<void> Task_ZBS_ATTACK_HOLE();
	boost::asio::awaitable<void> Task_ZBS_ATTACK_BOMB();
	boost::asio::awaitable<void> Task_ZBS_APPEAR();
	boost::asio::awaitable<void> Task_ZBS_SCENE_KNIFE();

	void Jump(const BossData& sk, CBaseEntity* target);
	void AttackHole(const BossData& sk, bool loop);
};


}

#endif
