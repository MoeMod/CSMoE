
#ifndef ZS_SUBS_H
#define ZS_SUBS_H
#ifdef _WIN32
#pragma once
#endif

#include "func_break.h" // CBreakable

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
	void KeyValue(KeyValueData *pkvd) override;
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;

public:
	float m_flHumanDamageRatio;
	float m_flZombiDamageRatio;

};

#endif
