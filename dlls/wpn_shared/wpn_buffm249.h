#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CBUFFM249 : public CBasePlayerWeapon
{
	enum buffm249_e
    {
        BUFFM249_IDLE1,
		BUFFM249_SHOOT1,
		BUFFM249_SHOOT2,
		BUFFM249_RELOAD,
		BUFFM249_DRAW
    };
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

public:
	void BUFFM249Fire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamageA() const;
	float GetDamageB() const;
	int m_iShell;
	time_point_t m_tNextPlayEvent;	//phs[4]
	

private:
	unsigned short m_usFireBUFFM249;	//phs[2]
};

}
