#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//Fiveseven
#define FIVESEVEN_MAX_SPEED		250
#define FIVESEVEN_DAMAGE		20
#define FIVESEVEN_RANGE_MODIFER		0.885
#define FIVESEVEN_RELOAD_TIME		2.7s
#define FIVESEVEN_INSPECT_TIME		5.20s

class CFiveSeven : public CBasePlayerWeapon
{
	enum fiveseven_e;
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
	void Inspect() override;
	duration_t GetInspectTime() override { return FIVESEVEN_INSPECT_TIME; }
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
	void FiveSevenFire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	float GetDamage() const;
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

public:
	int m_iShell;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireFiveSeven;
};

}
