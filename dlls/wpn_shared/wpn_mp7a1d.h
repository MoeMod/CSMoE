#ifndef WPN_MP7A1D_H
#define WPN_MP7A1D_H
#ifdef _WIN32
#pragma once
#endif

class CMP7A1D : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return 250; }
	virtual int iItemSlot() { return PRIMARY_WEAPON_SLOT; }
	virtual void PrimaryAttack();
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
	virtual BOOL IsPistol() { return FALSE; }

	KnockbackData GetKnockBackData() override { return { 450.0f, 400.0f, 400.0f, 200.0f, 0.5f }; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_dmp7a1.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() { return { "ammo_46mm" , AMMO_46MM_PRICE}; }
#endif

public:
	void ELITEFire(float flSpread, float flCycleTime, BOOL fUseSemi);

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

#endif