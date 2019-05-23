#ifndef WPN_AS50_H
#define WPN_AS50_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//as50
#define AS50_MAX_SPEED		205
#define AS50_MAX_SPEED_ZOOM	160
#define AS50_DAMAGE		115
#define AS50_RANGE_MODIFER	0.98
#define AS50_RELOAD_TIME	3.0
#define AS50_ARMOR_RATIO_MODIFIER	1.7
#define AS50_ZOOM_FOV_1	38
#define AS50_ZOOM_FOV_2	12

class CAS50 : public CBasePlayerWeapon
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
	KnockbackData GetKnockBackData() override { return { 2500, 600.0f, 800.0f, 800.0f, 0.4f }; }
	float GetArmorRatioModifier() override { return AS50_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_as50.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

public:
	void AS50Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireAS50;
};

}

#endif