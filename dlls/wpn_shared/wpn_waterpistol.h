#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//waterpistol
#define WATERPISTOL_MAX_SPEED		250
#define WATERPISTOL_DAMAGE			25
#define WATERPISTOL_RANGE_MODIFER		0.75
#define WATERPISTOL_RELOAD_TIME		2.2s

class CWaterPistol : public CBasePlayerWeapon
{
	enum waterpistol_e
    {
       ANIM_IDLE1,
	   ANIM_SHOOT1,
	   ANIM_SHOOT2,
	   ANIM_SHOOT_EMPTY,
	   ANIM_RELOAD1,
	   ANIM_DRAW,
    };

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;

	void Reload() override;
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

	const char* GetCSModelName() override { return "models/w_waterpistol.mdl"; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_waterpistol" , 20 }; }
#endif
	float GetArmorRatioModifier() override { return 1.8; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 7, 3, 170 }; }

public:
	void WaterPistolFire(float flSpread, duration_t flCycleTime);
	void FireRemaining(int& shotsFired, time_point_t& shootTime, BOOL isGlock18) override;

public:
	int m_usFireWaterPistol;
	int m_iShell;
	bool m_bBurstFire;

private:
	float GetDamageA();
	float GetDamageB();
};

}
