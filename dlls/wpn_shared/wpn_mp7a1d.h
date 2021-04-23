#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CMP7A1D : public CBasePlayerWeapon
{
	enum elite_e
    {
        ELITE_IDLE,
        ELITE_IDLE_LEFTEMPTY,
        ELITE_SHOOTLEFT1,
        ELITE_SHOOTLEFT2,
        ELITE_SHOOTLEFT3,
        ELITE_SHOOTLEFT4,
        ELITE_SHOOTLEFT5,
        ELITE_SHOOTLEFTLAST,
        ELITE_SHOOTRIGHT1,
        ELITE_SHOOTRIGHT2,
        ELITE_SHOOTRIGHT3,
        ELITE_SHOOTRIGHT4,
        ELITE_SHOOTRIGHT5,
        ELITE_SHOOTRIGHTLAST,
        ELITE_RELOAD,
        ELITE_DRAW
    };
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return 250; }
	int iItemSlot() override { return PRIMARY_WEAPON_SLOT; }
	void PrimaryAttack() override;
	void Reload() override;
	void WeaponIdle() override;
	BOOL UseDecrement() override {
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	BOOL IsPistol() override { return FALSE; }

	KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_dmp7a1.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() override { return { "ammo_46mm" , AMMO_46MM_PRICE}; }
#endif

public:
	void ELITEFire(float flSpread, duration_t flCycleTime, BOOL fUseSemi);

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

}
