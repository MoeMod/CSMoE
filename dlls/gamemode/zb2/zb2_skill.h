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
#include "wpn_shared/wpn_zombiebomb.h"
#include "effects.h"
#include "customentity.h"

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
	virtual int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) = 0;
	
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
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override { return 1; }

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
	int AddToFullPack_Post(struct entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, int player, unsigned char* pSet) override;

protected:
	void OnHideEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
	EngineClock::time_point m_flInvisiable;
};

class CZombieSkill_ZombieCrazy2 : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieCrazy2(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazy2Effect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
	EngineClock::time_point m_flInvisiable;
};

class CZombieSkill_ZombieHeal : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieHeal(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
};

class CZombieSkill_ZombieDeimos : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieDeimos(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
};

class CZombieSkill_ZombiePC : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombiePC(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeNextSmoke;
	EngineClock::time_point m_flTimeTriggerZombieSkill;
	int m_iSmokeEvent;
};


class CZombieSkill_ZombiePass : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombiePass(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
	bool HasZombieBomb();

	int m_bHasCreateBomb;
	ZombieSkillStatus m_iZombieSkillStatusStored;
	EngineClock::time_point m_flTimeTriggerZombieSkill;
};

class CZombieSkill_ZombieTeleport : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieTeleport(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	void FireZombieSkill();
	bool Is_Hull_Vacant(Vector Origin, int Hull);
	void CheckStuck();
	Vector CalcPositon();

	Vector vecSize[70] =
	{
	{0.0, 0.0, 1.0}, {0.0, 0.0, -1.0}, {0.0, 1.0, 0.0}, {0.0, -1.0, 0.0}, {1.0, 0.0, 0.0}, {-1.0, 0.0, 0.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}, {1.0, -1.0, 1.0}, {1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0}, {1.0, -1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, -1.0},
	{0.0, 0.0, 2.0}, {0.0, 0.0, -2.0}, {0.0, 2.0, 0.0}, {0.0, -2.0, 0.0}, {2.0, 0.0, 0.0}, {-2.0, 0.0, 0.0}, {-2.0, 2.0, 2.0}, {2.0, 2.0, 2.0}, {2.0, -2.0, 2.0}, {2.0, 2.0, -2.0}, {-2.0, -2.0, 2.0}, {2.0, -2.0, -2.0}, {-2.0, 2.0, -2.0}, {-2.0, -2.0, -2.0},
	{0.0, 0.0, 3.0}, {0.0, 0.0, -3.0}, {0.0, 3.0, 0.0}, {0.0, -3.0, 0.0}, {3.0, 0.0, 0.0}, {-3.0, 0.0, 0.0}, {-3.0, 3.0, 3.0}, {3.0, 3.0, 3.0}, {3.0, -3.0, 3.0}, {3.0, 3.0, -3.0}, {-3.0, -3.0, 3.0}, {3.0, -3.0, -3.0}, {-3.0, 3.0, -3.0}, {-3.0, -3.0, -3.0},
	{0.0, 0.0, 4.0}, {0.0, 0.0, -4.0}, {0.0, 4.0, 0.0}, {0.0, -4.0, 0.0}, {4.0, 0.0, 0.0}, {-4.0, 0.0, 0.0}, {-4.0, 4.0, 4.0}, {4.0, 4.0, 4.0}, {4.0, -4.0, 4.0}, {4.0, 4.0, -4.0}, {-4.0, -4.0, 4.0}, {4.0, -4.0, -4.0}, {-4.0, 4.0, -4.0}, {-4.0, -4.0, -4.0},
	{0.0, 0.0, 5.0}, {0.0, 0.0, -5.0}, {0.0, 5.0, 0.0}, {0.0, -5.0, 0.0}, {5.0, 0.0, 0.0}, {-5.0, 0.0, 0.0}, {-5.0, 5.0, 5.0}, {5.0, 5.0, 5.0}, {5.0, -5.0, 5.0}, {5.0, 5.0, -5.0}, {-5.0, -5.0, 5.0}, {5.0, -5.0, -5.0}, {-5.0, 5.0, -5.0}, {-5.0, -5.0, -5.0}
	};

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	int m_bHasFireTeleportEntity = 0;
	EngineClock::time_point m_flTimeBotUseSkill;
	EngineClock::time_point m_flTimeTriggerZombieSkill;
	int m_iTeleportZombie;
	int m_iTeleportZombieOut;
	
};

class CZombieSkill_ZombieBooster : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieBooster(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	void Reset() override 
	{
		m_flBoostCharge = 0.0;
		m_iZombieSkillStatus = SKILL_STATUS_USED; 
		m_flPercent = 0.0;
		m_iTimeAutoCharge = 0;
	}

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	Vector m_vecVelocity;
	float m_flBoostCharge = 0.0;
	float m_flPercent = 0.0;
	const float m_flMaxpower = 800.0;
	int m_iTimeAutoCharge;

	EngineClock::time_point m_flTimeBotUseSkill;
	ZombieSkillStatus m_iZombieSkillStatusStored;
	EngineClock::time_point m_flTimeNextBoost;
	EngineClock::time_point m_flTimeChargeCool;
};


class CZombieSkill_ZombieStiffen : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieStiffen(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazyEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;
};

enum ZombieFlyingSkillStatus
{
	SKILL_END,
	FLY_START,		//0.4s
	FLY_START_LOOP,	
	FLY_START_END,	//0.43s
	FLY_IDLE,		//0.833s
	DOWNHILL_START,	//0.333s
	DOWNHILL_LOOP,
	DOWNHILL_END,	//1s
	FLY_BOMB_IDLE,	//0.86s
	FLY_BOMB_ATTACK,//0.86s
	FLY_ATTACK,		//0.86s
};

class CZombieSkill_ZombieFlying : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieFlying(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazyEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	void Reset() override
	{
		m_bIsflying = 0;
		m_iFlyingState = SKILL_END;
		m_flPlayerAnimTime = invalid_time_point;
	}

	int m_bIsflying = 0;
	ZombieFlyingSkillStatus m_iFlyingState;
	EngineClock::time_point m_flTimeBotUseSkill = invalid_time_point;
	EngineClock::time_point m_flTimeStartFlying = invalid_time_point;
	EngineClock::time_point m_flPlayerAnimTime = invalid_time_point;
	EngineClock::time_point m_flTimeZombieSkillEffect;
};


class CZombieSkill_ZombieRevival : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieRevival(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;
	void ResetMaxSpeed()  override;

protected:
	duration_t GetDurationTime() const;
};

class CZombieSkill_ZombieTrap : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieTrap(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
};

class CZombiePile;
class CZombieSkill_ZombiePile : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombiePile(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;
	void FireZombieSkill();
	EntityHandle <CZombiePile> m_pPile;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
	EngineClock::time_point m_flTimeChangeFov;
	EngineClock::time_point m_flTimeBotUseSkill;
};

class CZombieSkill_ZombiePenetration : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombiePenetration(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
	void FireBullets(ULONG cShots, Vector vecSrc, Vector vecDirShooting, Vector vecSpread, float flDistance, float flDamage, entvars_t* pevAttacker);
};


class CZombieSkill_ZombieJumpUp : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieJumpUp(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void ResetMaxSpeed() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
};


class CZombieSkill_ZombieHook : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieHook(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;
	void ResetMaxSpeed()  override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
};


class CZombieSkill_ZombieJumpUpM : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieJumpUpM(CBasePlayer* player);

public:
	void Activate() override;
	void OnSkillEnd() override;
	void ResetMaxSpeed() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
};

class CZombieSkill_ZombieArmorRecovery : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieArmorRecovery(CBasePlayer* player);

public:
	void Activate() override;
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
};


class CZombieSkill_ZombieStrengthRecovery : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieStrengthRecovery(CBasePlayer* player);

public:
	void Activate() override;
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
};

enum ZombieDashSkillStatus
{
	DASH_START,
	DASH_LOOP,
	DASH_END,
};

class CZombieSkill_ZombieDash : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieDash(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void ResetMaxSpeed() override;
	void FireZombieSkill();
	void DashAttack();
	bool AngleCheck(CBaseEntity* pEntity, CBaseEntity* pAttacker, Vector vecSrc, Vector vecDir, Vector2D vecAngleCheckDir, float flRange, float flMinimumCosine, TraceResult* ptr);

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	void Reset() override
	{
		m_SkillStatus = DASH_END;
	}

	ZombieDashSkillStatus m_SkillStatus = DASH_END;
	EngineClock::time_point m_flTimeTriggerZombieSkill;
};

enum ZombieDefenseSkillStatus
{
	SKILL_IDLE,
	DEFENSE_START,
	DEFENSE_LOOP,
	DEFENSE_ENDTOSHOT,
	DEFENSE_ENDTOIDLE,
};

class CZombieSkill_ZombieDefense : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieDefense(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	float GetDamageRatio() const override;
	void ResetMaxSpeed() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
	float GetChargeDamage() const;

	void Reset() override
	{
		m_flPlayerAnimTime = invalid_time_point;
	}

	EngineClock::time_point m_flTimeBotUseSkill = invalid_time_point;
	EngineClock::time_point m_flTimeTriggerZombieSkill = invalid_time_point;
	EngineClock::time_point m_flPlayerAnimTime = invalid_time_point;
	ZombieDefenseSkillStatus m_SkillStatus = SKILL_IDLE;
};


class CZombieSkill_ZombieSelfDestruct : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieSelfDestruct(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void ResetMaxSpeed()  override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	duration_t GetDurationTime() const;
};

class CZombieSkill_ZombieCreateRocket : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieCreateRocket(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void FireZombieSkill();
	void OnSkillEnd() override;

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
	EngineClock::time_point m_flTimeTriggerZombieSkill = invalid_time_point;
};

class CZombieSkill_ZombieWebShooter : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieWebShooter(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	Vector GetShootPosition(int* bHit = 0);
	void FireZombieSkill();
	void Reset() override
	{
		bHit = 0;
		m_flPlayerAnimTime = invalid_time_point;
		m_flFlyingTime = invalid_time_point;
	}

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flPlayerAnimTime = invalid_time_point;
	EngineClock::time_point m_flFlyingTime = invalid_time_point;
	EngineClock::time_point m_flTimeBotUseSkill = invalid_time_point;
	EngineClock::time_point m_flTimeTriggerZombieSkill = invalid_time_point;
	EntityHandle <CBeam> m_pBeam;
	Vector m_vecHookOrigin, m_vecPlayerOrigin;
	int bHit = 0;
};


class CZombieSkill_ZombieWebBomb : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieWebBomb(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
};

class CZombieSkill_ZombieProtect : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieProtect(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	float GetDamageRatio() const override;

protected:
	void OnCrazyEffect();
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeZombieSkillEffect;

	int m_iDeathKnightEffectShield;
};


class CZombieSkill_ZombieChargeSlash : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieChargeSlash(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void ResetMaxSpeed() override;
	void FireZombieSkill();
	Vector GetShootPosition();
	void DashAttack();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	void Reset() override
	{
		m_SkillStatus = DASH_END;
	}

	Vector m_vecPosition;
	ZombieDashSkillStatus m_SkillStatus = DASH_END;
	EngineClock::time_point m_flTimeTriggerZombieSkill;

	int m_iDeathKnightEffectFloor;
	int m_iDeathKnightEffectFlash;
	int m_iDeathKnightEffectFlashArea;
};

class CZombieSkill_ZombieDogShoot : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieDogShoot(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	void OnSkillEnd() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;

	EngineClock::time_point m_flTimeTriggerZombieSkill;
};

enum ZombieRushSkillStatus
{
	RUSH_IDLE,
	RUSH_START,
	RUSH_LOOP,
	RUSH_HOLD,
	RUSH_FINISH,
};

class CZombieSkill_ZombieRush : public CZombieSkill_Base
{
public:
	explicit CZombieSkill_ZombieRush(CBasePlayer* player);

public:
	void Think() override;
	void Activate() override;
	float GetDamageRatio() const override;
	void ResetMaxSpeed() override;
	void FireZombieSkill();

protected:
	duration_t GetDurationTime() const;
	duration_t GetCooldownTime() const;
	float GetChargeDamage() const;

	void Reset() override
	{
		m_flPlayerAnimTime = invalid_time_point;
	}

	Vector m_vecLockAngle;
	EngineClock::time_point m_flTimeBotUseSkill = invalid_time_point;
	EngineClock::time_point m_flTimeTriggerZombieSkill = invalid_time_point;
	EngineClock::time_point m_flPlayerAnimTime = invalid_time_point;
	ZombieRushSkillStatus m_SkillStatus = RUSH_IDLE;
};


}


#endif