#ifndef WPN_AUG_H
#define WPN_AUG_H
#ifdef _WIN32
#pragma once
#endif

//AUG
#define AUG_MAX_SPEED			240
#define AUG_DAMAGE			32
#define AUG_RANGE_MODIFER		0.96
#define AUG_RELOAD_TIME			3.3

class CAUG : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return AUG_MAX_SPEED; }
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
	virtual KnockbackData GetKnockBackData() override { return { 350.0f,250.0f,300.0f,100.0f,0.6f }; }

public:
	void AUGFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireAug;
};

#endif