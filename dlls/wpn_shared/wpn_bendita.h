#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//bendita
#define BENDITA_MAX_SPEED		205
#define BENDITA_MAX_SPEED_ZOOM	160
#define BENDITA_RANGE_MODIFER	0.98
#define BENDITA_RELOAD_TIME	3.0s
#define BENDITA_ARMOR_RATIO_MODIFIER	1.6
#define BENDITA_ZOOM_FOV_1	38
#define BENDITA_ZOOM_FOV_2	12

class CBendita : public CBasePlayerWeapon
{
	enum bendita_e
    {
        BENDITA_IDLE,
        BENDITA_SHOOT1,
        BENDITA_SHOOT2,
        BENDITA_SHOOT3,
        BENDITA_RELOAD,
        BENDITA_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override;
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void SecondaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	KnockbackData GetKnockBackData() override { return { 3000.f, 300.0f, 800.0f, 800.0f, 0.4f }; }
	float GetArmorRatioModifier() override { return 1.8; }
	const char *GetCSModelName() override { return "models/w_bendita.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_50bmgholy" , AMMO_50BMG_PRICE }; }
#endif

public:
	void BenditaFire(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	float GetDamage() const;

private:
	unsigned short m_usFireBendita;
};

}
