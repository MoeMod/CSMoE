
#ifndef MONSTER_ENTITY_H
#define MONSTER_ENTITY_H
#ifdef _WIN32
#pragma once
#endif

#include "hostage/hostage.h"
#include <memory>

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

	virtual void DeathSound() = 0;
};

inline IBaseMonsterStrategy::~IBaseMonsterStrategy() {}

class CMonsterModStrategy_Default : public IBaseMonsterStrategy
{
public:
	CMonsterModStrategy_Default(CMonster *p) : IBaseMonsterStrategy(p) {}

	void OnSpawn() override;
	void OnThink() override;
	void OnKilled(entvars_t *pKiller, int iGib) override;

	void DeathSound() override;
};

class CMonster : public CHostage
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int ObjectCaps() override { return (CBaseMonster::ObjectCaps() | FCAP_MUST_SPAWN); }
	virtual int Classify() { return CLASS_PLAYER_ALLY; }
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	virtual void BecomeDead(void);
	virtual void Killed(entvars_t *pevAttacker, int iGib);
	virtual int BloodColor() { return BLOOD_COLOR_RED; }
	virtual void Touch(CBaseEntity *pOther);
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	// RAII support to prevent memory leak.
	CMonster();
	~CMonster();

public:
	void EXPORT IdleThink();
	void Remove();

	CBaseEntity *FindTarget();
	void Wander();
	CBaseEntity *CheckAttack();
	CBaseEntity *CheckTraceHullAttack(float flDist, int iDamage, int iDmgType);
	bool ShouldAttack(CBaseEntity *target);

	void PlayDeathSound();
	void SetAnimation(MonsterAnim anim);
	void CheckTarget();
	CBasePlayer *GetClosestPlayer(bool bVisible);

public:
	float m_flAttackDist;
	float m_flAttackRate;
	float m_flAttackAnimTime;
	float m_flAttackDamage;
	int m_iKillBonusMoney;
	int m_iKillBonusFrags;
	float m_flTimeLastActive;
	float m_flTargetChange;

public:
	std::unique_ptr<IBaseMonsterStrategy> m_pMonsterStrategy;

};

#endif
