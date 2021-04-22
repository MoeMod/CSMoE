#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//mac10
#define MAC10_MAX_SPEED			250
#define MAC10_DAMAGE			29
#define MAC10_RANGE_MODIFER		0.82
#define MAC10_RELOAD_TIME		3.15s
#define MAC10_INSPECT_TIME		5.78s

class CMAC10 : public CBasePlayerWeapon
{
	enum mac10_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MAC10_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return MAC10_INSPECT_TIME; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }

public:
	void MAC10Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireMAC10;
};

}
