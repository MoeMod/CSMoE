#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//m3dragon
#define M3DRAGON_MAX_SPEED		240
#define M3DRAGON_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads
#define M3DRAGON_MAX_CLIP	8

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
	float GetMaxSpeed() override { return M3DRAGON_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	bool HasSecondaryAttack() override { return true; }
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	void Reload() override;
	void WeaponIdle() override;
	void Holster(int skiplocal) override;
	const char* GetCSModelName() override { return "models/w_m3dragon.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	void StopIdleSound();
	int GetDamage();
	KnockbackData GetKnockBackData() override { return { 3000.0f, 480.0f, 900.0f, 1600.0f, 0.3f }; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return ::sv::GetBuyAmmoConfig(WEAPON_M3); }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 8, 6, 0 }; }

public:
	int m_iShell;
	duration_t m_flPumpTime;

private:
	time_point_t m_flNextPlaySound;
	unsigned short m_usFireM3Dragon;
};

}
