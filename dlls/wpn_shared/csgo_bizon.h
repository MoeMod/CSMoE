#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//bizon
#define BIZON_MAX_SPEED		245
#define BIZON_DAMAGE		21
#define BIZON_RANGE_MODIFER	0.885
#define BIZON_RELOAD_TIME		2.56s
#define BIZON_INSPECT_TIME		4.5s
#define BIZON_MAX_CLIP 64

class CBizon : public CBasePlayerWeapon
{
	enum bizon_e
    {
        BIZON_IDLE1,
        BIZON_RELOAD,
        BIZON_DRAW,
        BIZON_SHOOT1,
        BIZON_SHOOT2,
        BIZON_SHOOT3,
        BIZON_INSPECT
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return BIZON_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	void Inspect() override;
	float GetDamage();
	duration_t GetInspectTime() override { return BIZON_INSPECT_TIME; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }
	const char* GetCSModelName() override { return "models/w_bizon.mdl"; }
public:
	void BizonFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireBizon;
};

}
