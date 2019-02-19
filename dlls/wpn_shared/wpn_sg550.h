#ifndef WPN_SG550_H
#define WPN_SG550_H
#ifdef _WIN32
#pragma once
#endif

//sg550
#define SG550_MAX_SPEED			210
#define SG550_MAX_SPEED_ZOOM		150
#define SG550_DAMAGE			70
#define SG550_RANGE_MODIFER		0.98
#define SG550_RELOAD_TIME		3.35

class CSG550 : public CBasePlayerWeapon
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
	KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }

public:
	void SG550Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;

private:
	unsigned short m_usFireSG550;
};

#endif