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

#include "gamemode/zb1/zb1_zclass.h"
#include "gamemode/zb2/zb2_const.h"

namespace sv {

class IZombieModeCharacter_ZB2_Extra
{
public:
	virtual float HealthRecoveryAmount() const = 0;
	virtual void ActivateSkill(ZombieSkillSlot which) = 0;
	virtual void InitHUD() const = 0;
	virtual ZombieSkillStatus GetSkillStatus(ZombieSkillSlot which) const = 0;
	virtual std::string GetZombieKnifeViewModel() const = 0;
	virtual std::string GetZombieBombViewModel() const = 0;
};

class CHuman_ZB2 : public CHuman_ZB1, public IZombieModeCharacter_ZB2_Extra
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
	std::string GetZombieKnifeViewModel() const override { return {}; }
	std::string GetZombieBombViewModel() const override { return {}; }

protected:
	class impl_t;
	const std::unique_ptr<impl_t> pimpl;
};

class IZombieSkill;

class CBaseZombieClass_ZB2 : public CZombie_ZB1, public IZombieModeCharacter_ZB2_Extra
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

protected:
	std::unique_ptr<IZombieSkill> m_pZombieSkill;
};

void ZombieSkill_Precache();
void HumanSkill_Precache();

std::shared_ptr<CBaseZombieClass_ZB2> ZombieClassFactory(CBasePlayer *player, ZombieLevel lv, const char *name = nullptr);

}

#endif //PROJECT_ZCLASS_H
