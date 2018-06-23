#ifndef WPN_FIVESEVEN_H
#define WPN_FIVESEVEN_H
#ifdef _WIN32
#pragma once
#endif

//Fiveseven
#define FIVESEVEN_MAX_SPEED		250
#define FIVESEVEN_DAMAGE		20
#define FIVESEVEN_RANGE_MODIFER		0.885
#define FIVESEVEN_RELOAD_TIME		2.7

class CFiveSeven : public CBasePlayerWeapon
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
	void FiveSevenFire(float flSpread, float flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

public:
	int m_iShell;

private:
	unsigned short m_usFireFiveSeven;
};

#endif