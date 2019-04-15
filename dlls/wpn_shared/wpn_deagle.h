#ifndef WPN_DEAGLE_H
#define WPN_DEAGLE_H
#ifdef _WIN32
#pragma once
#endif

//Deagle
#define DEAGLE_MAX_SPEED	250
#define DEAGLE_DAMAGE		54
#define DEAGLE_RANGE_MODIFER	0.81
#define DEAGLE_RELOAD_TIME	2.2

class CDEAGLE : public CBasePlayerWeapon
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
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 350.0f, 100.0f, 0.6f }; }

public:
	void DEAGLEFire(float flSpread, float flCycleTime, BOOL fUseSemi);

	int m_iShell;

private:
	unsigned short m_usFireDeagle;
};

#endif