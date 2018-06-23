#ifndef WPN_XM1014_H
#define WPN_XM1014_H
#ifdef _WIN32
#pragma once
#endif

//xm1014
#define XM1014_MAX_SPEED	240
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads

class CXM1014 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return XM1014_MAX_SPEED; }
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
	int m_iShell;
	float m_flPumpTime;

private:
	unsigned short m_usFireXM1014;
};

#endif