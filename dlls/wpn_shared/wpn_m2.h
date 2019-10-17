#ifndef WPN_M4A1_H
#define WPN_M4A1_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m4a1
#define M4A1_MAX_SPEED		180
#define	M2_DEFAULT_GIVE 250

class CM2 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL CanHolster() override;
	BOOL CanDrop() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.85f }; }
	const char *GetCSModelName() override { return "models/w_m2.mdl"; }
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_556natobox" , AMMO_556MM_PRICE }; }

public:
	void M2Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void SwitchThink(void);
	float GetDamageA();
	float GetDamageB();
	int m_iShell;
	int iShellOn;
	time_point_t fTime;

private:
	unsigned short m_usFireM2;
};

}

#endif