#ifndef WPN_GLOCK18_H
#define WPN_GLOCK18_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//glock18
#define GLOCK18_MAX_SPEED		250
#define GLOCK18_DAMAGE			25
#define GLOCK18_RANGE_MODIFER		0.75
#define GLOCK18_RELOAD_TIME		2.2s
#define GLOCK18_INSPECT_TIME		4.48s

class CGLOCK18 : public CBasePlayerWeapon
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
	void Inspect() override;
	duration_t GetInspectTime() override { return GLOCK18_INSPECT_TIME; }
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
	void GLOCK18Fire(float flSpread, duration_t flCycleTime, BOOL bFireBurst);

public:
	int m_iShell;
	bool m_bBurstFire;
	time_point_t m_NextInspect;
};

}

#endif