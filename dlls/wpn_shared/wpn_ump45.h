#ifndef WPN_UMP45_H
#define WPN_UMP45_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//ump45
#define UMP45_MAX_SPEED			250
#define UMP45_DAMAGE			30
#define UMP45_RANGE_MODIFER		0.82
#define UMP45_RELOAD_TIME		3.5s
#define UMP45_INSPECT_TIME		4.48s

class CUMP45 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return UMP45_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return UMP45_INSPECT_TIME; }
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
	void UMP45Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireUMP45;
};

}

#endif