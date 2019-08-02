
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

class CMonster;

struct BaseMonsterExtra
{
public:
	BaseMonsterExtra(CMonster *p) : m_pMonster(p) {}

public:
	CMonster * const m_pMonster;
};

class IBaseMonsterStrategy : public BaseMonsterExtra
{
public:
	IBaseMonsterStrategy(CMonster *p) : BaseMonsterExtra(p) {}
	virtual ~IBaseMonsterStrategy() = 0;

	virtual void OnSpawn() = 0;
	virtual void OnThink() = 0;
	virtual void OnKilled(entvars_t *pKiller, int iGib) = 0;

	virtual void DeathSound() const = 0;
	virtual bool IsTeamMate(CBaseEntity *that) const = 0;
};

inline IBaseMonsterStrategy::~IBaseMonsterStrategy() {}

class CMonsterModStrategy_Default : public IBaseMonsterStrategy
{
public:
	CMonsterModStrategy_Default(CMonster *p) : IBaseMonsterStrategy(p) {}

	void OnSpawn() override;
	void OnThink() override;
	void OnKilled(entvars_t *pKiller, int iGib) override;

	void DeathSound() const override;
	bool IsTeamMate(CBaseEntity *that) const override { return false; }
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
	~CMonster() override;

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
	EngineClock::time_point m_flTimeLastActive;
	EngineClock::time_point m_flTargetChange;

	std::map<std::string, int> m_mapLookupSequenceCache;
	std::mutex m_mutexSetAnimation;

public:
	std::unique_ptr<IBaseMonsterStrategy> m_pMonsterStrategy;

};

}

#endif
