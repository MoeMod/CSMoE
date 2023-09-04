
#ifndef BOSSCLASS_BIG08_H
#define BOSSCLASS_BIG08_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"
#include "gamemode/mod_zbs.h"
#include "zs_subs.h"

namespace sv {


	class CZBSBig08Tentacle : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT TentacleTouch(CBaseEntity* pOther);
		void EXPORT TentacleThink();
		void EXPORT RemoveThink();

		float m_flDamage;
		float m_flRadius;

		EntityHandle <CBasePlayer> m_hTarget;

		duration_t GetAliveTime() const { return 15s; }
		duration_t GetTrappedTime() const { return 5.0s; }
	};

	class CZBSBig08Hurricane : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT PoisonTouch(CBaseEntity* pOther);
		void EXPORT PoisonThink();
		void EXPORT FollowThink();

		float m_flDamage;
		float m_flRadius;


	};

class CBossClass_Big08 : public CBaseMonsterClass
{
public:
	CBossClass_Big08(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
	//const Vector GetCenterid() const override;
};

class CBossSkill_Big08 : public CBaseMonsterSkill
{

public:
	CBossSkill_Big08(CMonster* p) : CBaseMonsterSkill(p)
	{ 
		PRECACHE_MODEL("models/dione_tentacle.mdl");
		PRECACHE_MODEL("models/ef_dione_tentacle.mdl");
		m_ioc_enabled = true;
	}

	void OnThink() override;
	void OnTouch(CBaseEntity* other) override;
	void OnActivate(CBaseEntity* target) override;
	void ActivateSkill(CBaseEntity* target);
	void CreateStraightTentacle();
	void CreateSpecialTentacle();
	void GetPositon(float forward, float right, float up, Vector& start);
	Vector TraceFloor(Vector vecStart);
	CZBSBig08Tentacle* CreateTentacle(Vector vecOrigin);
	CZBSBig08Hurricane* CreatePoisonBall(Vector vecOrigin);
protected:
	int m_iTentacleSkillCount;
	BossData m_CurSkillData;
	time_point_t m_flForceAttackTime;

	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;

	boost::asio::awaitable<void> TASK_ZBS_APPEAR();
	boost::asio::awaitable<void> TASK_ZBS_RUN();
	boost::asio::awaitable<void> ZBS_ATTACK_BITE();
	boost::asio::awaitable<void> ZBS_ATTACK_SWING();
	boost::asio::awaitable<void> ZBS_ATTACK_TENTACLE1();
	boost::asio::awaitable<void> ZBS_ATTACK_TENTACLE2();
	boost::asio::awaitable<void> ZBS_ATTACK_POISON1();
	boost::asio::awaitable<void> ZBS_ATTACK_POISON2();
	boost::asio::awaitable<void> ZBS_ATTACK_ROLLING();

};


}

#endif
