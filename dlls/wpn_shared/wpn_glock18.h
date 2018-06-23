#ifndef WPN_GLOCK18_H
#define WPN_GLOCK18_H
#ifdef _WIN32
#pragma once
#endif

//glock18
#define GLOCK18_MAX_SPEED		250
#define GLOCK18_DAMAGE			25
#define GLOCK18_RANGE_MODIFER		0.75
#define GLOCK18_RELOAD_TIME		2.2

class CGLOCK18 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return m_fMaxSpeed; }
	virtual int iItemSlot() { return PISTOL_SLOT; }
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
	virtual BOOL IsPistol() { return TRUE; }

public:
	void GLOCK18Fire(float flSpread, float flCycleTime, BOOL bFireBurst);

public:
	int m_iShell;
	bool m_bBurstFire;
};

#endif