#ifndef WPN_THOMPSON_H
#define WPN_THOMPSON_H
#ifdef _WIN32
#pragma once
#endif

// thompson
#define THOMPSON_MAX_SPEED		240
#define THOMPSON_DAMAGE		30
#define THOMPSON_RANGE_MODIFER      0.82
#define THOMPSON_RELOAD_TIME	3.7
#define THOMPSON_ARMOR_RATIO_MODIFIER	1.0
#define THOMPSON_FIRE_RATE	0.1

class CTHOMPSON : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return THOMPSON_MAX_SPEED; }
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

	virtual KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }
	float GetArmorRatioModifier() override { return THOMPSON_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_thompson.mdl"; }

public:
	void THOMPSONFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireTHOMPSON;
};

#endif