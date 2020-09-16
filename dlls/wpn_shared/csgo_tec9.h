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
#define TEC9_MAX_SPEED		250
#define TEC9_RELOAD_TIME	2.5s
#define TEC9_RANGE_MODIFER	0.75
#define TEC9_MAX_CLIP	18
#define TEC9_INSPECT_TIME		3.9s

class CTec9 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return TEC9_MAX_SPEED; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return TEC9_INSPECT_TIME; }
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
	const char* GetCSModelName() override { return "models/w_tec9.mdl"; }
public:
	void Tec9Fire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	float GetDamage();
	time_point_t m_NextInspect;
public:
	int m_iShell;
	unsigned short m_usFireTec9;
};

}

#endif