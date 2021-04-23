#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//xm1014
#define XM1014_MAX_SPEED	225
#define XM1014_CONE_VECTOR	Vector(0.0725, 0.0725, 0.0)	// special shotgun spreads
#define	MAG7_DEFAULT_GIVE 5
#define MAG7_INSPECT_TIME		5.05s

class CMag7 : public CBasePlayerWeapon
{
    enum mag7_e
    {
        MAG7_IDLE,
        MAG7_FIRE1,
        MAG7_FIRE2,
        MAG7_RELOAD,
        MAG7_DRAW,
        MAG7_INSPECT,
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return XM1014_MAX_SPEED; }
	void ItemPostFrame() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	int GetPrimaryAttackDamage() const;
	duration_t GetInspectTime() override { return MAG7_INSPECT_TIME; }
	void Inspect() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 750.0f, 550.0f, 650.0f, 500.0f, 0.4f }; }
	const char *GetCSModelName() override { return "models/w_mag7.mdl"; }

public:
	int m_iShell;
	time_point_t m_NextInspect;

private:
	unsigned short m_usFireMag7;
};

}
