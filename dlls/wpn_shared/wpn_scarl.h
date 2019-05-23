#ifndef WPN_GALIL_H
#define WPN_GALIL_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CScarLight : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 230; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override {}
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
	float GetArmorRatioModifier() override { return 1.4; }
	const char *GetCSModelName() override { return "models/w_scar.mdl"; }

#ifndef CLIENT_DLL
	virtual void ItemPostFrame() override;
	~CScarLight();
	virtual void AttachToPlayer(CBasePlayer *pPlayer) override;
#endif

public:
	void GalilFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);
	float GetDamage();

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireGalil;
};

}

#endif