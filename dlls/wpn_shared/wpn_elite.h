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
#define ELITE_MAX_SPEED		250
#define ELITE_RELOAD_TIME	4.5s
#define ELITE_DAMAGE		36
#define ELITE_RANGE_MODIFER	0.75
#define ELITE_INSPECT_TIME		4.48s

class CELITE : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return ELITE_MAX_SPEED; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return ELITE_INSPECT_TIME; }
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

public:
	void ELITEFire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);
	float GetDamage() const;
	time_point_t m_NextInspect;
public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

}

#endif