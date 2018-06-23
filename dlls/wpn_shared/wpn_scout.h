#ifndef WPN_SCOUT_H
#define WPN_SCOUT_H
#ifdef _WIN32
#pragma once
#endif

//scout
#define SCOUT_MAX_SPEED			260
#define SCOUT_MAX_SPEED_ZOOM		220
#define SCOUT_DAMAGE			75
#define SCOUT_RANGE_MODIFER		0.98
#define SCOUT_RELOAD_TIME		2

class CSCOUT : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed();
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
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

public:
	void SCOUTFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	int m_iShell;

private:
	unsigned short m_usFireScout;
};

#endif