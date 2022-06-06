
#ifndef MONSTER_ENTITY_H
#define MONSTER_ENTITY_H
#ifdef _WIN32
#pragma once
#endif

#include "hostage/hostage.h"
#include <memory>
#include <map>
#include <string>
#include <mutex>

namespace sv {

enum MonsterClass
{
	MONSTER_CLASS_RANDOM,
	MONSTER_CLASS_ZOMBIE,
	MONSTER_CLASS_SPEED,
	MONSTER_CLASS_HEAVY,
	MONSTER_CLASS_THROWER,
	MONSTER_CLASS_HEAL,
	MONSTER_CLASS_ZOMBIEDOG,
	MONSTER_CLASS_BOOMER,
	MONSTER_CLASS_RESIDENT,
	MONSTER_CLASS_DEIMOS,
	MONSTER_CLASS_HELMET,
	MONSTER_CLASS_DEIMOS2,
};

enum MonsterAnim
{
	MONSTERANIM_IDLE,
	MONSTERANIM_WALK,
	MONSTERANIM_JUMP,
	MONSTERANIM_DIE,
	
	MONSTERANIM_FLINCH,
	MONSTERANIM_LARGE_FLINCH,
	MONSTERANIM_ATTACK,
	MONSTERANIM_SKILL,
};

enum MonsterSkillStatus : byte
{
	SKILL_STATUS_READY,
	SKILL_STATUS_USING,
	SKILL_STATUS_FREEZING
};

class CMonster;

struct BaseMonsterExtra
{
public:
	BaseMonsterExtra(CMonster *p) : m_pMonster(p) {}

public:
	CMonster * const m_pMonster;
};

class IBaseMonsterSkill : public BaseMonsterExtra
{
public:
	IBaseMonsterSkill(CMonster* p) : BaseMonsterExtra(p) {}
	virtual ~IBaseMonsterSkill() = 0;

	virtual void OnThink() = 0;
	virtual void OnTouch(CBaseEntity* other) = 0;
	virtual void OnActivate(CBaseEntity *target) = 0;
	virtual void OnReset() = 0;
	virtual void OnKilled(entvars_t *pKiller, int iGib) = 0;
	virtual MonsterSkillStatus GetStatus() const = 0;
};

inline IBaseMonsterSkill::~IBaseMonsterSkill() {}

class IBaseMonsterStrategy : public BaseMonsterExtra
{
public:
	IBaseMonsterStrategy(CMonster *p) : BaseMonsterExtra(p) {}
	virtual ~IBaseMonsterStrategy() = 0;

	virtual void OnSpawn() = 0;
	virtual void OnThink() = 0;
	virtual void OnTouch(CBaseEntity* other) = 0;
	virtual void OnKilled(entvars_t *pKiller, int iGib) = 0;

	virtual void DeathSound() const = 0;
	virtual bool IsTeamMate(CBaseEntity *that) const = 0;
};

inline IBaseMonsterStrategy::~IBaseMonsterStrategy() {}

class CBaseMonsterSkill : public IBaseMonsterSkill
{
public:
	explicit CBaseMonsterSkill(CMonster* p) : IBaseMonsterSkill(p), m_iSkillStatus(SKILL_STATUS_READY) {}

	void OnThink() override;
	void OnTouch(CBaseEntity* other) override {}
	void OnActivate(CBaseEntity* target) override {}
	void OnReset() override { m_iSkillStatus = SKILL_STATUS_READY; }
	void OnKilled(entvars_t* pKiller, int iGib) override {}
	MonsterSkillStatus GetStatus() const override { return m_iSkillStatus; }

protected:
	virtual void OnSkillEnd() {}
	virtual void OnSkillReady() {}

protected:
	MonsterSkillStatus m_iSkillStatus;
	EngineClock::time_point m_flTimeSkillEnd;
	EngineClock::time_point m_flTimeSkillNext;
};

class IBaseMonsterClass : public BaseMonsterExtra
{
public:
	IBaseMonsterClass(CMonster* p) : BaseMonsterExtra(p) {}
	virtual ~IBaseMonsterClass() = 0;

	virtual void OnInit() = 0;
	virtual void OnSpawn() = 0;
	virtual void OnThink() = 0;
	virtual void OnTouch(CBaseEntity* other) = 0;
	virtual void OnKilled(entvars_t* pKiller, int iGib) = 0;

	virtual const Vector GetCenterid() const = 0;
	virtual void DeathSound() const = 0;
};

inline IBaseMonsterClass::~IBaseMonsterClass() {}

class CBaseMonsterClass : public IBaseMonsterClass
{
public:
	explicit CBaseMonsterClass(CMonster* p) : IBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override {}
	void OnThink() override {}
	void OnTouch(CBaseEntity* other) override {}
	void OnKilled(entvars_t* pKiller, int iGib) override  {}

	const Vector GetCenterid() const override;
	void DeathSound() const override;
};

class CMonsterModStrategy_Default : public IBaseMonsterStrategy
{
public:
	CMonsterModStrategy_Default(CMonster *p) : IBaseMonsterStrategy(p) {}

	void OnSpawn() override;
	void OnThink() override;
	void OnTouch(CBaseEntity* other) override;
	void OnKilled(entvars_t *pKiller, int iGib) override;

	void DeathSound() const override;
	bool IsTeamMate(CBaseEntity *that) const override
	{
		return that->pev->team == TEAM_TERRORIST;
	}

public:
	std::shared_ptr<IBaseMonsterClass> m_pClass;
	std::unique_ptr<IBaseMonsterSkill> m_pSkill;
	
public:
	void SetClass(MonsterClass iClass);
	void SetSkill(std::unique_ptr<IBaseMonsterSkill> ptr)
	{
		m_pSkill = std::move(ptr);
	}
	std::shared_ptr<CBaseMonsterClass> MonsterClassFactory(MonsterClass iClass);
};

class CMonster : public CHostage
{
public:
	void Spawn() override;
	void Precache() override;
	int ObjectCaps() override { return (CBaseMonster::ObjectCaps() | FCAP_MUST_SPAWN); }
	int Classify() override { return CLASS_PLAYER_ALLY; }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void BecomeDead(void) override;
	void Killed(entvars_t *pevAttacker, int iGib) override;
	int BloodColor() override { return BLOOD_COLOR_RED; }
	void Touch(CBaseEntity *pOther) override;
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) override;

	// RAII support to prevent memory leak.
	CMonster();

public:
	int LookupSequence(const char *label);

public:
	void EXPORT IdleThink();
	void Remove();

	void Wander();
	
	CBaseEntity *CheckTraceHullAttack(float flDist, int iDamage, int iDmgType);
	bool ShouldAttack(CBaseEntity *target) const;

	void SetAnimation(MonsterAnim anim);

	bool CheckTarget();
	bool CheckAttack();
	bool CheckSequence();

protected:
	// pTarget, bCanSee
	std::pair<CBasePlayer *, bool> FindTarget() const;
	CBasePlayer *GetClosestPlayer(bool bVisible) const;
	float GetModifiedDamage(float flDamage, int nHitGroup) const;

public:
	float m_flAttackDist;
	duration_t m_flAttackRate;
	duration_t m_flAttackAnimTime;
	float m_flAttackDamage;
	int m_iKillBonusMoney;
	int m_iKillBonusFrags;
	MonsterClass m_iClass;
	EngineClock::time_point m_flTimeLastActive;
	EngineClock::time_point m_flTargetChange;
	EngineClock::time_point m_flSkillCheck;
	EngineClock::time_point m_flStop;

	std::map<std::string, int> m_mapLookupSequenceCache;
	std::mutex m_mutexSetAnimation;

	time_point_t m_flCustomSequenceUntilTime;
	int m_iCustomSequence;
public:
	std::unique_ptr<IBaseMonsterStrategy> m_pMonsterStrategy;
	std::shared_ptr<CMonster *> m_pLifeCycleHolder; // auto release with RAII
public:
	void SetClass(MonsterClass iClass);
	void SetSkill(std::unique_ptr<IBaseMonsterSkill> ptr);
};

class CMonsterClass_Normal : public CBaseMonsterClass
{
public:
	CMonsterClass_Normal(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
};

class CMonsterSkill_Dog : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Dog(CMonster* p) : CBaseMonsterSkill(p) {}

	void OnActivate(CBaseEntity* target) override;
	void OnTouch(CBaseEntity* other) override;
	void OnSkillEnd() override;
};

class CMonsterClass_Dog : public CBaseMonsterClass
{
public:
	CMonsterClass_Dog(CMonster* p) : CBaseMonsterClass(p) {}

	void OnSpawn() override;
	void OnInit() override;
	const Vector GetCenterid() const override;
	void DeathSound() const override;
};

class CMonsterClass_Speed : public CBaseMonsterClass
{
public:
	CMonsterClass_Speed(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void DeathSound() const override;
};

class CMonsterSkill_Heavy : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Heavy(CMonster* p) : CBaseMonsterSkill(p) {}

	void OnActivate(CBaseEntity* target) override;
	void OnSkillEnd() override;
	duration_t GetTrapTime();
};

class CMonsterClass_Heavy : public CBaseMonsterClass
{
public:
	CMonsterClass_Heavy(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
};

class CMonsterClass_Thrower : public CBaseMonsterClass
{
public:
	CMonsterClass_Thrower(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
};

class CMonsterSkill_Heal : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Heal(CMonster* p) : CBaseMonsterSkill(p) {}

	void OnActivate(CBaseEntity* target) override;
	void OnSkillEnd() override;
	float GetHealBonus();
};

class CMonsterClass_Heal : public CBaseMonsterClass
{
public:
	CMonsterClass_Heal(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
};

class CMonsterSkill_Boomer : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Boomer(CMonster* p) : CBaseMonsterSkill(p) {}

	void OnKilled(entvars_t* pKiller, int iGib) override;

	float GetDamage();
};


class CMonsterClass_Boomer : public CBaseMonsterClass
{
public:
	CMonsterClass_Boomer(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void DeathSound() const override;
};

class CMonsterSkill_Resident : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Resident(CMonster* p) : CBaseMonsterSkill(p) {}

	void OnActivate(CBaseEntity* target) override;
	void OnSkillEnd() override;
	float GetDamage();
};

class CMonsterClass_Resident : public CBaseMonsterClass
{
public:
	CMonsterClass_Resident(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void DeathSound() const override;
};

class CMonsterSkill_Deimos : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Deimos(CMonster* p) : CBaseMonsterSkill(p) { m_flTimeTriggerZombieSkill = invalid_time_point; }

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void OnSkillEnd() override;
	void FireZombieSkill();
	time_point_t m_flTimeTriggerZombieSkill;

};

class CMonsterClass_Deimos : public CBaseMonsterClass
{
public:
	CMonsterClass_Deimos(CMonster* p) : CBaseMonsterClass(p) {}
	void OnSpawn() override;
	void OnInit() override;
};

class CMonsterClass_Helmet : public CBaseMonsterClass
{
public:
	CMonsterClass_Helmet(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
};

class CMonsterSkill_Deimos2 : public CBaseMonsterSkill
{
public:
	CMonsterSkill_Deimos2(CMonster* p) : CBaseMonsterSkill(p) { m_flTimeTriggerZombieSkill = invalid_time_point; }

	void OnThink() override;
	void OnActivate(CBaseEntity* target) override;
	void OnTouch(CBaseEntity* other) override;
	void OnSkillEnd() override;
	void FireZombieSkill();
	float GetDamage();
	time_point_t m_flTimeTriggerZombieSkill;

};

class CMonsterClass_Deimos2 : public CBaseMonsterClass
{
public:
	CMonsterClass_Deimos2(CMonster* p) : CBaseMonsterClass(p) {}

	void OnInit() override;
	void OnSpawn() override;
};

}

#endif
