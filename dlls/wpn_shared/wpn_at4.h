#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum at4_e
{
	ANIM_IDLE1,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_RELOAD,
	ANIM_DRAW,
};

class CAT4 : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed(void) override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack();
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	KnockbackData GetKnockBackData() override { return { 3000.0f, 3000.0f, 3000.0f, 500.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_at4.mdl"; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_84mmrocket" , 200 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 7, 3, 0 }; }

public:
	void AT4Fire(bool autoaim);

	int m_usFireAT4;
};


class CAT4EX : public CAT4
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	void PrimaryAttack() override;

	const char* GetCSModelName() override { return "models/w_at4ex.mdl"; }
};

class CBazooka : public CAT4
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	void PrimaryAttack() override;
	void Reload() override;

	const char* GetCSModelName() override { return "models/w_bazooka.mdl"; }


#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_84mmrocket2" , 200 }; }
#endif
};

}
