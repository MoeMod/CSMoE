#ifndef WPN_TAR21_H
#define WPN_TAR21_H
#ifdef _WIN32
#pragma once
#endif

//TAR21
#define TAR21_MAX_SPEED			220
#define TAR21_MAX_SPEED_ZOOM			150
#define TAR21_DAMAGE			32
#define TAR21_RANGE_MODIFER		0.96
#define TAR21_RELOAD_TIME			3.0

class CTAR21 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override {
		if (m_pPlayer->m_iFOV == 90)
			return TAR21_MAX_SPEED;

		return TAR21_MAX_SPEED_ZOOM;
	}
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
	KnockbackData GetKnockBackData() override { return { 1200.0f,950.0f,1100.0f,950.0f,0.5f }; }
	float GetArmorRatioModifier() override { return 1.2; }
	const char *GetCSModelName() override { return "models/w_tar21.mdl"; }

public:
	void TAR21Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireTar21;
};

#endif