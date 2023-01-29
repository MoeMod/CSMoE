/*
zclass.h - CSMoE Gameplay server : zombie class for zb2
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

#ifndef PROJECT_ZCLASS_H
#define PROJECT_ZCLASS_H
#pragma once
#include "gamemode/zb1/zb1_zclass.h"
#include "gamemode/zb2/zb2_const.h"

namespace sv {

class IZombieModeCharacter_ZB2 : virtual public IZombieModeCharacter
{
public:
	virtual float HealthRecoveryAmount() const = 0;
	virtual void ActivateSkill(ZombieSkillSlot which) = 0;
	virtual void InitHUD() const = 0;
	virtual ZombieSkillStatus GetSkillStatus(ZombieSkillSlot which) const = 0;
	virtual void OnWeaponDeploy(CBasePlayerItem *item) = 0;
	virtual int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) = 0;
	virtual void Evolution(ZombieLevel iEvolutionLevel) = 0;
	virtual float GetZombiMaxHealth(ZombieLevel iEvolutionLevel) = 0;
};

class CHuman_ZB2 : public CHuman_ZB1, virtual public IZombieModeCharacter_ZB2
{
public:
	explicit CHuman_ZB2(CBasePlayer *player);
	~CHuman_ZB2() override;
	void ActivateSkill(ZombieSkillSlot which) override;
	float HealthRecoveryAmount() const override { return 0.0f; }
	void InitHUD() const override;
	void Think() override;
	void ResetMaxSpeed() const override;
	ZombieSkillStatus GetSkillStatus(ZombieSkillSlot which) const override;
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override {}
	void DeathSound_Zombie() override {}
	void OnWeaponDeploy(CBasePlayerItem *item) override {}
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override { return 1; }
	void Evolution(ZombieLevel iEvolutionLevel) override {}
	float GetZombiMaxHealth(ZombieLevel iEvolutionLevel) override { return 0.0f; }


protected:
	class impl_t;
	const std::unique_ptr<impl_t> pimpl;

public:
	time_point_t beingTrapped;
	time_point_t m_tSlowdown;
};

class IZombieSkill;

class CBaseZombieClass_ZB2 : public CZombie_ZB1, virtual public IZombieModeCharacter_ZB2
{
public:
	explicit CBaseZombieClass_ZB2(CBasePlayer *player, ZombieLevel lv) : CZombie_ZB1(player, lv) {}
	~CBaseZombieClass_ZB2() override;
	float HealthRecoveryAmount() const override;
	void ActivateSkill(ZombieSkillSlot which) override;
	void InitHUD() const override;
	void Think() override;
	void ResetMaxSpeed() const override;
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData & kbd) override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) const override;
	ZombieSkillStatus GetSkillStatus(ZombieSkillSlot which) const override;
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override { return 1; }
	void Evolution(ZombieLevel iEvolutionLevel);
	float GetZombiMaxHealth(ZombieLevel iEvolutionLevel) override
	{
		if (iEvolutionLevel == 0)
			return 4000.0f;
		else if (iEvolutionLevel == 1)
			return 7000.0f;
		else
			return 14000.0f;
	}

	time_point_t m_tBanSkillsTime;
	bool m_bForceStopSkills;
	bool m_bCanUseSkills;

protected:
	std::unique_ptr<IZombieSkill> m_pZombieSkill;
	std::unique_ptr<IZombieSkill> m_pZombieSkill2;
};

void ZombieSkill_Precache();
void HumanSkill_Precache();

}

#endif //PROJECT_ZCLASS_H
