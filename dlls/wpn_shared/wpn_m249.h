#ifndef WPN_M249_H
#define WPN_M249_H
#ifdef _WIN32
#pragma once
#endif

//m249
#define M249_MAX_SPEED			220
#define M249_DAMAGE			32
#define M249_RANGE_MODIFER		0.97
#define M249_RELOAD_TIME		4.7

class CM249 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return M249_MAX_SPEED; }
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
	void M249Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireM249;
};

#endif