#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum dartpistol_e
{
	ANIM_IDLE1,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_RELOAD,
	ANIM_DRAW,
	ANIM_SHOOT_EMPTY,
};

class CDartPistol : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return false; }
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
	KnockbackData GetKnockBackData() override { return { 3000.0f, 3000.0f, 3000.0f, 500.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_dartpistol.mdl"; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_dart" , 300 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 7, 3, 0 }; }

public:
	void DartPistolFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);

	int m_usFireDartPistol;
};

//
//class CM79G : public CM79
//{
//public:
//	void Spawn() override;
//	void Precache() override;
//	int GetItemInfo(ItemInfo* p) override;
//	BOOL Deploy() override;
//
//#ifndef CLIENT_DLL
//	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_40mmgrenade2" , 300 }; }
//#endif
//
//	const char* GetCSModelName() override { return "models/w_m79g.mdl"; }
//
//public:
//	void M79Fire();
//
//};


}
