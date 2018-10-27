#ifndef WPN_KRISS_H
#define WPN_KRISS_H
#ifdef _WIN32
#pragma once
#endif

//kriss
#define KRISS_MAX_SPEED		245
#define KRISS_DAMAGE		29
#define KRISS_DAMAGE_SIL		28
#define KRISS_RANGE_MODIFER      0.83
#define KRISS_RANGE_MODIFER_SIL  0.81
#define KRISS_RELOAD_TIME	3.5
#define KRISS_ARMOR_RATIO_MODIFIER	0.8
#define KRISS_FIRE_RATE	0.088028

class CKRISS : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return KRISS_MAX_SPEED; }
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
	virtual KnockbackData GetKnockBackData() override { return { 500.0f, 400.0f, 500.0f, 180.0f, 0.25f }; }
	float GetArmorRatioModifier() override { return KRISS_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_kriss.mdl"; }

public:
	void KRISSFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireKRISS;
};

#endif