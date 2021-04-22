#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//tmp
#define TMP_MAX_SPEED			250
#define TMP_DAMAGE			20
#define TMP_RANGE_MODIFER		0.85
#define TMP_RELOAD_TIME			2.12s
#define TMP_INSPECT_TIME			5.84s

class CTMP : public CBasePlayerWeapon
{
	enum tmp_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return TMP_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return TMP_INSPECT_TIME; }
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
	void TMPFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireTMP;
};

}
