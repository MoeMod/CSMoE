#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//g3sg1
#define G3SG1_MAX_SPEED		210
#define G3SG1_MAX_SPEED_ZOOM	150
#define G3SG1_DAMAGE		80
#define G3SG1_RANGE_MODIFER	0.98
#define G3SG1_RELOAD_TIME	3.5s
#define G3SG1_INSPECT_TIME		3.69s

class CG3SG1 : public CBasePlayerWeapon
{
	enum g3sg1_e;
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
	duration_t GetInspectTime() override { return G3SG1_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 400.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }

public:
	void G3SG1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireG3SG1;
};

}
