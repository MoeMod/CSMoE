#ifndef WPN_AK47_H
#define WPN_AK47_H
#ifdef _WIN32
#pragma once
#endif

//AK47
#define AK47_MAX_SPEED			221
#define AK47_DAMAGE			36
#define AK47_RANGE_MODIFER		0.98
#define AK47_RELOAD_TIME		2.45

class CAK47 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return AK47_MAX_SPEED; }
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
	void AK47Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireAK47;
};

#endif