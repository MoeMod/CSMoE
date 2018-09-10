#ifndef WPN_SG550_H
#define WPN_SG550_H
#ifdef _WIN32
#pragma once
#endif

//sg550
#define SG550_MAX_SPEED			210
#define SG550_MAX_SPEED_ZOOM		150
#define SG550_DAMAGE			70
#define SG550_RANGE_MODIFER		0.98
#define SG550_RELOAD_TIME		3.35

class CSG550 : public CBasePlayerWeapon
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
	virtual KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }

public:
	void SG550Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;

private:
	unsigned short m_usFireSG550;
};

#endif