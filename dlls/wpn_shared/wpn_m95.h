#ifndef WPN_M95_H
#define WPN_M95_H
#ifdef _WIN32
#pragma once
#endif

//M95
#define M95_MAX_SPEED		200
#define M95_MAX_SPEED_ZOOM	140
#define M95_DAMAGE		115
#define M95_RANGE_MODIFER	0.985
#define M95_RELOAD_TIME		3.38
#define M95_ARMOR_RATIO_MODIFIER	1.9

class CM95 : public CBasePlayerWeapon
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
	virtual KnockbackData GetKnockBackData() override { return { 5000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }
	float GetArmorRatioModifier() override { return M95_ARMOR_RATIO_MODIFIER; }
	const char *GetCSModelName() override { return "models/w_m95.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() { return { "ammo_50bmg" , AMMO_50BMG_PRICE }; }
#endif

public:
	void M95Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;

private:
	unsigned short m_usFireM95;
};

#endif