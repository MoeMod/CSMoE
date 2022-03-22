#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m3dragon
#define m3dragon_MAX_SPEED		230
#define m3dragon_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads
#define m3dragon_INSPECT_TIME		4.50s

class CM3Dragon : public CBasePlayerWeapon
{
	enum m3dragon_e
    {
        M3DRAGON_IDLE,
        M3DRAGON_FIRE1,
        M3DRAGON_FIRE2,
		M3DRAGON_INSERT,
		M3DRAGON_AFTER_RELOAD,
		M3DRAGON_BEFORE_RELOAD,
        M3DRAGON_DRAW,
        M3DRAGON_SECONDARY_READY,
		M3DRAGON_SECONDARY_SHOOT,
		M3DRAGON_SECONDARY_INSERT,
		M3DRAGON_SECONDARY_AFTER_RELOAD,
		M3DRAGON_SECONDARY_BEFORE_RELOAD,
		M3DRAGON_SECONDARY_DRAW,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m3dragon_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void Inspect() override;
	duration_t GetInspectTime() override { return m3dragon_INSPECT_TIME; }
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
	duration_t m_flPumpTime;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFirem3dragon;
};

}
