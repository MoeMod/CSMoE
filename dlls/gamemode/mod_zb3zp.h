/*
mod_zb3.h - CSMoE Gameplay server : Zombie Hero
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

#ifndef PROJECT_MOD_ZB3ZP_H
#define PROJECT_MOD_ZB3ZP_H
#pragma once

#include "mod_zb3.h"
#include "usercmd.h"

struct Reciter;

namespace sv {
class CMod_ZB3ZP : public CMod_ZombieHero
{
public:
	void CheckMapConditions() override;
	void InstallPlayerModStrategy(CBasePlayer*) override;
	float GetAdjustedEntityDamage(CBaseEntity* victim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	void RestartRound() override;
	void HumanInfectionByZombie(CBasePlayer* player, CBasePlayer* attacker) override;

	EventDispatcher<void()> m_eventRestartRound;
	EventDispatcher<void(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType)> m_eventTakeDamage;
};

class CPlayerModStrategy_ZB3ZP : public CPlayerModStrategy_ZB3
{
public:
	CPlayerModStrategy_ZB3ZP(CBasePlayer* player, CMod_ZB3ZP* mp);

	using CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3;
	bool CanPlayerBuy(bool display) override;
	void GiveDefaultItems() override;
	std::shared_ptr<CBaseZombieClass_ZB2> ZombieClassFactory(ZombieLevel lv, const char* name) override;
	void CmdStart(struct usercmd_s* cmd, unsigned int random_seed) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);
	void OnSpawn() override;
	void Evolution(ZombieLevel iEvolutionLevel) override;
	void Event_TakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType);
	void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override;
	void OnThink() override;
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void Event_RestartRound();

protected:
	EventListener m_eventTakeDamageListener;
	EventListener m_RestartRoundListener;
private:
	int m_iKilledTime;
	time_point_t m_flNextCheckButton;
	time_point_t m_flNextTimeChangeZombie;

};

}

#endif //PROJECT_MOD_ZB3_H
