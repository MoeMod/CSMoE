#ifndef WPN_M249_H
#define WPN_M249_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m249
#define M249_MAX_SPEED            220
#define M249_RANGE_MODIFER        0.97
#define M249_RELOAD_TIME        4.7s
#define M249_INSPECT_TIME		6.9s

class CM249 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M249_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return M249_INSPECT_TIME; }
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return {350.0f, 250.0f, 300.0f, 100.0f, 0.6f}; }

public:
	void M249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	time_point_t m_NextInspect;
	int iShellOn;

private:
	unsigned short m_usFireM249;
};

}

#endif