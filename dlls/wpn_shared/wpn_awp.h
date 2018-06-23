#ifndef WPN_AWP_H
#define WPN_AWP_H
#ifdef _WIN32
#pragma once
#endif

//AWP
#define AWP_MAX_SPEED		210
#define AWP_MAX_SPEED_ZOOM	150
#define AWP_DAMAGE		115
#define AWP_RANGE_MODIFER	0.99
#define AWP_RELOAD_TIME		2.5

class CAWP : public CBasePlayerWeapon
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
	void AWPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireAWP;
};

#endif