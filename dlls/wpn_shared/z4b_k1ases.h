#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//k1ases
#define K1ASES_MAX_SPEED			240
#define K1A_RANGE_MODIFER		0.98
#define K1A_RELOAD_TIME		3.0

class CK1ases : public CBasePlayerWeapon
{
	enum k1ases_e
    {
        K1A_IDLE1,
        K1A_RELOAD,
        K1A_DRAW,
        K1A_SHOOT1,
        K1A_SHOOT2,
        K1A_SHOOT3,
		K1A_SKILL
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return K1ASES_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	bool HasSecondaryAttack() override { return true; }
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void PrimaryAttack() override;
#ifndef CLIENT_DLL
	void SecondaryAttack() override;
	void DelaySecondaryAttack();
#endif
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.6f }; }
	const char *GetCSModelName() override { return "models/w_k1ases.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_GALIL); }
#endif
	float GetArmorRatioModifier() override { return 1.7; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 4, 3, 140 }; }
public:
	void K1asesFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
public:
	int m_iDefaultAmmo2;
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireK1ases;
};

}
