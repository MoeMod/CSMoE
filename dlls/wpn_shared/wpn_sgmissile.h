#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSGMissile : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		SGMISSILE_IDLE,
		SGMISSILE_RELOAD,
		SGMISSILE_DRAW,
		SGMISSILE_SHOOT,
		SGMISSILE_IDLE_B,
		SGMISSILE_RELOAD_B,
		SGMISSILE_DRAW_B,
		SGMISSILE_SHOOT_B,
		SGMISSILE_SHOOT2_B,
		SGMISSILE_SHOOT_MISSILE,
		SGMISSILE_SHOOT_MISSILE_LAST,
		SGMISSILE_MISSILE_ON
    };

	enum sgmissile_bodygroup_e
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
	void SGMissileBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList);
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_sgdrill_buckshot" , 150 }; }
#endif

	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 750.0f, 550.0f, 750.0f, 550.0f, 0.45f }; }
	const char* GetCSModelName() override { return "models/w_sgmissile.mdl"; }

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
	unsigned short m_usFireSGMissile;
	time_point_t m_flTimeNextCharge;
};

}
