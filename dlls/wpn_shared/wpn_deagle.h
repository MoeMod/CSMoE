#ifndef WPN_DEAGLE_H
#define WPN_DEAGLE_H
#ifdef _WIN32
#pragma once
#endif

//Deagle
#define DEAGLE_MAX_SPEED	250
#define DEAGLE_DAMAGE		54
#define DEAGLE_RANGE_MODIFER	0.81
#define DEAGLE_RELOAD_TIME	2.2

class CDEAGLE : public CBasePlayerWeapon
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
	void DEAGLEFire(float flSpread, float flCycleTime, BOOL fUseSemi);

	int m_iShell;

private:
	unsigned short m_usFireDeagle;
};

#endif