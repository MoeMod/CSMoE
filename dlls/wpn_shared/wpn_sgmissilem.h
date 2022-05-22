#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CSGMissilem : public CBasePlayerWeapon
{
	enum xm1014_e
    {
		SGMISSILEM_IDLE,
		SGMISSILEM_RELOAD,
		SGMISSILEM_DRAW,
		SGMISSILEM_SHOOT,
		SGMISSILEM_IDLE_B,
		SGMISSILEM_RELOAD_B,
		SGMISSILEM_DRAW_B,
		SGMISSILEM_SHOOT_B,
		SGMISSILEM_SHOOT2_B,
		SGMISSILEM_SHOOT_MISSILEM,
		SGMISSILEM_SHOOT_MISSILEM_LAST,
		SGMISSILEM_MISSILEM_ON
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
	void SGMissilemBcsFire(float fYaw, CUtlVector<CBaseEntity*>* pList, CUtlVector<CBaseEntity*>* pGroupList);
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
	const char* GetCSModelName() override { return "models/w_sgmissilem.mdl"; }

public:
	float GetDamage() const;
	float GetDamageBcs() const;

	void InitChargeState(void);
	void ResetChargeTimer(void);
	void SendChargeState(void);

	float GetDirectDamageForce(void);
	float GetExplodeDamage(void);
	float GetExplodeRadius(void);


private:
	int m_iDefaultAmmo2;
	int m_iModelIndex;
	unsigned short m_usFireSGMissilem;
	time_point_t m_flTimeNextCharge;
};

}
