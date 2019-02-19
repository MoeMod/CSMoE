#ifndef WPN_M3_H
#define WPN_M3_H
#ifdef _WIN32
#pragma once
#endif

//m3
#define M3_MAX_SPEED		230
#define M3_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads

class CM3 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M3_MAX_SPEED; }
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
	KnockbackData GetKnockBackData() override { return { 1800.0f, 480.0f, 900.0f, 600.0f, 0.3f }; }

public:
	int m_iShell;
	float m_flPumpTime;

private:
	unsigned short m_usFireM3;
};

#endif