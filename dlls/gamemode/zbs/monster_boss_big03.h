
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

		void EXPORT BombTouch(CBaseEntity* pOther);

		float m_flDamage;
		float m_flRadius;


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
	enum BossSkillStatus : byte
	{
		ZBS_NONE,
		ZBS_APPEAR,
		ZBS_APPEAR2,
		ZBS_APPEAR3,
		ZBS_ESCAPE,
		ZBS_ATTACK_BITE,
		ZBS_ATTACK_BITE_DMG,
		ZBS_ATTACK_SWING,
		ZBS_ATTACK_SWING_DMG,
		ZBS_ATTACK_TENTACLE,
		ZBS_ATTACK_TENTACLE_ACTION,
		ZBS_ATTACK_TENTACLE_END,
		ZBS_ATTACK_POISON,
		ZBS_ATTACK_POISON_ACTION,
		ZBS_ATTACK_POISON_END,
		ZBS_ATTACK_ROLLING,
		ZBS_ATTACK_ROLLING_ACTION,
		ZBS_ATTACK_ROLLING_END,
	};

public:
	CBossSkill_Big03(CMonster* p) : CBaseMonsterSkill(p) , m_iBossSkillStatus(ZBS_NONE)
	{ 


	}

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;

	void Jump(const BossData& sk, CBaseEntity* target);
	void AttackHole(const BossData& sk, bool loop);

protected:
	BossData m_CurSkillData;
	time_point_t m_flHoleSkillTime;
	int m_iBombSkillCount;
	BossSkillStatus m_iBossSkillStatus;
	std::unordered_map<int, EngineClock::time_point> m_MapSkillTime;
};


}

#endif
