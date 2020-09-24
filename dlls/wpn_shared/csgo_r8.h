#ifndef WPN_ELITE_H
#define WPN_ELITE_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//Elites
#define R8_MAX_SPEED		250
#define R8_RELOAD_TIME	2.5s
#define R8_RANGE_MODIFER	0.75
#define R8_MAX_CLIP	8
#define R8_INSPECT_TIME		5.87s

class CR8 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return R8_MAX_SPEED; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void SecondaryAttack() override;
	void Inspect() override;
	bool HasSecondaryAttack() override { return true; }
	duration_t GetInspectTime() override { return R8_INSPECT_TIME; }
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override { return { 85.0f, 100.0f, 100.0f, 80.0f, 0.8f }; }
	const char* GetCSModelName() override { return "models/w_r8.mdl"; }
public:
	void R8FireA();
	void R8FireB();
	time_point_t m_tPreTime;
	void R8Fire(float flSpread, duration_t flCycleTime, BOOL fUseSemi, BOOL fIsModeA);
	float GetDamage(BOOL fIsModeA);
	time_point_t m_NextInspect;
public:
	int m_iShell;
	unsigned short m_usFireR8;
};

}

#endif