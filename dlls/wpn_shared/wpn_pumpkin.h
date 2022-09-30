#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CPumpkin : public CBasePlayerWeapon
{
	enum
    {
        HEGRENADE_IDLE,
        HEGRENADE_PULLPIN,
        HEGRENADE_THROW,
        HEGRENADE_DRAW
    };
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL CanDeploy();
	virtual BOOL CanDrop() { return FALSE; }
	virtual BOOL CanHolster();
	virtual BOOL Deploy();
	virtual void Holster(int skiplocal);
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return GRENADE_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	const char* GetCSModelName() override { return "models/w_pumpkin.mdl"; }
	bool ShieldSecondaryFire(int iUpAnim, int iDownAnim);
	void SetPlayerShieldAnim();
	void ResetPlayerShieldAnim();
	float GetDamage();
	float GetRange();
};


class CPumpkinGrenade : public CGrenade
{
public:
	void Spawn() override;
	void Precache() override;

public:
#ifndef CLIENT_DLL
	static CPumpkinGrenade* ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage);
#else
	static CPumpkinGrenade* ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage) { return NULL; }
#endif

public:
	void Explode(TraceResult* pTrace, int bitsDamageType);
	void EXPORT Detonate();
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT TumbleThink();
	void EXPORT Smoke();

private:
	duration_t GetExplosionTime() const { return 1.5s; }
	float GetRange() const { return 350.0f; }
};

}
