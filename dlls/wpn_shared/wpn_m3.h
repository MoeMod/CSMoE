#ifndef WPN_M3_H
#define WPN_M3_H
#ifdef _WIN32
#pragma once
#endif

//m3
#define M3_MAX_SPEED		230
#define M3_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads

class CM3 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return M3_MAX_SPEED; }
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
	unsigned short m_usFireM3;
};

#endif