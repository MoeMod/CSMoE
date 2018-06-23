#ifndef WPN_GALIL_H
#define WPN_GALIL_H
#ifdef _WIN32
#pragma once
#endif

//galil
#define GALIL_MAX_SPEED			240
#define GALIL_DAMAGE			30
#define GALIL_RANGE_MODIFER		0.98
#define GALIL_RELOAD_TIME		2.45

class CGalil : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return GALIL_MAX_SPEED; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack() {}
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
	void GalilFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireGalil;
};

#endif