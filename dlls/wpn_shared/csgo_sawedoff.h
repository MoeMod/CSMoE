#ifndef WPN_M3_H
#define WPN_M3_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//sawedoff
#define SAWEDOFF_MAX_SPEED		230
#define SAWEDOFF_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads
#define SAWEDOFF_INSPECT_TIME		4.48s
#define SAWEDOFF_MAX_CLIP		7


class CSawedoff : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return SAWEDOFF_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return SAWEDOFF_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 1800.0f, 480.0f, 900.0f, 600.0f, 0.3f }; }
	const char* GetCSModelName() override { return "models/w_sawedoff.mdl"; }
public:
	int m_iShell;
	duration_t m_flPumpTime;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireSawedoff;
};

}

#endif