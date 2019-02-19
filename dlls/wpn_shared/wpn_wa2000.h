#ifndef WPN_WA2000_H
#define WPN_WA2000_H
#ifdef _WIN32
#pragma once
#endif

//wa2000
#define WA2000_MAX_SPEED			210
#define WA2000_MAX_SPEED_ZOOM		150
#define WA2000_DAMAGE			70
#define WA2000_RANGE_MODIFER		0.98
#define WA2000_RELOAD_TIME		3.35

class CWA2000 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
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
	KnockbackData GetKnockBackData() override { return { 1000.0f, 480.0f, 900.0f, 900.0f, 0.3f }; }
	const char *GetCSModelName() override;
	float GetArmorRatioModifier() override { return 1.7; }

public:
	void WA2000Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;

private:
	unsigned short m_usFireWA2000;
};

#endif