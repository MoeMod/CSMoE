#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

enum m79_e
{
	ANIM_IDLE1,
	ANIM_SHOOT1,
	ANIM_SHOOT2,
	ANIM_DRAW,
};

class CM79 : public CBasePlayerWeapon
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
	KnockbackData GetKnockBackData() override { return { 3000.0f, 3000.0f, 3000.0f, 500.0f, 1.0f }; }

	const char* GetCSModelName() override { return "models/w_m79.mdl"; }

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_40mmgrenade" , 300 }; }
#endif
	float GetArmorRatioModifier() override { return 1.0; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 7, 3, 0 }; }

public:
	void M79Fire();

	int m_usFireM79;
};


class CM79G : public CM79
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;

#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_40mmgrenade2" , 300 }; }
#endif

	const char* GetCSModelName() override { return "models/w_m79g.mdl"; }

public:
	void M79Fire();

};

class CFireCracker : public CM79
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;

	const char* GetCSModelName() override { return "models/w_firecracker.mdl"; }

public:
	void FireCrackerFire(bool bBounce);

};

class CJANUS1 : public CM79
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo* p) override;
	BOOL Deploy() override;
	void PrimaryAttack() override;
	bool HasSecondaryAttack() override { return true; }
	void SecondaryAttack() override;
	void WeaponIdle() override;
	void ItemPostFrame() override;
	const char* GetCSModelName() override { return "models/w_janus1.mdl"; }

	WeaponCrosshairData GetCrosshairData() override
	{
		if (m_iWeaponState)
			return { CROSSHAIR_COLOR_CUSTOM | ACCURACY_AIR, 6, 3, 0 };
		else
			return { ACCURACY_AIR, 6, 3, 0 };
	}
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;

public:
	void Janus1Fire(bool bmode);

	time_point_t m_flNextReset;
	int m_iShotsRequired;

};

class CJanus1Rocket : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);

public:
	void EXPORT FollowThink(void);
	void EXPORT IgniteThink(void);
	void EXPORT RocketTouch(CBaseEntity* pOther);

public:
	int GetWeaponsId(void)
	{
		return WEAPON_JANUS11;
	}

	int m_iType;

	CJANUS1* m_pWeapon;

private:
	float GetRadius();
	float GetDamage();


	int m_iTrail;
};


}
