/*
player_mod_strategy.h - CSMoE Gameplay server : player strategy for gamemodes
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

#ifndef PLAYER_MOD_STRATEGY_H
#define PLAYER_MOD_STRATEGY_H

#include <player.h>

#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;

// Strategy Pattern
// impliments part of player to deduce unnecessary references...
struct BasePlayerExtra
{
public:
	explicit BasePlayerExtra(CBasePlayer *p) : m_pPlayer(p) {}
	
public:
	CBasePlayer * const m_pPlayer;
};

class IBasePlayerModStrategy
{
public:
	virtual ~IBasePlayerModStrategy() = 0;

	virtual void OnThink() = 0;
	virtual void OnSpawn() = 0;
	virtual void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void OnResetMaxSpeed() = 0;
	virtual void OnInitHUD() = 0;

	virtual void CheckBuyZone() = 0;
	virtual bool CanPlayerBuy(bool display) = 0;
	virtual void Pain(int m_LastHitGroup, bool HasArmour) = 0;
	virtual void DeathSound() = 0;

	virtual int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) = 0;
	virtual bool ClientCommand(const char *pcmd) = 0;
	virtual float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData &data) = 0;
};

inline IBasePlayerModStrategy::~IBasePlayerModStrategy() = default;

class CPlayerModStrategy_Default : public BasePlayerExtra, public IBasePlayerModStrategy
{
public:
	explicit CPlayerModStrategy_Default(CBasePlayer *p) : BasePlayerExtra(p), IBasePlayerModStrategy() {}

	void OnThink() override {}
	void OnSpawn() override {}
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override {}
	void OnResetMaxSpeed() override {}
	void OnInitHUD() override {}

	void CheckBuyZone() override;
	bool CanPlayerBuy(bool display) override;
	void Pain(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound() override;

	int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) override { return iOriginalMax; }
	bool ClientCommand(const char *pcmd) override { return false; }
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override { return flDamage; }
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData &data) override { return false; }
};

class IPlayerModStrategyExtra_Zombie
{
	virtual void Pain_Zombie(int m_LastHitGroup, bool HasArmour) = 0;
	virtual void DeathSound_Zombie() = 0;
};

class CPlayerModStrategy_Zombie : public CPlayerModStrategy_Default, public IPlayerModStrategyExtra_Zombie
{
public:
	explicit CPlayerModStrategy_Zombie(CBasePlayer *p) : CPlayerModStrategy_Default(p) {}

public:
	void Pain(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound() override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;

public:
	void Pain_Zombie(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound_Zombie() override;
};

#endif