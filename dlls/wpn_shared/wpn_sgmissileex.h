#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSGMissileEX : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		SGMISSILEEX_IDLE,
		SGMISSILEEX_RELOAD,
		SGMISSILEEX_DRAW,
		SGMISSILEEX_SHOOT,
		SGMISSILEEX_IDLE_B,
		SGMISSILEEX_RELOAD_B,
		SGMISSILEEX_DRAW_B,
		SGMISSILEEX_SHOOT_B,
		SGMISSILEEX_SHOOT2_B,
		SGMISSILEEX_SHOOT_MISSILEEX,
		SGMISSILEEX_SHOOT_MISSILEEX_LAST,
		SGMISSILEEX_MISSILEEX_ON
    };

	enum sgmissileex_bodygroup_e
	{	
		GROUP_HANDS,
		GROUP_WEAPON,
		GROUP_SKINGROUP,
		GROUP_SKINGROUP2,
		GROUP_LED,
		GROUP_FX1,
		GROUP_FX2,
		GROUP_FX3,
		GROUP_FX4,
		GROUP_FX5,	
		GROUP_FX6
	};

public:
#ifndef CLIENT_DLL
	void ItemPreFrame(void);
#endif
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 240; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	bool HasSecondaryAttack(void) override { return true; }
	void Reload() override;
	void Holster(int skiplocal) override;
	void WeaponIdle() override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
#ifndef CLIENT_DLL
	void SGMissileEXBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList);
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_sgdrill_buckshot" , 150 }; }
#endif
	float GetArmorRatioModifier() override { return 1.3; }
	WeaponCrosshairData GetCrosshairData() override { return { 0, 9, 4, 0 }; }

	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 750.0f, 550.0f, 750.0f, 550.0f, 0.45f }; }
	const char* GetCSModelName() override { return "models/w_sgmissileex.mdl"; }

public:
	float GetDamage() const;
	float GetDamageBcs() const;

	void InitChargeState(void);
	void ResetChargeTimer(void);
	void SetChargeSkin(void);
	void SendChargeState(void);

	void SetToAMode(void);
	void SetToBMode(void);

	float GetDirectDamageForce(void);
	float GetExplodeDamage(void);
	float GetExplodeRadius(void);


private:
	int m_iDefaultAmmo2;
	int m_iModelIndex;
	unsigned short m_usFireSGMissileEX;
	time_point_t m_flTimeNextCharge;
};

}
