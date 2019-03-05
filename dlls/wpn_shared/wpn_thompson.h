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
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return THOMPSON_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }
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