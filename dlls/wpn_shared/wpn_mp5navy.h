#ifndef WPN_MP5NAVY_H
#define WPN_MP5NAVY_H
#ifdef _WIN32
#pragma once
#endif

//mp5navy
#define MP5N_MAX_SPEED			250
#define MP5N_DAMAGE			26
#define MP5N_RANGE_MODIFER		0.84
#define MP5N_RELOAD_TIME		2.63

class CMP5N : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return MP5N_MAX_SPEED; }
	int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
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
	virtual KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }

public:
	void MP5NFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireMP5N;
};

#endif