#ifndef WPN_AS50_H
#define WPN_AS50_H
#ifdef _WIN32
#pragma once
#endif

//as50
#define AS50_MAX_SPEED		205
#define AS50_MAX_SPEED_ZOOM	160
#define AS50_DAMAGE		115
#define AS50_RANGE_MODIFER	0.98
#define AS50_RELOAD_TIME	3.0
#define AS50_ARMOR_RATIO_MODIFIER	1.7
#define AS50_ZOOM_FOV_1	38
#define AS50_ZOOM_FOV_2	12

class CAS50 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed();
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
	virtual void SecondaryAttack();
	virtual void Reload();
	virtual void WeaponIdle();
	virtual BOOL UseDecrement()
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
	virtual KnockbackData GetKnockBackData() override { return { 2500, 600.0f, 800.0f, 800.0f, 0.4f }; }
	float GetArmorRatioModifier() override { return AS50_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_as50.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

public:
	void AS50Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireAS50;
};

#endif