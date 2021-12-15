/*
mod_zbz.h - CSMoE Gameplay server : Zombie Z
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef PROJECT_MOD_ZBZ_H
#define PROJECT_MOD_ZBZ_H

#include "mod_zb3.h"
#include "zbz/zbz_skill.h"
#include "usercmd.h"

#include <set>

namespace sv {
class CZombieZSkill_Base;
class CMod_ZombieZ : public CMod_ZombieHero
{
public:
	CMod_ZombieZ();
	void InstallPlayerModStrategy(CBasePlayer *) override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	float GetAdjustedEntityDamage(CBaseEntity* victim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	HitBoxGroup GetAdjustedTraceAttackHitgroup(CBaseEntity* victim, entvars_t* pevAttacker, float flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType) override;

public:
	EventDispatcher<void(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType)> m_eventTakeDamage;
	EventDispatcher<void(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType)> m_eventTraceAttack;
};

class CPlayerModStrategy_ZBZ : public CPlayerModStrategy_ZB3
{
public:
	CPlayerModStrategy_ZBZ(CBasePlayer *player, CMod_ZombieZ *mp);

	void OnSpawn() override;
	void BecomeHuman() override;
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker) override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void Event_AdjustHumanDamage(CBasePlayer* attacker, float& flDamage) override;
	void OnThink() override;
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override;
	void Event_OnRoundStart() override;
	void CmdStart(struct usercmd_s* cmd, unsigned int random_seed) override;
	void OnResetMaxSpeed() override;

	virtual void Event_TakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType);
	virtual void Event_TraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType);

protected:
	EventListener m_eventTakeDamageListener;
	EventListener m_eventTraceAttackListener;
private:
	CMod_ZombieZ* const m_pModZBZ;
	
	unsigned int m_iLevel;
	unsigned int m_iExp;
	unsigned int m_iPoint;
	float m_flTakeDamage;
	float m_flSufferDamage;
	void AddExp(int exp);
	
	std::set<std::shared_ptr<CZombieZSkill_Base>> m_vecSkill;
	std::set<byte> m_setSkillID;
};

}

#endif //PROJECT_MOD_ZBZ_H
