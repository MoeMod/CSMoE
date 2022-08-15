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

namespace sv {

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
	virtual void OnTouch(CBaseEntity* other) = 0;
	virtual void OnSpawn() = 0;
	virtual void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void OnResetMaxSpeed() = 0;
	virtual void OnInitHUD() = 0;
	virtual void OnPostThink() = 0;

	virtual void CheckBuyZone() = 0;
	virtual bool CanPlayerBuy(bool display) = 0;
	virtual void Pain(int m_LastHitGroup, bool HasArmour) = 0;
	virtual void DeathSound() = 0;

	virtual int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) = 0;
	virtual bool ClientCommand(const char *pcmd) = 0;
	virtual float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData &data) = 0;
	virtual bool CanDropWeapon(const char *pszItemName = nullptr) = 0;
	virtual void GiveDefaultItems() = 0;
	virtual void CmdStart(struct usercmd_s *cmd, unsigned int random_seed) = 0;
	virtual void UpdateClientData(int sendweapons, struct clientdata_s *cd, entvars_t *pevOrg) = 0;
	virtual BOOL DeployWeapon(CBasePlayerItem *item) = 0;

	bool GiveSlotAmmo(CBasePlayer* player, int nSlot);
	bool GiveGunAmmo(CBasePlayer* player, CBasePlayerItem* weapon);

	virtual int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) = 0;
	virtual int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) = 0;
	virtual float AdjustDamageGivenToPlayer(entvars_t *pevInflictor, CBasePlayer *victim, float flDamage, int bitsDamageType) = 0;
	virtual duration_t OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t fDelay) = 0;
	virtual void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int &iClip, int &iBackAmmo) = 0;
	virtual int AdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int iClip) = 0;
	virtual float AdjustZombieKnifeRange(bool bStab, float flRange) = 0;
	virtual int AdjustScoreAttrib(int state) = 0;
	virtual Vector AdjustFireBullets3Dir(Vector vecDir) = 0;
	virtual void ClientPutInServer() = 0;
	virtual void ShootGrenade(const std::function<void(void)>& f) = 0;
	virtual void GrenadeExploded(Vector vecStart) = 0;
	virtual void ZombiBombExploded(float &flKickRate, float &flRadius, float &flDamage) = 0;
	virtual void FlashbangExploded(float& flKickRate, float& flRadius, time_point_t& flFreezeTime) = 0;
	virtual void Holster_Post(CBasePlayerWeapon* weapon) = 0;
	virtual void UpdateStatusBar(CBaseEntity *pEntity, char *sbuf0, int* newSBarState) = 0;
	virtual BOOL OnReactToDeimosTail() = 0;
	
};

inline IBasePlayerModStrategy::~IBasePlayerModStrategy() = default;

class CPlayerModStrategy_Default : public BasePlayerExtra, virtual public IBasePlayerModStrategy
{
public:
	explicit CPlayerModStrategy_Default(CBasePlayer *p) : BasePlayerExtra(p) {}

	void OnThink() override {}
	void OnTouch(CBaseEntity* other) override {}
	void OnSpawn() override {}
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override {}
	void OnResetMaxSpeed() override {}
	void OnInitHUD() override {}
	void OnPostThink() override {}

	void CheckBuyZone() override;
	bool CanPlayerBuy(bool display) override;
	void Pain(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound() override;

	int ComputeMaxAmmo(const char *szAmmoClassName, int iOriginalMax) override { return iOriginalMax; }
	bool ClientCommand(const char *pcmd) override { return false;}
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override { return flDamage; }
	bool ApplyKnockback(CBasePlayer *attacker, const KnockbackData &data) override { return false; }
	bool CanDropWeapon(const char *pszItemName) override { return true; }
	void GiveDefaultItems() override;

	void CmdStart(struct usercmd_s *cmd, unsigned int random_seed) override;
	void UpdateClientData(int sendweapons, struct clientdata_s *cd, entvars_t *pevOrg) override;

	BOOL DeployWeapon(CBasePlayerItem *item) override { return item->Deploy(); }

	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override { return 1; }
	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override { return 1; }
	float AdjustDamageGivenToPlayer(entvars_t *pevInflictor, CBasePlayer *victim, float flDamage, int bitsDamageType) override { return flDamage; }
	duration_t OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t fDelay) override { return fDelay; }
	void OnAttack_Post(CBasePlayerWeapon* weapon, int iClipBeforeAttack, int &iClip, int &iBackAmmo) override {}
	int AdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int iClip) override { return iClip; }
	float AdjustZombieKnifeRange(bool bStab, float flRange) override { return flRange; }
	int AdjustScoreAttrib(int state) override { return state; }
	Vector AdjustFireBullets3Dir(Vector vecDir) override { return vecDir; }
	void ClientPutInServer() {}
	void ShootGrenade(const std::function<void(void)>& f) {}
	void GrenadeExploded(Vector vecStart) {}
	void ZombiBombExploded(float& flKickRate, float& flRadius, float& flDamage) {};
	void FlashbangExploded(float& flKickRate, float& flRadius, time_point_t& flFreezeTime) {};
	void Holster_Post(CBasePlayerWeapon* weapon) {};
	void UpdateStatusBar(CBaseEntity* pEntity, char* sbuf0, int *newSBarState) override {};
	BOOL OnReactToDeimosTail() override { return false; };
};

class CPlayerModStrategy_Zombie : public CPlayerModStrategy_Default
{
public:
	explicit CPlayerModStrategy_Zombie(CBasePlayer *p) : CPlayerModStrategy_Default(p) {}

public:
	void Pain(int m_LastHitGroup, bool HasArmour) override;
	void DeathSound() override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void GiveDefaultItems() override;
	bool CanPlayerBuy(bool display) override;
	
};

}

#endif