#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum tknife_e
{
	ANIM_IDLE1,
	ANIM_SHOOT1,
	ANIM_DRAW,
	ANIM_SHOOT2,
};

class CTKnife : public CBasePlayerWeapon
{
	
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return m_fMaxSpeed; }
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;
	void Holster(int skiplocal) override;
	void Reload() override {};
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_tknife.mdl"; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_tknife" , 30 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 7, 3, 0 }; }

public:

	void ThrowingKnifeFire(float flSpread, duration_t flCycleTime);
	float GetDamage();
};


class CTKnifeEX : public CTKnife
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;

	void PrimaryAttack() override;
	void ItemPostFrame() override;
	void Holster(int skiplocal) override;
	void WeaponIdle() override;

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_tknifeex.mdl"; }
public:
	
	int m_iKnifeHas;
	void ThrowingKnifeFire(float flSpread, duration_t flCycleTime);
#ifndef CLIENT_DLL
	void ThrowingKnife(int iType, float flSpread);
#endif
};

class CTKnifeEX2 : public CTKnifeEX
{

public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;

	void PrimaryAttack() override;
	void ItemPostFrame() override;
	void Holster(int skiplocal) override;
	void WeaponIdle() override;

	KnockbackData GetKnockBackData() override { return { 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_tknifeex2.mdl"; }
public:

	int m_iKnifeHas;
	void ThrowingKnifeFire(float flSpread, duration_t flCycleTime);

};

}
