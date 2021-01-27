#ifndef WPN_SG552_H
#define WPN_SG552_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//sg552
#define SPECIALSCOPE		0
#define SG552_MAX_SPEED			235
#define SG552_MAX_SPEED_ZOOM		200
#define SG552_DAMAGE			33
#define SG552_RANGE_MODIFER		0.955
#define SG552_RELOAD_TIME		3s
#define SG552_INSPECT_TIME		4.48s

class CSG552 : public CBasePlayerWeapon
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
#if SPECIALSCOPE
	void SpecialScope();
#endif
	void Inspect() override;
	duration_t GetInspectTime() override { return SG552_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }

public:
	void SG552Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireSG552;
};

}

#endif