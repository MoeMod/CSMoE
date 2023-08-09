
#ifndef ZS_SUBS_H
#define ZS_SUBS_H
#ifdef _WIN32
#pragma once
#endif

#include "func_break.h" // CBreakable
#include "triggers.h"

namespace sv {

class CZombieSpawn: public CPointEntity
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData *pkvd) override;
	BOOL IsTriggered(CBaseEntity *pEntity) override;

public:
	int m_iId;
	// pev->origin, pev_angles
};

class CZBSBreak : public CBreakable
{
public:
	void Spawn() override;
	void KeyValue(KeyValueData *pkvd) override;
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void EXPORT BreakTouch(CBaseEntity *pOther);
public:
	float m_flHumanDamageRatio;
	float m_flZombiDamageRatio;

};

class CZBSTrigger : public CBaseTrigger
{
public:
	void Spawn() override;
	void EXPORT TriggerThink();
	void EXPORT TriggerTouch(CBaseEntity* pOther);
	void KeyValue(KeyValueData* pkvd) override;

public:
	bool m_bTriggerClosed;
	int m_iRound;
	int m_iCurentRound;
	int m_iTriggerId;
};


}

#endif
