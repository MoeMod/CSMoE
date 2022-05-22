#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//bizon
#define BISON_MAX_SPEED		250
#define BISON_RANGE_MODIFER	0.8
#define BISON_RELOAD_TIME		2.43s
#define BISON_MAX_CLIP 64

class CBison : public CBasePlayerWeapon
{
	enum bizon_e
    {
        BIZON_IDLE1,
        BIZON_RELOAD,
        BIZON_DRAW,
        BIZON_SHOOT1,
        BIZON_SHOOT2,
        BIZON_SHOOT3,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return BISON_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	float GetDamage();
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }
	const char* GetCSModelName() override { return "models/w_bison.mdl"; }
public:
	void BisonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_P90); }
#endif
	float GetArmorRatioModifier() override { return 0.85; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR, 6, 2, 0 }; }
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireBison;
};

}
