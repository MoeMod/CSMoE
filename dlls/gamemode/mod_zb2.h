
#ifndef MOD_ZB2_H
#define MOD_ZB2_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_zb1.h"

#include "player/player_zombie_skill.h"

#include "EventDispatcher.h"

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
	void MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel) override;

public:
	EventDispatcher<void(CBasePlayer *who, ZombieLevel iEvolutionLevel)> m_eventBecomeZombie;
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
	void OnResetMaxSpeed() override;
	void Pain(int m_LastHitGroup, bool HasArmour) override;

protected:
	virtual bool CanUseZombieSkill();
	virtual void Zombie_HealthRecoveryThink();
	virtual void UpdatePlayerEvolutionHUD();

protected:
	virtual void Event_OnBecomeZombie(CBasePlayer *who, ZombieLevel iEvolutionLevel);
	virtual void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker);
	EventListener m_eventBecomeZombieListener;
	EventListener m_eventInfectionListener;
	
private:
	CMod_ZombieMod2 * const m_pModZB2;

	std::unique_ptr<IZombieSkill> m_pZombieSkill;
	float m_flTimeNextZombieHealthRecovery;
	int m_iZombieInfections;
};

#endif
