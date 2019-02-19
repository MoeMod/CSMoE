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
	explicit IZombieSkill(CBasePlayer *player) : BasePlayerExtra(player) {}
	virtual ~IZombieSkill() = default;

	virtual void InitHUD() = 0;
	virtual void Think() = 0;
	virtual void Activate() = 0;
	virtual void Reset() = 0;
	virtual void ResetMaxSpeed() = 0;
	virtual float GetDamageRatio() const = 0;
	virtual ZombieSkillStatus GetStatus() const = 0;
	
};

class CZombieSkill_Base : public IZombieSkill
{
public:
	explicit CZombieSkill_Base(CBasePlayer *player);

	void InitHUD() override;
	void Think() override;
	void Activate() override {}
	void Reset() override { m_iZombieSkillStatus = SKILL_STATUS_READY; }
	void ResetMaxSpeed() override {}
	float GetDamageRatio() const override { return 1.0f; }
	ZombieSkillStatus GetStatus() const override {	return m_iZombieSkillStatus; }

protected:
	virtual void OnSkillEnd() {}
	virtual void OnSkillReady() {}

protected:
	ZombieSkillStatus m_iZombieSkillStatus;
	float m_flTimeZombieSkillEnd;
	float m_flTimeZombieSkillNext;

};

class CZombieSkill_Empty : public IZombieSkill
{
public:
	explicit CZombieSkill_Empty(CBasePlayer *player)  : IZombieSkill(player) {}

	void InitHUD() override {}
	void Think() override {}
	void Activate() override {}
	void Reset() override {}
	void ResetMaxSpeed() override {}
	float GetDamageRatio() const override { return 1.0f; }
	ZombieSkillStatus GetStatus() const override {	return SKILL_STATUS_USED; }
};

class CZombieSkill_ZombieCrazy : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieCrazy(CBasePlayer *player);

public:
	void InitHUD() override;
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazyEffect();

	float m_flTimeZombieSkillEffect;
};


#endif