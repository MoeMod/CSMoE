#ifndef WPN_TMP_H
#define WPN_TMP_H
#ifdef _WIN32
#pragma once
#endif

//tmp
#define TMP_MAX_SPEED			250
#define TMP_DAMAGE			20
#define TMP_RANGE_MODIFER		0.85
#define TMP_RELOAD_TIME			2.12

class CTMP : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return TMP_MAX_SPEED; }
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
	void TMPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireTMP;
};

#endif