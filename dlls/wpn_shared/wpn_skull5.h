#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//SKULL5
#define SKULL5_MAX_CLIP			24
#define SKULL5_MAX_SPEED		205
#define SKULL5_MAX_SPEED_ZOOM	150
#define SKULL5_RANGE_MODIFER	0.98
#define SKULL5_RELOAD_TIME	1.9s

class CSkull5 : public CBasePlayerWeapon
{
	enum skull5_e
    {
        SKULL5_IDLE,
		SKULL5_RELOAD,
		SKULL5_DRAW,
        SKULL5_SHOOT,
        SKULL5_SHOOT2,
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
	KnockbackData GetKnockBackData() override { return { 1500.0f, 500.0f, 900.0f, 600.0f, 0.3f }; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 4, 3, 0 }; }
	const char* GetCSModelName() override { return "models/w_skull5.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556az" , 200 }; }
#endif

public:
	void Skull5Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;

private:
	unsigned short m_usFireSkull5;
};

}
