#ifndef WPN_MAC10_H
#define WPN_MAC10_H
#ifdef _WIN32
#pragma once
#endif

//mac10
#define MAC10_MAX_SPEED			250
#define MAC10_DAMAGE			29
#define MAC10_RANGE_MODIFER		0.82
#define MAC10_RELOAD_TIME		3.15

class CMAC10 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return MAC10_MAX_SPEED; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
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

public:
	void MAC10Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireMAC10;
};

#endif