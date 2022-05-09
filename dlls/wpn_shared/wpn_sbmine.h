#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSbmine : public CBasePlayerWeapon
{
	enum
    {
        SBMINE_IDLE,
		SBMINE_SHOOT_READY,
		SBMINE_READY_IDLE,
		SBMINE_SHOOT1,
		SBMINE_SHOOT2,
		SBMINE_DRAW,
		SBMINE_TRIGGER_IDLE,
		SBMINE_TRIGGER_SHOOT,
		SBMINE_TRIGGER_DRAW,
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
	float GetDamage();
	float GetRange();

	EntityHandle <CSbmineGrenade> m_pSBMine;
};


class CSbmineGrenade : public CGrenade
{
public:
	void Spawn() override;
	void Precache() override;

public:
#ifndef CLIENT_DLL
	static CSbmineGrenade* ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage);
#else
	static CSbmineGrenade* ShootGrenade(entvars_t* pevOwner, Vector vecStart, Vector vecVelocity, int iTeam, float flDamage) { return NULL; }
#endif

public:
	void Explode(TraceResult* pTrace, int bitsDamageType);
	void EXPORT Detonate();
	void EXPORT BounceTouch(CBaseEntity* pOther);
	void EXPORT TumbleThink();
	void EXPORT Smoke();
	CBasePlayer* m_pOwner;

private:
	duration_t GetExplosionTime() const { return 1.5s; }
	float GetRange() const { return 100.0f; }

private:
	short m_sModelIndexExp;
};

}
