#ifndef WPN_SCOUT_H
#define WPN_SCOUT_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//scout
#define SCOUT_MAX_SPEED			260
#define SCOUT_MAX_SPEED_ZOOM		220
#define SCOUT_DAMAGE			75
#define SCOUT_RANGE_MODIFER		0.98
#define SCOUT_RELOAD_TIME		2s
#define SCOUT_INSPECT_TIME		4.48s

class CSCOUT : public CBasePlayerWeapon
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
	void Inspect() override;
	duration_t GetInspectTime() override { return SCOUT_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 3000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }

public:
	void SCOUTFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	int m_iShell;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireScout;
};

}

#endif