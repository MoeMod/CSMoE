#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define GUILLOTINE_MAX_AMMO 10

class CGuillotine : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 235.0f; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void ItemPostFrame() override;
	void Reload() override { ; }
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	const char* GetCSModelName() override { return "models/w_guillotine.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
#ifndef CLIENT_DLL
	void Drop(void);
	void CrashAmmo(void);
#endif
	void CatchAmmo(void);


	int iMaxClip(void);
	float GetDamageAmountA(void);
	float GetDamageAmountB(void);

	time_point_t m_flTimeWeaponIdle2;

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 400.0f, 300.0f, 1.0f }; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_guillotine" , 120 }; }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 4, 3, 0 }; }

#ifndef CLIENT_DLL
private:
	EHANDLE m_hAmmo;
#endif
};

#define GUILLOTINEEX_MAX_AMMO 50
#define GUILLOTINEEX_MAX_PROJECTILE 2
class CGuillotineex : public CGuillotine
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 235.0f; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void ItemPostFrame() override;
	void Reload() override { ; }
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	const char* GetCSModelName() override { return "models/w_guillotineex.mdl"; }
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
#ifndef CLIENT_DLL
	void Drop(void);
	void CrashAmmo(void);
#endif
	void CatchAmmo(void);

	int iMaxClip(void);
	float GetDamageAmountA(void);
	float GetDamageAmountB(void);

	KnockbackData GetKnockBackData() override { return { 450.0f, 350.0f, 400.0f, 300.0f, -1.0f }; }
	int m_iState;
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_guillotineex" , 120 }; }
#endif
	float GetArmorRatioModifier() override { return 1.1; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 4, 3, 0 }; }

	int m_iHitCount;
#ifndef CLIENT_DLL
private:
	EHANDLE m_hAmmo[GUILLOTINEEX_MAX_PROJECTILE];
#endif
};

}
