#ifndef WPN_P228_H
#define WPN_P228_H
#ifdef _WIN32
#pragma once
#endif

//p228
#define P228_MAX_SPEED		250
#define P228_DAMAGE		32
#define P228_RANGE_MODIFER	0.8
#define P228_RELOAD_TIME	2.7

class CP228 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
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
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override { return { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f }; }

public:
	void P228Fire(float flSpread, float flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

public:
	int m_iShell;

private:
	unsigned short m_usFireP228;
};

#endif