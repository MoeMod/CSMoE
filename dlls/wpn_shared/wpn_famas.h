#ifndef WPN_FAMAS_H
#define WPN_FAMAS_H
#ifdef _WIN32
#pragma once
#endif

//Famas
#define FAMAS_MAX_SPEED		240
#define FAMAS_RELOAD_TIME	3.3
#define FAMAS_DAMAGE		30
#define FAMAS_DAMAGE_BURST	34
#define FAMAS_RANGE_MODIFER	0.96

class CFamas : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return FAMAS_MAX_SPEED; }
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
	void FamasFire(float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL bFireBurst);

public:
	int m_iShell;
	int iShellOn;
};

#endif