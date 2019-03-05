#ifndef WPN_M95_H
#define WPN_M95_H
#ifdef _WIN32
#pragma once
#endif

//M95
#define M95_MAX_SPEED		200
#define M95_MAX_SPEED_ZOOM	140
#define M95_DAMAGE		115
#define M95_RANGE_MODIFER	0.985
#define M95_RELOAD_TIME		3.38
#define M95_ARMOR_RATIO_MODIFIER	1.9

class CM95 : public CBasePlayerWeapon
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
	KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }
	float GetArmorRatioModifier() override { return M95_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_m95.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

public:
	void M95Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireM95;
};

#endif