#ifndef WPN_USP_H
#define WPN_USP_H
#ifdef _WIN32
#pragma once
#endif

//tmp
#define USP_MAX_SPEED		250
#define USP_DAMAGE		34
#define USP_DAMAGE_SIL		30
#define USP_RANGE_MODIFER	0.79
#define USP_RELOAD_TIME		2.7

class CUSP : public CBasePlayerWeapon
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
	void USPFire(float flSpread, float flCycleTime, BOOL fUseSemi);
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

	int m_iShell;

private:
	unsigned short m_usFireUSP;
};

#endif