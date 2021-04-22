#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m3
#define M3_MAX_SPEED		230
#define M3_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads
#define M3_INSPECT_TIME		4.50s

class CM3 : public CBasePlayerWeapon
{
	enum m3_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M3_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return M3_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 1800.0f, 480.0f, 900.0f, 600.0f, 0.3f }; }

public:
	int m_iShell;
	duration_t m_flPumpTime;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireM3;
};

}
