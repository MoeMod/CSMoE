#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//mg3
#define MG3_MAX_SPEED			180
#define MG3_DAMAGE			35
#define MG3_RANGE_MODIFER		0.79
#define MG3_RELOAD_TIME		4.8
#define NEGEV_INSPECT_TIME		6.9s

class CNegev : public CBasePlayerWeapon
{
	enum negev_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return MG3_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return NEGEV_INSPECT_TIME; }
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		const char *GetCSModelName() override { return "models/w_negev.mdl"; }

public:
	void NegevFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	time_point_t m_NextInspect;
	int iShellOn;

private:
	unsigned short m_usFireNegev;
};

}
