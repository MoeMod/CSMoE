#ifndef WPN_SG552_H
#define WPN_SG552_H
#ifdef _WIN32
#pragma once
#endif

//sg552
#define SG552_MAX_SPEED			235
#define SG552_MAX_SPEED_ZOOM		200
#define SG552_DAMAGE			33
#define SG552_RANGE_MODIFER		0.955
#define SG552_RELOAD_TIME		3

class CSG552 : public CBasePlayerWeapon
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
	void SG552Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireSG552;
};

#endif