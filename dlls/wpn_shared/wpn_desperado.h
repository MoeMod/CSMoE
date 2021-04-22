#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//tmp
#define DESPERADO_MAX_SPEED		260
#define DESPERADO_MAX_CLIP	7

class CDesperado : public CBasePlayerWeapon
{
	enum desperado_e;
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return DESPERADO_MAX_SPEED; }
	int GetDamage() const;
	int iItemSlot() override { return PISTOL_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	bool HasSecondaryAttack() override { return true; }
	void ResetRunIdle();
	const char *GetCSModelName() override { return "models/w_desperado.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_44magnum" , 100 }; }
#endif
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return TRUE; }
	KnockbackData GetKnockBackData() override
	{
		if (pev->iuser1)
			return { 1250, 1150, 1200, 1100, 0.8f };
		else
			return { 450, 350, 400, 300, 0.3f };		
	}
	float GetArmorRatioModifier() override { return 1.2; }

public:
	void DesperadoFire(float flSpread, BOOL fUseSemi, int iMode);
	time_point_t m_flNextRunIdle;
	time_point_t m_flNextRunEnd;
	int m_iShell;

private:
	unsigned short m_usFireDesperado;
};

}
