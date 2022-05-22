#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define M1887SE_DAMAGE			75
#define M1887SE_RANGE_MODIFER		0.98
#define M1887SE_MAX_CLIP			3

class CM1887se : public CBasePlayerWeapon
{
	enum scout_e
    {
		M1887_IDLE,
		M1887_FIRE1,
		M1887_FIRE2,
		M1887_RELOAD,
		M1887_PUMP,
		M1887_START_RELOAD,
		M1887_DRAW,
    };
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
	KnockbackData GetKnockBackData() override { return { 1000.0f, 250.0f, 600.0f, 700.0f, 0.9f };}
	const char* GetCSModelName() override { return "models/pw_m1887se.mdl"; }
	float GetArmorRatioModifier() override { return 1.7; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_SCOUT); }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { 0, 5, 3, 170 }; }
public:
	void M1887seFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage();
	int m_iShell;
	duration_t m_flPumpTime;

private:
	unsigned short m_usFireM1887se;
};

}
