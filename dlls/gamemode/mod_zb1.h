/*
mod_zb1.h - CSMoE Gameplay server : Zombie Mod
Copyright (C) 2018 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef MOD_ZB1_H
#define MOD_ZB1_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_base.h"
#include "zb1/zb1_zclass.h"
#include "zb1/zb1_countdown.h"

#include <vector>
#include <utility>

#include "EventDispatcher.h"

class CMod_Zombi : public TBaseMod_RemoveObjects<TBaseMod_RandomSpawn<>>
{
public:
	CMod_Zombi();

public: // CHalfLifeMultiplay
	void CheckMapConditions() override;
	BOOL IsTeamplay(void) override { return TRUE; }
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void Think() override;
	BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char *szRejectReason) override;
	void ClientDisconnected(edict_t *pClient) override;
	BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) override;
	void CheckWinConditions() override;
	int IPointsForKill(CBasePlayer *pAttacker, CBasePlayer *pKilled) override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;

public: // IBaseMod
	DamageTrack_e DamageTrack() override { return DT_ZB; }
	void InstallPlayerModStrategy(CBasePlayer *player) override;

public:
	virtual size_t ZombieOriginNum();
	virtual void PickZombieOrigin();
	virtual void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker);
	virtual void RoundEndScore(int iWinStatus);

protected:
	void TeamCheck();
	void InfectionSound();

	void HumanWin();
	void ZombieWin();

	BOOL FInfectionStarted();

	void MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel) { m_eventBecomeZombie.dispatch(player, iEvolutionLevel); }

public:
	EventDispatcher<void(CBasePlayer *who, ZombieLevel iEvolutionLevel)> m_eventBecomeZombie;
	CModCountdownHelper m_Countdown;
};

class CPlayerModStrategy_ZB1 : public CPlayerModStrategy_Zombie
{
public:
	CPlayerModStrategy_ZB1(CBasePlayer *player, CMod_Zombi *mp);
	void CheckBuyZone() override { m_pPlayer->m_signals.Signal(SIGNAL_BUY); };
	bool CanPlayerBuy(bool display) override;
	int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) override;

	void OnSpawn() override;
	void OnThink() override { m_pCharacter->Think(); return CPlayerModStrategy_Zombie::OnThink(); }
	void OnResetMaxSpeed() override { m_pCharacter->ResetMaxSpeed(); return CPlayerModStrategy_Zombie::OnResetMaxSpeed(); }
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData &data) override { return m_pCharacter->ApplyKnockback(attacker, data); }
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void Pain(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound() override;

private:
	void Event_OnBecomeZombie(CBasePlayer *who, ZombieLevel iEvolutionLevel);
	const EventListener m_eventBecomeZombieListener;

public:
	virtual void BecomeZombie(ZombieLevel iEvolutionLevel);
	virtual void BecomeHuman();

	std::shared_ptr<IZombieModeCharacter> m_pCharacter;
};

class CZB1CountdownDelegate : public ICountdownDelegate
{
public:
	CZB1CountdownDelegate(CMod_Zombi* mod) : m_pMod(mod) {}
	void OnCountdownStart() override;
	void OnCountdownChanged(int iCurrentCount) override;
	void OnCountdownEnd() override;
protected:
	CMod_Zombi* const m_pMod;
};

#endif
