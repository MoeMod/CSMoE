#ifndef WPN_P228_H
#define WPN_P228_H
#ifdef _WIN32
#pragma once
#endif

//p228
#define P228_MAX_SPEED		250
#define P228_DAMAGE		32
#define P228_RANGE_MODIFER	0.8
#define P228_RELOAD_TIME	2.7

class CP228 : public CBasePlayerWeapon
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
	void P228Fire(float flSpread, float flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

public:
	int m_iShell;

private:
	unsigned short m_usFireP228;
};

#endif