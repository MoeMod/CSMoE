#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m4a1
#define M4A1_MAX_SPEED		230
#define M4A1_DAMAGE		32
#define M4A1_DAMAGE_SIL		33
#define M4A1_RANGE_MODIFER      0.97
#define M4A1_RANGE_MODIFER_SIL  0.95
#define M4A1_RELOAD_TIME	3.05s
#define M4A1_INSPECT_TIME		4.85s

class CM4A1 : public CBasePlayerWeapon
{
	enum m4a1_e
    {
        M4A1_IDLE,
        M4A1_SHOOT1,
        M4A1_SHOOT2,
        M4A1_SHOOT3,
        M4A1_RELOAD,
        M4A1_DRAW,
        M4A1_ATTACH_SILENCER,
        M4A1_UNSIL_IDLE,
        M4A1_UNSIL_SHOOT1,
        M4A1_UNSIL_SHOOT2,
        M4A1_UNSIL_SHOOT3,
        M4A1_UNSIL_RELOAD,
        M4A1_UNSIL_DRAW,
        M4A1_DETACH_SILENCER
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return M4A1_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return M4A1_INSPECT_TIME; }
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }

public:
	void M4A1Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	time_point_t m_NextInspect;
	int iShellOn;

private:
	unsigned short m_usFireM4A1;
};

}
