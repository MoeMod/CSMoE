#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum buffm249_e
{
	BUFFM249_IDLE1,
	BUFFM249_SHOOT1,
	BUFFM249_SHOOT2,
	BUFFM249_RELOAD,
	BUFFM249_DRAW,
	BUFFM249_SHOOT3,
};

class CBUFFM249 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;
	void Reload() override;
	void ItemPostFrame() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override;
	float GetArmorRatioModifier() override { return 1.5; }
	const char *GetCSModelName() override { return "models/w_buffm249.mdl"; }
#ifndef CLIENT_DLL
		WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50az" , 600 }; }
#endif
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED, 6, 3, 140 }; }
public:
	void BUFFM249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamageA() const;
	float GetDamageB() const;
	int m_iShell;
	time_point_t m_tNextPlayEvent;	//phs[4]
	

private:
	unsigned short m_usFireBUFFM249;	//phs[2]
};

class CY23S3BUFFM249 : public CBUFFM249
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	void Holster(int skiplocal) override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;

	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void ItemPostFrame() override;
	void WeaponIdle() override;
	KnockbackData GetKnockBackData() override;
	const char* GetCSModelName() override { return "models/w_y23s3buffm249.mdl"; }

public:
	void Y23S3BUFFM249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetTouchDamage() const;
	float GetRadiusDamage() const;
	float GetDamage() const;
	duration_t GetChargeTime();
private:
	time_point_t m_flTimeNextCharge;
	unsigned short m_usFireY23S3BUFFM249;
};

}
