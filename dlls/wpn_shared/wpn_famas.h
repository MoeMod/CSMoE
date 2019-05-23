#ifndef WPN_FAMAS_H
#define WPN_FAMAS_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//Famas
#define FAMAS_MAX_SPEED		240
#define FAMAS_RELOAD_TIME	3.3
#define FAMAS_DAMAGE		30
#define FAMAS_DAMAGE_BURST	34
#define FAMAS_RANGE_MODIFER	0.96

class CFamas : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return FAMAS_MAX_SPEED; }
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
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }

public:
	void FamasFire(float flSpread, float flCycleTime, BOOL fUseAutoAim, BOOL bFireBurst);

public:
	int m_iShell;
	int iShellOn;
};

}

#endif