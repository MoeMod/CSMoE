#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//AWP
#define AWP_MAX_SPEED		210
#define AWP_MAX_SPEED_ZOOM	150
#define AWP_DAMAGE		115
#define AWP_RANGE_MODIFER	0.99
#define AWP_RELOAD_TIME		2.5s
#define AWP_INSPECT_TIME		5.04s

class CAWP : public CBasePlayerWeapon
{
	enum awp_e
    {
        AWP_IDLE,
        AWP_SHOOT1,
        AWP_SHOOT2,
        AWP_SHOOT3,
        AWP_RELOAD,
        AWP_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	duration_t GetInspectTime() override { return AWP_INSPECT_TIME; }
	void Inspect() override;
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }

public:
	void AWPFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
	int m_iShell;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireAWP;
};

}
