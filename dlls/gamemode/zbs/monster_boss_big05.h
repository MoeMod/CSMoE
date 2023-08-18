
#ifndef BOSSCLASS_BIG05_H
#define BOSSCLASS_BIG05_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"
#include "gamemode/mod_zbs.h"
#include "zs_subs.h"

namespace sv {

	class CSpringCooler : public CPointEntity
	{
		//revenant_groggy
	public:
		void Spawn() override;
		void Run();

	private:
		int m_iSmokeIndex;

	};

	class CRevenantGroggy : public CBaseTrigger
	{
		//revenant_groggy
	public:
		void Spawn() override;
		void KeyValue(KeyValueData* pkvd) override;
		void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
	public:


	};

	class CZBSBig05FireBall : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);
		void Launch(void);
		void EXPORT PosThink(void);
		void EXPORT IgniteThink(void);
		void EXPORT FollowThink(void);
		void EXPORT FireBallTouch(CBaseEntity* pOther);

		float m_flDamage;
		float m_flRadius;
		int m_iType;

	private:
		int m_iTrail;
		
	};

class CBossClass_Big05 : public CBaseMonsterClass
{
public:
	CBossClass_Big05(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
};

class CBossSkill_Big05 : public CBaseMonsterSkill
{
public:
	CBossSkill_Big05(CMonster* p) : CBaseMonsterSkill(p)
	{
		m_iSpringCoolerIndex = 0;
		m_bHellFireActivated = false;
		m_iHellFireIndex = PRECACHE_MODEL("models/zombiegiant/revenant_hellfire.mdl");
		m_iSkyFlameIndex = PRECACHE_MODEL("models/zombiegiant/ef_sky_flame.mdl");
		m_ioc_enabled = true;
	}

	void OnTouch(CBaseEntity* other) override;
	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void ActivateSkill(CBaseEntity* target);
	
	CZBSBig05FireBall* CreateFireBall(Vector vecOrigin);
	void SetSpringCooler(bool reset);
	void CreateRoundFire(Vector vecStart, float dis);
	void GetPositon(float forward, float right, float up, Vector& start);
	void CreateHellFire();
	int VecCheckThrow(Vector vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj, Vector &vecGrenadeVel);
	void RadiusDamage(Vector vecSrc, float flDamage, float angle, float flRadius, float flHeight, int bitsDamageType, float flKickSide, float flKickUp, float flRushPower, bool bFalloff = false, bool bDropWeapons = false);
	bool m_bHellFireActivated;
	bool m_bSpringCoolerUsed;
	int m_iSpringCoolerIndex;

	boost::asio::awaitable<void> TASK_ZBS_GROGGY();
protected:

	BossData m_CurSkillData;
	time_point_t m_flAppearTime;
	time_point_t m_flForceAttackTime;
	int m_iSkyFlameIndex;
	int m_iHellFireIndex;

	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;

	boost::asio::awaitable<void> TASK_ZBS_APPEAR();
	boost::asio::awaitable<void> TASK_ZBS_FIREBALL1();
	boost::asio::awaitable<void> TASK_ZBS_FIREBALL2();
	boost::asio::awaitable<void> TASK_ZBS_BURNNING1();
	boost::asio::awaitable<void> TASK_ZBS_BURNNING2();
	boost::asio::awaitable<void> TASK_ZBS_DASH();
	boost::asio::awaitable<void> TASK_ZBS_ATTACK1();
	boost::asio::awaitable<void> TASK_ZBS_ATTACK4();
	boost::asio::awaitable<void> TASK_ZBS_ATTACK5();



};


}

#endif
