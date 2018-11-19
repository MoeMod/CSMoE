#ifndef PLAYER_ZOMBIE_SKILL_H
#define PLAYER_ZOMBIE_SKILL_H
#ifdef _WIN32
#pragma once
#endif

void ZombieSkill_Precache();

#include "player_mod_strategy.h"
#include "gamemode/zb2/zb2_const.h"

class IZombieSkill : public BasePlayerExtra
{
public:
	IZombieSkill(CBasePlayer *player);

	virtual void InitHUD();
	virtual void Think();
	virtual void Activate() {}
	virtual void Reset() { m_iZombieSkillStatus = SKILL_STATUS_READY; }
	virtual void ResetMaxSpeed() {}
	virtual float GetDamageRatio() { return 1.0f; }

protected:
	virtual void OnSkillEnd() {}
	virtual void OnSkillReady() {}

public:
	ZombieSkillStatus GetStatus() {	return m_iZombieSkillStatus; }

protected:
	ZombieSkillStatus m_iZombieSkillStatus;
	float m_flTimeZombieSkillEnd;
	float m_flTimeZombieSkillNext;
	
};

class CZombieSkill_ZombieCrazy : public IZombieSkill
{
public:
	CZombieSkill_ZombieCrazy(CBasePlayer *player);

public:
	void InitHUD() override;
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() override;

protected:
	void OnCrazyEffect();

	float m_flTimeZombieSkillEffect;
};


#endif