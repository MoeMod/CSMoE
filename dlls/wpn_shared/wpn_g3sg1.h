#ifndef WPN_G3SG1_H
#define WPN_G3SG1_H
#ifdef _WIN32
#pragma once
#endif

//g3sg1
#define G3SG1_MAX_SPEED		210
#define G3SG1_MAX_SPEED_ZOOM	150
#define G3SG1_DAMAGE		80
#define G3SG1_RANGE_MODIFER	0.98
#define G3SG1_RELOAD_TIME	3.5

class CG3SG1 : public CBasePlayerWeapon
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
	virtual KnockbackData GetKnockBackData() override { return { 400.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }

public:
	void G3SG1Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireG3SG1;
};

#endif