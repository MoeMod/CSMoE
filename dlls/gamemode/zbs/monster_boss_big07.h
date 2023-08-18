
#ifndef BOSSCLASS_BIG07_H
#define BOSSCLASS_BIG07_H
#ifdef _WIN32
#pragma once
#endif

#include "zbs_map_manager.h"
#include "monster_entity.h"
#include "gamemode/mod_zbs.h"
#include "zs_subs.h"

namespace sv {

	class CZBSBig07Landmine : public CBaseEntity
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT LandmineThink();
		void EXPORT LandmineTouch(CBaseEntity* pOther);
		void MakeExplosion();
		void LieFlat();

		float m_flDamage;
		float m_flRadius;
		time_point_t m_flExpTime;
	};

	class CZBSBig07Grenade : public CBaseAnimating
	{
	public:
		void Spawn(void);
		void Precache(void);

		void EXPORT IgniteThink(void);
		void EXPORT FollowThink(void);
		void EXPORT GrenadeTouch(CBaseEntity* pOther);

		float m_flDamage;
		float m_flRadius;

	private:
		int m_iTrail;
		
	};

class CBossClass_Big07 : public CBaseMonsterClass
{
public:
	CBossClass_Big07(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
	void OnKilled(entvars_t* pKiller, int iGib) override;
};

class CBossSkill_Big07 : public CBaseMonsterSkill
{
public:
	CBossSkill_Big07(CMonster* p) : CBaseMonsterSkill(p)
	{
		m_ioc_enabled = true;
	}

	void OnTouch(CBaseEntity* other) override;
	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void ActivateSkill(CBaseEntity* target);

	void CreateCircleLandmines(Vector vecOrigin);
	void CreateStraightLandmines(Vector vecStart);
	CZBSBig07Landmine* CreateLandmine(Vector vecOrigin, Vector vecOrigin2);
	void CreateSpecialRocket(float flFOffset, float flROffset, float flUOffset, Vector vecOrigin2);
	void CreateStraightRocket(Vector vecOrigin, float flDistance, float flROffset, float flUOffset);
	CZBSBig07Grenade* CreateRocket(Vector vecOrigin);
	void GetPositon(float forward, float right, float up, Vector& start);
	int VecCheckThrow(Vector vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj, Vector &vecGrenadeVel);

protected:

	BossData m_CurSkillData;
	time_point_t m_flCannon1SkillTime;
	time_point_t m_flCannon2SkillTime;
	time_point_t m_flAppearTime;
	time_point_t m_flForceAttackTime;
	int m_iShootTimes;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;

	boost::asio::awaitable<void> TASK_ZBS_APPEAR();
	boost::asio::awaitable<void> TASK_ZBS_CANNON_READY();
	boost::asio::awaitable<void> TASK_ZBS_CANNON2();
	boost::asio::awaitable<void> TASK_ZBS_LANDMINE1();
	boost::asio::awaitable<void> TASK_ZBS_LANDMINE2();
	boost::asio::awaitable<void> TASK_ZBS_DASH();
	boost::asio::awaitable<void> TASK_ZBS_ATTACK1();
	boost::asio::awaitable<void> TASK_ZBS_ATTACK2();
	boost::asio::awaitable<void> TASK_ZBS_HOWLING();


};


}

#endif
