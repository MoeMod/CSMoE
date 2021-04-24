#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//k1a
#define K1A_MAX_SPEED			221
#define K1A_DAMAGE			30
#define K1A_RANGE_MODIFER		0.98
#define K1A_RELOAD_TIME		3.0

class CK1a : public CBasePlayerWeapon
{
	enum k1a_e
    {
        K1A_IDLE1,
        K1A_RELOAD,
        K1A_DRAW,
        K1A_SHOOT1,
        K1A_SHOOT2,
        K1A_SHOOT3
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return K1A_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override {}
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.6f }; }
	const char *GetCSModelName() override { return "models/w_k1a.mdl"; }

public:
	void K1aFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;
public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireK1a;
};

}
