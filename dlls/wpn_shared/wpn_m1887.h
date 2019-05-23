#ifndef WPN_M1887_H
#define WPN_M1887_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m1887
#define M1887_MAX_SPEED		240
#define M1887_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads

class CM1887 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M1887_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 1000.0f, 250.0f, 600.0f, 700.0f, 0.9f }; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_m1887.mdl"; }

public:
	int m_iShell;
	float m_flPumpTime;

private:
	unsigned short m_usFireM1887;
};

}

#endif