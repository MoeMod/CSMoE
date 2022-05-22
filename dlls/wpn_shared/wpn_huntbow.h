#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

#define HUNTBOW_MAX_SPEED            240

class CHuntbow : public CBasePlayerWeapon
{
	enum huntbow_e
    {
		BOW_IDLE,
		BOW_IDLE_EMPTY,
		BOW_SHOOT1,
		BOW_SHOOT1_EMPTY,
		BOW_DRAW,
		BOW_DRAW_EMPTY,
		BOW_CHARGE_START1,
		BOW_CHARGE_FINISH1,
		BOW_CHARGE_IDLE1,
		BOW_CHARGE_IDLE2,
		BOW_CHARGE_SHOOT1,
		BOW_CHARGE_SHOOT1_EMPTY,
		BOW_CHARGE_SHOOT2,
		BOW_CHARGE_SHOOT2_EMPTY
    };

	enum huntbow_crosshair
	{
		AIM_NO = 3,
		HUNTBOW_AIM_IDLE,
		HUNTBOW_AIM_HIT,
		HUNTBOW_HIT_LEFT,
		HUNTBOW_HIT_RIGHT,
		HUNTBOW_AIM_CHARGIN,
		HUNTBOW_AIM_HIT_LEFT,
		HUNTBOW_AIM_HIT_RIGHT
	};
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	bool HasSecondaryAttack() override { return true; }
	float GetMaxSpeed() override { return HUNTBOW_MAX_SPEED; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Holster(int skiplocal) override;
	void ItemPostFrame() override;
#ifndef CLIENT_DLL
	Vector Get_ShootPosition(CBaseEntity* pevAttacker, Vector Start, int* bHit);
	
	void RadiusDamage(Vector vecAiming, float flDamage, float flRadius);
	void ScannerFunc(Vector vecSrc, Vector vecDir, float flRadius, float flAngleDegrees, entvars_t* pevPlayer, int m_iChargeState);
	Vector CalcPositon(Vector vecSrc, Vector vecDir, int &bHit, int &iTarget);
#endif
	float GetPrimaryAttackDamage() const;
	float GetSecondaryAttackDamage() const;
	void WeaponIdle() override;
	BOOL UseDecrement() override
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override 
	{ 
		if (m_iMode)
			return { 0, 0, 0, 0, 0.2f };
		else
			return { 1500, 900, 1000, 900, 0.25f };
	}
	const char *GetCSModelName() override { return "models/w_huntbow.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_bow" , 200 }; }
#endif
	float GetArmorRatioModifier() override { return 1.95; }
	WeaponCrosshairData GetCrosshairData() override { return { ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK, 4, 3, 0 }; }

	int ExtractAmmo(CBasePlayerWeapon *pWeapon) override;

	void HuntbowFire(float flSpread, duration_t flCycleTime, BOOL fChargeInterrupted);
	void ChargedFire(void);
	
	void OnAttack2Released(void);
	void SetWeaponModel(void);
	void SetInChargeState(bool state);
	float GetSpread(void);

private:
	int m_iMode;
	int m_iChargeState;
	int m_iChargeSound;
	int m_CrossHairStateStored;
	int m_CrossHairState;
	time_point_t m_flTimeCheckCharge;
	time_point_t m_flNextTimeCheck;
	bool m_fInCharge;
	unsigned short m_usFireHuntbow;
};

}
