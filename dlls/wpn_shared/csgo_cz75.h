#ifndef WPN_FIVESEVEN_H
#define WPN_FIVESEVEN_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//cz75
#define CZ75_MAX_SPEED		250
#define CZ75_RANGE_MODIFER		0.885
#define CZ75_RELOAD_TIME_1		2.26s
#define CZ75_RELOAD_TIME_2	2.82s
#define CZ75_INSPECT_TIME		6.2s
#define	CZ75_DEFAULT_GIVE		12
#define	CZ75_MAX_CLIP		12

class CCz75 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return CZ75_INSPECT_TIME; }
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
	const char* GetCSModelName() override { return "models/w_cz75.mdl"; }
public:
	void Cz75Fire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	float GetDamage();
	NOXREF void MakeBeam();
	NOXREF void BeamUpdate();

public:
	int m_iShell;
	time_point_t m_NextInspect;
	bool m_bHasReload;

private:
	unsigned short m_usFireCz75;
};

}

#endif