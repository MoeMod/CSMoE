#ifndef WPN_P90_H
#define WPN_P90_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//p90
#define P90_MAX_SPEED		245
#define P90_DAMAGE		21
#define P90_RANGE_MODIFER	0.885
#define P90_RELOAD_TIME		3.4s
#define P90_INSPECT_TIME		4.24s

class CP90 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return P90_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return P90_INSPECT_TIME; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }

public:
	void P90Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireP90;
};

}

#endif