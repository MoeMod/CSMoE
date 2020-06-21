#ifndef WPN_AK47_H
#define WPN_AK47_H
#ifdef _WIN32
#pragma once
#endif

//AK47
#define AK47_MAX_SPEED			221
#define AK47_DAMAGE			36
#define AK47_RANGE_MODIFER		0.98
#define AK47_RELOAD_TIME		2.45s
#define AK47_INSPECT_TIME		4.48s

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CAK47 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return AK47_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override {}
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return AK47_INSPECT_TIME;}
	void WeaponIdle()override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 200.0f, 100.0f, 0.6f }; }

public:
	void AK47Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;
	time_point_t m_tLastFire;

private:
	unsigned short m_usFireAK47;
};

}

#endif