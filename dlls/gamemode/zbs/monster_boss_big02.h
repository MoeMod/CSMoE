
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
};

class CBossSkill_Big02 : public CBaseMonsterSkill
{
	enum BossSkillStatus : byte
	{
		ZBS_NONE,
		ZBS_ATTACK1,
		ZBS_ATTACK1_DMG,
		ZBS_ATTACK2,
		ZBS_ATTACK2_DMG,
		ZBS_ATTACK3,
		ZBS_ATTACK3_ACTION_JUMP,
		ZBS_ATTACK3_ACTION_TRACE,
		ZBS_ATTACK3_END,
		ZBS_ATTACK_BOMB,
		ZBS_ATTACK_BOMB_ACTION,
		ZBS_ATTACK_BOMB_END,
		ZBS_ATTACK_HOLE,
		ZBS_ATTACK_HOLE_ACTION,
		ZBS_ATTACK_HOLE_END,
		ZBS_APPEAR,
		ZBS_SCENE_KNIFE,
	};

public:
	CBossSkill_Big02(CMonster* p) : CBaseMonsterSkill(p) , m_iBossSkillStatus(ZBS_NONE)
	{ 
		m_iHoleIndex = PRECACHE_MODEL("models/ef_hole.mdl");
	}

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;

	void Jump(const BossData& sk, CBaseEntity* target);
	void AttackHole(const BossData& sk, bool loop);

protected:
	BossData m_CurSkillData;
	time_point_t m_flHoleSkillTime;
	int m_iHoleIndex;
	int m_iBombSkillCount;
	BossSkillStatus m_iBossSkillStatus;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;
};


}

#endif
