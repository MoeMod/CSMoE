#ifndef PLAYER_MOD_STRATEGY_H
#define PLAYER_MOD_STRATEGY_H
#ifdef _WIN32
#pragma once
#endif

class CBasePlayer;

// Strategy Pattern
// impliments part of player to deduce unnecessary references...
struct BasePlayerExtra
{
public:
	BasePlayerExtra(CBasePlayer *p) : m_pPlayer(p) {}
	
public:
	CBasePlayer * const m_pPlayer;
};

class IBasePlayerModStrategy : public BasePlayerExtra
{
public:
	IBasePlayerModStrategy(CBasePlayer *p) : BasePlayerExtra(p) {}
	virtual ~IBasePlayerModStrategy() = 0 {};

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

};

class CPlayerModStrategy_Default : public IBasePlayerModStrategy
{
public:
	CPlayerModStrategy_Default(CBasePlayer *p) : IBasePlayerModStrategy(p) {}

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
};

#endif