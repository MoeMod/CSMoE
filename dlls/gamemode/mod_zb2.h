/*
mod_zb2.h - CSMoE Gameplay server : Zombie Mod 2
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

#ifndef MOD_ZB2_H
#define MOD_ZB2_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_zb1.h"

#include "player/player_zombie_skill.h"

#include "EventDispatcher.h"

#include "dlls/gamemode/zb2/zb2_zclass.h"

class CSupplyBox;

class CMod_ZombieMod2 : public CMod_Zombi
{
public:
	CMod_ZombieMod2();

public:
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void Think() override;
	void PlayerSpawn(CBasePlayer *pPlayer) override;
	void PlayerThink(CBasePlayer *pPlayer) override;
	BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd) override;

public: // IBaseMod
	void InstallPlayerModStrategy(CBasePlayer *player) override;

protected:
	void MakeSupplyboxThink();
	void RemoveAllSupplybox();
	CSupplyBox *CreateSupplybox();
	int SupplyboxCount();

public:
	void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker) override;

public:
	EventDispatcher<void(CBasePlayer *victim, CBasePlayer *attacker)> m_eventInfection;

protected:
	float m_flTimeNextMakeSupplybox;
};

class CPlayerModStrategy_ZB2 : public CPlayerModStrategy_ZB1
{
public:
	CPlayerModStrategy_ZB2(CBasePlayer *player, CMod_ZombieMod2 *mp);

	bool ClientCommand(const char *pcmd) override;
	void OnSpawn() override;
	void OnThink() override;
	void Pain(int m_LastHitGroup, bool HasArmour) override;

protected:
	virtual bool CanUseZombieSkill();
	virtual void Zombie_HealthRecoveryThink();
	virtual void UpdatePlayerEvolutionHUD();
	virtual void CheckEvolution();
	virtual void EvolutionSound() const;

protected:
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void BecomeHuman() override;
	virtual void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker);
	EventListener m_eventInfectionListener;

	
private:
	CMod_ZombieMod2 * const m_pModZB2;

	//std::unique_ptr<IZombieSkill> m_pZombieSkill;
	std::shared_ptr<IZombieModeCharacter_ZB2_Extra> m_pCharacter_ZB2;
	float m_flTimeNextZombieHealthRecovery;
	int m_iZombieInfections;
};

#endif
