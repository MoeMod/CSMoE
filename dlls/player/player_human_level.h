#ifndef PLAYER_HUMAN_LEVEL_H
#define PLAYER_HUMAN_LEVEL_H
#ifdef _WIN32
#pragma once
#endif

class PlayerExtraHumanLevel_ZBS : public BasePlayerExtra
{
public:
	PlayerExtraHumanLevel_ZBS(CBasePlayer *player);

	float GetHealthBonus();
	float GetAttackBonus();

	void LevelUpHealth();
	void LevelUpAttack();
	void Reset();
	void UpdateHUD();

protected:
	int m_iHealth;
	int m_iAttack;
};

#endif