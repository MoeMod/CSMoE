/*
mod_zbz.h - CSMoE Gameplay server : Zombie Z
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

#ifndef PROJECT_MOD_ZBZ_H
#define PROJECT_MOD_ZBZ_H

#include "mod_zb3.h"
#include "zbz/zbz_skill.h"
#include "usercmd.h"

#include <set>

namespace sv {
class CZombieZSkill_Base;
class CMod_ZombieZ : public CMod_ZombieHero
{
public:
	CMod_ZombieZ();
	void InstallPlayerModStrategy(CBasePlayer *) override;
	void UpdateGameMode(CBasePlayer *pPlayer) override;
	void RestartRound() override;
	void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) override;
	float GetAdjustedEntityDamage(CBaseEntity* victim, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override;
	HitBoxGroup GetAdjustedTraceAttackHitgroup(CBaseEntity* victim, entvars_t* pevAttacker, float flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType) override;
	void HumanWin() override;
	void ZombieWin() override;

protected:
	void PickHero() override;

public:
	EventDispatcher<void(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType)> m_eventTakeDamage;
	EventDispatcher<void(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType)> m_eventTraceAttack;
};

class CPlayerModStrategy_ZBZ : public CPlayerModStrategy_ZB3
{
public:
	CPlayerModStrategy_ZBZ(CBasePlayer *player, CMod_ZombieZ *mp);

	bool ClientCommand(const char* pcmd) override;
	void OnSpawn() override;
	void BecomeHuman() override;
	void BecomeZombie(ZombieLevel iEvolutionLevel) override;
	void Event_OnInfection(CBasePlayer *victim, CBasePlayer *attacker) override;
	float AdjustDamageTaken(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType) override;
	void Event_AdjustHumanDamage(CBasePlayer* attacker, float& flDamage) override;
	void OnThink() override;
	void OnTouch(CBaseEntity* other) override;
	void OnKilled(entvars_t *pKiller, entvars_t *pInflictor) override;
	void Event_OnRoundStart() override;
	void CmdStart(struct usercmd_s* cmd, unsigned int random_seed) override;
	void OnResetMaxSpeed() override;
	void OnInitHUD() override;
	bool ApplyKnockback(CBasePlayer* attacker, const KnockbackData& data) override;
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override;
	duration_t OnDefaultReload_Pre(CBasePlayerWeapon* weapon, duration_t fDelay) override;
	int AdjustDefaultReloadClip(CBasePlayerWeapon* weapon, int iClip) override;
	int ComputeMaxAmmo(const char* szAmmoClassName, int iOriginalMax) override;
	float AdjustZombieKnifeRange(bool bStab, float flRange) override;
	Vector AdjustFireBullets3Dir(Vector vecDir) override;
	void ClientPutInServer() override;

	virtual void Event_TakeDamage(CBasePlayer* victim, entvars_t* pevInflictor, CBasePlayer* pevAttacker, float& flDamage, int bitsDamageType);
	virtual void Event_TraceAttack(CBasePlayer* victim, CBasePlayer* pevAttacker, float& flDamage, const Vector& vecDir, TraceResult* ptr, int bitsDamageType);

	bool IsLadderDisabled() const { return m_bLadderDisabled; }
	void SetLadderDisabled(bool value) { m_bLadderDisabled = value; }
	bool IsOnFire() const { return m_pPlayer->IsAlive() && m_bIsFiring; }
	void SetOnFire(CBasePlayer *atk, duration_t value)
	{
		m_bIsFiring = true;
		m_vecFireList[atk->entindex()] = gpGlobals->time + value;

		MESSAGE_BEGIN(MSG_BROADCAST, gmsgZBZMsg);
		WRITE_BYTE(ZBZ_MESSAGE_BURN);
		WRITE_BYTE(ENTINDEX(m_pPlayer->edict()));
		WRITE_BYTE(value.count() * 10);
		MESSAGE_END();
	}
	void ClearFire() { m_bIsFiring = false;  m_vecFireList.fill(invalid_time_point); }

protected:
	void UpdatePlayerEvolutionHUD() override;

protected:
	EventListener m_eventTakeDamageListener;
	EventListener m_eventTraceAttackListener;

public:
	bool CheckSkill(ZombieZSkillSkillId id) const { return m_setSkillID.find(id) != m_setSkillID.end(); }
	void SetRoundStartAddLevel(bool value) { m_bRoundStartAddLevel = value; }

private:
	CMod_ZombieZ* const m_pModZBZ;
	
	unsigned int m_iLevel;
	unsigned int m_iExp;
	unsigned int m_iPoint;
	time_point_t m_flSkillDelay;
	float m_flTakeDamage;
	float m_flSufferDamage;
	float m_flExpScale;
	bool m_bRoundStartAddLevel;
	void AddExp(int exp);
	void AddLevel(int lv);
	void SetLevel(int lv);
	bool UpgradeSkill();

	std::set<std::shared_ptr<CZombieZSkill_Base>> m_vecSkill;
	std::set<byte> m_setSkillID;

private:
	bool m_bLadderDisabled;
	std::array<time_point_t, MAX_CLIENTS + 1> m_vecFireList;
	time_point_t m_flFireDmg;
	bool m_bIsFiring;
};

}

#endif //PROJECT_MOD_ZBZ_H
