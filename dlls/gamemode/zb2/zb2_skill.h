/*
zb2_skill.h - CSMoE Gameplay server : CBasePlayer impl for zombie skill
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

#ifndef PLAYER_ZOMBIE_SKILL_H
#define PLAYER_ZOMBIE_SKILL_H
#ifdef _WIN32
#pragma once
#endif

#include "player/player_mod_strategy.h"
#include "gamemode/zb2/zb2_const.h"
#include "gamemode/mod_zb1.h"

namespace sv {

class IZombieSkill
{
public:
	virtual ~IZombieSkill() = 0;

	virtual void Think() = 0;
	virtual void Activate() = 0;
	virtual void Reset() = 0;
	virtual void ResetMaxSpeed() = 0;
	virtual float GetDamageRatio() const = 0;
	virtual ZombieSkillStatus GetStatus() const = 0;
	
};

inline IZombieSkill::~IZombieSkill() = default;

class CZombieSkill_Base : public BasePlayerExtra, public IZombieSkill
{
public:
	explicit CZombieSkill_Base(CBasePlayer *player);

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
	EngineClock::time_point m_flTimeZombieSkillEnd;
	EngineClock::time_point m_flTimeZombieSkillNext;

};

class CZombieSkill_Empty : public BasePlayerExtra, public IZombieSkill
{
public:
	explicit CZombieSkill_Empty(CBasePlayer *player)  : BasePlayerExtra(player) {}

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
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazyEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
};

class CZombieSkill_ZombieHide : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieHide(CBasePlayer *player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet);

protected:
	void OnHideEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
	EngineClock::time_point m_flInvisiable;
};

}


#endif