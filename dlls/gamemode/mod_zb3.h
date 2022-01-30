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
#pragma once

#include "mod_zb2.h"
#include "zb3/zb3_const.h"
#include "zb3/zb3_morale.h"
#include "usercmd.h"

struct Reciter;

namespace sv {

class CMod_ZombieHero;
class CZB3CountdownDelegate : public CZB1CountdownDelegate {
public:
	CZB3CountdownDelegate(CMod_ZombieHero *mod);

	void OnCountdownStart() override;
	void OnCountdownChanged(int iCurrentCount) override;
	void OnCountdownEnd() override;
};

class CMod_ZombieHero : public CMod_ZombieMod2
{
public:
	CMod_ZombieHero();
	~CMod_ZombieHero();
	void InstallPlayerModStrategy(CBasePlayer *) override;
	void PickZombieOrigin() override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	void CheckWinConditions() override;
	BOOL FPlayerCanTakeDamage(CBasePlayer* pPlayer, CBaseEntity* pAttacker) override;
	void Think() override;
	int IPointsForKill(CBasePlayer* pAttacker, CBasePlayer* pKilled) override;
	bool Host_Say(edict_t *pEntity, int teamonly, const char *content) override;

	const CZB3HumanMorale& HumanMorale() const { return m_Morale; }
	bool isForceRespawn() const { return m_flForceRespawnEnd > gpGlobals->time; }
	virtual void MakeZombieOrigin();
	BOOL FInfectionStarted() override;

	EventDispatcher<void(CBasePlayer *who)> m_eventBecomeHero;
	EventDispatcher<void()> m_eventRoundStart;
	EventDispatcher<void(CBasePlayer* who)> m_eventPickAsZombie;

protected:
	virtual void PickHero();
	void MakeHero(CBasePlayer* p) { m_eventBecomeHero.dispatch(p); }
	void MakePickAsZombie(CBasePlayer *p) { m_eventPickAsZombie.dispatch(p); }

	void MakeReciter();

	CZB3HumanMorale m_Morale;
	std::unique_ptr<Reciter> m_pCurrentReciter;
	bool m_bHasAnswered = false;
	CZB3CountdownDelegate m_CountDownDelegate;
	time_point_t m_flForceRespawnEnd;
};

class CPlayerModStrategy_ZB3 : public CPlayerModStrategy_ZB2
{
public:
	CPlayerModStrategy_ZB3(CBasePlayer *player, CMod_ZombieHero *mp);

	void OnSpawn() override;
	bool CanUseZombieSkill() override { return true; }
	void CheckEvolution() override;
	void Evolution(ZombieLevel iEvolutionLevel) override;
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker) override;
	void UpdatePlayerEvolutionHUD() override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void OnThink() override;
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override;
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override;
	int AdjustScoreAttrib(int state) override { return IsHero() ? (state | PLAYER_VIP) : state; }
	void CmdStart(struct usercmd_s* cmd, unsigned int random_seed) override;

	virtual void Event_OnBecomeHero(CBasePlayer *who);
	virtual void Event_OnRoundStart();
	virtual void Event_OnPickAsZombie(CBasePlayer* who);

	bool IsHero() const { return m_bIsHero; }
	void SetHero(bool value) { m_bIsHero = value; }
	void SetRagePercent(float value) { m_flRagePercent = value; }
	bool IsPickAsZombie() const { return m_bPickAsZombie; }
	void SetPickAsZombie(bool value) { m_bPickAsZombie = value; }

private:
	CMod_ZombieHero * const m_pModZB3;
	const EventListener m_eventBecomeHeroListener;
	const EventListener m_eventRoundStartListener;
	const EventListener m_eventPickAsZombieListener;

	float m_flRagePercent; // [0.0-100.0]
	EngineClock::time_point m_flDeadTime;
	float m_flBackupMaxHealth;
	float m_flBackupArmor;
	bool m_bIsHero;
	time_point_t m_flGodMode;
	bool m_bPickAsZombie;
};

class CMod_ZB3F2P : public CMod_ZombieHero
{
public:
	void InstallPlayerModStrategy(CBasePlayer *) override;
	void PickHero() override {} // no hero
};

class CPlayerModStrategy_ZB3F2P : public CPlayerModStrategy_ZB3
{
public:
	using CPlayerModStrategy_ZB3::CPlayerModStrategy_ZB3;
	void GiveDefaultItems() override;
	std::shared_ptr<CBaseZombieClass_ZB2> ZombieClassFactory(ZombieLevel lv, const char *name) override;
};

}

#endif //PROJECT_MOD_ZB3_H
