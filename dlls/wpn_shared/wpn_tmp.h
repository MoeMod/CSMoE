#ifndef WPN_TMP_H
#define WPN_TMP_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//tmp
#define TMP_MAX_SPEED			250
#define TMP_DAMAGE			20
#define TMP_RANGE_MODIFER		0.85
#define TMP_RELOAD_TIME			2.12

class CTMP : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return TMP_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }

public:
	void TMPFire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireTMP;
};

}

#endif