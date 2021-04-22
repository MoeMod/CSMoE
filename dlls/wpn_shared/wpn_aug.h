#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//AUG
#define SPECIALSCOPE		0
#define AUG_MAX_SPEED			240
#define AUG_DAMAGE			32
#define AUG_RANGE_MODIFER		0.96
#define AUG_RELOAD_TIME			3.3s
#define AUG_INSPECT_TIME			3.47s

class CAUG : public CBasePlayerWeapon
{
	enum aug_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return AUG_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void Inspect() override;
	duration_t GetInspectTime() override { return AUG_INSPECT_TIME; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
#if SPECIALSCOPE
	void SpecialScope();
#endif
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f,250.0f,300.0f,100.0f,0.6f }; }

public:
	void AUGFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireAug;
};

}
