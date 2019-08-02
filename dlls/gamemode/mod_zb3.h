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

#ifndef PROJECT_MOD_ZB3_H
#define PROJECT_MOD_ZB3_H

#include "mod_zb2.h"
#include "zb3/zb3_const.h"
#include "zb3/zb3_morale.h"

namespace sv {

class CMod_ZombieHero : public CMod_ZombieMod2
{
public:
	CMod_ZombieHero();
	void InstallPlayerModStrategy(CBasePlayer *) override;
	void PickZombieOrigin() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	void CheckWinConditions() override;

	const CZB3HumanMorale &HumanMorale() const { return m_Morale; }

	EventDispatcher<void(CBasePlayer *who)> m_eventBecomeHero;
	EventDispatcher<void()> m_eventRoundStart;

protected:
	void PickHero();
	void MakeHero(CBasePlayer *p) { m_eventBecomeHero.dispatch(p); }

	CZB3HumanMorale m_Morale;
};

class CPlayerModStrategy_ZB3 : public CPlayerModStrategy_ZB2
{
public:
	CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp);

	void OnSpawn() override;
	bool CanUseZombieSkill() override { return true; }
	void CheckEvolution() override;
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker) override;
	void UpdatePlayerEvolutionHUD() override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void OnThink() override;
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override;

	virtual void Event_OnBecomeHero(CBasePlayer *who);
	virtual void Event_OnRoundStart();

	bool IsHero() const { return m_pPlayer->m_bIsVIP; }

private:
	CMod_ZombieHero * const m_pModZB3;
	const EventListener m_eventBecomeHeroListener;
	const EventListener m_eventRoundStartListener;

	float m_flRagePercent; // [0.0-100.0]
	EngineClock::time_point m_flDeadTime;
	float m_flBackupMaxHealth;
	float m_flBackupArmor;
};

}

#endif //PROJECT_MOD_ZB3_H
