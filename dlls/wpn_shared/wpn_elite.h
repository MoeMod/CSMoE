#ifndef WPN_ELITE_H
#define WPN_ELITE_H
#ifdef _WIN32
#pragma once
#endif

//Elites
#define ELITE_MAX_SPEED		250
#define ELITE_RELOAD_TIME	4.5
#define ELITE_DAMAGE		36
#define ELITE_RANGE_MODIFER	0.75

class CELITE : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return ELITE_MAX_SPEED; }
	virtual int iItemSlot() { return PISTOL_SLOT; }
	virtual void PrimaryAttack();
	virtual void Reload();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual BOOL IsPistol() { return TRUE; }

public:
	void ELITEFire(float flSpread, float flCycleTime, BOOL fUseSemi);

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

#endif