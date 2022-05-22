#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSFMG : public CBasePlayerWeapon
{
	enum sfmg_e
    {
		SFMG_IDLEA,
		SFMG_RELOADA,
		SFMG_DRAWA,
		SFMG_SHOOTA1,
		SFMG_SHOOTA2,
		SFMG_SHOOTA3,
		SFMG_CHANGEA,	
		SFMG_IDLEB,
		SFMG_RELOADB,
		SFMG_DRAWB,
		SFMG_SHOOTB1,
		SFMG_SHOOTB2,
		SFMG_SHOOTB3,
		SFMG_CHANGEB,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
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
	KnockbackData GetKnockBackData() override { return { 400.0f, 250.0f, 330.0f, 200.0f, 0.6f }; }
	const char *GetCSModelName() override { return "models/w_sfmg.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_762natobox" , 60 }; }
#endif

	float GetArmorRatioModifier() override { return 1.55; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }

public:
	void SFMGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamageA() const;
	float GetDamageB() const;
	int m_iShell;
	float m_flAccuracy2;

private:
	unsigned short m_usFireSFMG;
};

}
