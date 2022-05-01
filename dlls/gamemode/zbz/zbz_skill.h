/*
zbz_skill.h - Zombie Z Skill System (Server)
Copyright (C) 2019 Moemod Hyakuya

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef ZBZ_SKILL_H
#define ZBZ_SKILL_H
#ifdef _WIN32
#pragma once
#endif

#include "player/player_mod_strategy.h"
#include "gamemode/zbz/zbz_const.h"
#include "gamemode/mod_zbz.h"

#include <set>

namespace sv {

class IZombieZSkill
{
public:
	virtual ~IZombieZSkill() = 0;

	virtual void OnGetSkill() = 0;
	virtual void OnThink() = 0;
	virtual void OnTouch(CBaseEntity* other) = 0;
	virtual void OnRoundStart() = 0;
	virtual void OnSpawn() = 0;
	virtual void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) = 0;
	virtual float OnAdjustDamageTaken(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual void OnAdjustDamage(CBasePlayer* attacker, float& flDamage) = 0;
	virtual void OnInfection(CBasePlayer* victim, CBasePlayer* attacker) = 0;
	virtual void OnBecomeHuman() = 0;
	virtual void OnBecomeZombie(ZombieLevel iEvolutionLevel) = 0;
	virtual void OnButtonEvent(unsigned short& bitsCurButton, int bitsOldButton) = 0;
	virtual void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType) = 0;
	virtual void OnTraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType) = 0;
	virtual void OnResetMaxSpeed() = 0;
	virtual void OnApplyKnockback(CBasePlayer* attacker, KnockbackData& data) = 0;
	virtual void OnAddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) = 0;
	virtual void OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t& fDelay) = 0;
	virtual void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int& iClip, int& iBackAmmo) = 0;
	virtual void OnAdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int& iClip) = 0;
	virtual void OnComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax, int& iMax) = 0;
	virtual void OnAdjustZombieKnifeRange(bool bStab, float& flRange) = 0;
	virtual void OnAdjustFireBullets3Dir(Vector& vecDir) = 0;
	virtual void OnShootGrenade(Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent) = 0;
	virtual void OnGrenadeExploded(Vector vecOrigin) = 0;
	virtual void OnZombiBombExploded(float& flKickRate, float& flRadius, float& flDamage) = 0;
	virtual void OnEvolution(ZombieLevel iEvolutionLevel) = 0;

	virtual bool IsActivate() = 0;
	virtual const char* GetName() const = 0;
	virtual const char* GetDesc() const = 0;
	virtual ZombieZSkillType GetType() const = 0;
};

inline IZombieZSkill::~IZombieZSkill() = default;

class CZombieZSkill_Base : public BasePlayerExtra, public IZombieZSkill
{
public:
	explicit CZombieZSkill_Base(CBasePlayer* player) : BasePlayerExtra(player) {}

	void OnGetSkill() override {}
	void OnThink() override {}
	void OnTouch(CBaseEntity* other) override {}
	void OnRoundStart() override {}
	void OnSpawn() override {}
	void OnKilled(entvars_t* pKiller, entvars_t* pInflictor) override {}
	float OnAdjustDamageTaken(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) { return flDamage; }
	void OnAdjustDamage(CBasePlayer* attacker, float& flDamage) override {}
	void OnInfection(CBasePlayer* victim, CBasePlayer* attacker) override {}
	void OnBecomeHuman() override {}
	void OnBecomeZombie(ZombieLevel iEvolutionLevel) override {}
	void OnButtonEvent(unsigned short& bitsCurButton, int bitsOldButton) override {}
	void OnTakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType) override {}
	void OnTraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType) override {}
	void OnResetMaxSpeed() override {}
	void OnApplyKnockback(CBasePlayer* attacker, KnockbackData& data) override {}
	void OnAddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override {}
	void OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t& fDelay) override {}
	void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int& iClip, int& iBackAmmo) override {}
	void OnAdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int& iClip) override {}
	void OnComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax, int& iMax) override {}
	void OnAdjustZombieKnifeRange(bool bStab, float& flRange) override {}
	void OnAdjustFireBullets3Dir(Vector& vecDir) override {}
	void OnShootGrenade(Vector vecStart, Vector vecVelocity, duration_t time, int iTeam, unsigned short usEvent) override {}
	void OnGrenadeExploded(Vector vecOrigin) override {}
	void OnZombiBombExploded(float& flKickRate, float& flRadius, float& flDamage) override {};
	void OnEvolution(ZombieLevel iEvolutionLevel) override {}

	bool IsActivate() override { 
		return (GetType() == TYPE_HUMAN && !m_pPlayer->m_bIsZombie)
			|| (GetType() == TYPE_ZOMBIE && m_pPlayer->m_bIsZombie);
	}
	const char* GetName() const override { return ""; };
	const char* GetDesc() const override { return ""; };
	ZombieZSkillType GetType() const override { return TYPE_GENERAL; }
};

void Skill_Precache();
std::pair<byte, std::shared_ptr<CZombieZSkill_Base>> ZSkillFactory(CBasePlayer* player, const std::set<byte>& skillIDSet);
}

#endif
