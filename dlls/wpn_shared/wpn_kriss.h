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
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return KRISS_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 500.0f, 400.0f, 500.0f, 180.0f, 0.25f }; }
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