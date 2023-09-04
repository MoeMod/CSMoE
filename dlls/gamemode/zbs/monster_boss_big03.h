
#ifndef BOSSCLASS_BIG03_H
#define BOSSCLASS_BIG03_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"
#include "gamemode/mod_zbs.h"
#include "zs_subs.h"

namespace sv {


	class CZBSBig03Tentacle : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT TentacleTouch(CBaseEntity* pOther);
		void EXPORT TentacleThink();
		void EXPORT RemoveThink();

		float m_flDamage;
		float m_flRadius;
		Vector vecOrigin;

	private:
		int m_iTentacleIndex[2];


	};

	class CZBSBig03PoisonBall : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT PoisonTouch(CBaseEntity* pOther);
		void EXPORT PoisonThink();
		void EXPORT FollowThink();

		float m_flDamage;
		float m_flRadius;
		float m_flPoisonDamage;
	private:
		int m_iPoisonIndex[2];


	};

class CBossClass_Big03 : public CBaseMonsterClass
{
public:
	CBossClass_Big03(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
	//const Vector GetCenterid() const override;
};

class CBossSkill_Big03 : public CBaseMonsterSkill
{

public:
	CBossSkill_Big03(CMonster* p) : CBaseMonsterSkill(p)
	{ 
		m_iSwingIndex = PRECACHE_MODEL("models/ef_swing.mdl");
		m_ioc_enabled = true;
	}

	void OnThink() override;
	void OnTouch(CBaseEntity* other) override;
	void OnActivate(CBaseEntity* target) override;
	void ActivateSkill(CBaseEntity* target);
	void CreateStraightTentacle();
	void CreateSpecialTentacle();
	void vRotationTransform(float dx, float dy, float dAngle, float& dbNewX, float& dbNewY);
	void GetPositon(float forward, float right, float up, Vector& start);
	Vector TraceFloor(Vector vecStart);
	CZBSBig03Tentacle* CreateTentacle(Vector vecOrigin);
	CZBSBig03PoisonBall* CreatePoisonBall(float fYaw, float gravity);
protected:
	int m_iSwingIndex;
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
	//boost::asio::awaitable<void> ZBS_ATTACK_POISON2();
	boost::asio::awaitable<void> ZBS_ATTACK_ROLLING();

};


}

#endif
