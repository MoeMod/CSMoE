#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//galil
#define GALIL_MAX_SPEED			240
#define GALIL_DAMAGE			30
#define GALIL_RANGE_MODIFER		0.98
#define GALIL_RELOAD_TIME		2.45s
#define GALIL_INSPECT_TIME		3.67s
class CGalil : public CBasePlayerWeapon
{
	enum galil_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return GALIL_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override {}
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return GALIL_INSPECT_TIME; }
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
	void GalilFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
public:
	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireGalil;
};

}
