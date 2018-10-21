
#ifndef MOD_ZB2_H
#define MOD_ZB2_H
#ifdef _WIN32
#pragma once
#endif

#include "mod_zb1.h"

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

protected:
	void MakeSupplyboxThink();
	void RemoveAllSupplybox();
	CSupplyBox *CreateSupplybox();
	int SupplyboxCount();

public:
	void HumanInfectionByZombie(CBasePlayer *player, CBasePlayer *attacker) override;
	void MakeZombie(CBasePlayer *player, ZombieLevel iEvolutionLevel) override;

public:
	virtual void UpdatePlayerEvolutionHUD(CBasePlayer *player);
	virtual bool CanUseZombieSkill(CBasePlayer *player);

protected:
	float m_flTimeNextMakeSupplybox;
};

#endif
