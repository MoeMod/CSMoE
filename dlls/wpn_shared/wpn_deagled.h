#ifndef WPN_DEAGLED_H
#define WPN_DEAGLED_H
#ifdef _WIN32
#pragma once
#endif

class CDeagleD : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return 250; }
	virtual int iItemSlot() { return PISTOL_SLOT; }
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
	virtual BOOL IsPistol() { return TRUE; }

	KnockbackData GetKnockBackData() override { return { 3000.0f, 500.0f, 1200.0f, 800.0f, 0.3f }; }
	float GetArmorRatioModifier() override { return 1.5; }
	const char *GetCSModelName() override { return "models/w_ddeagle.mdl"; }
#ifndef CLIENT_DLL
	WeaponBuyAmmoConfig GetBuyAmmoConfig() { return ::GetBuyAmmoConfig(WEAPON_DEAGLE); }
#endif

public:
	void ELITEFire(float flSpread, float flCycleTime, BOOL fUseSemi);

public:
	int m_iShell;
	unsigned short m_usFireELITE_LEFT;
	unsigned short m_usFireELITE_RIGHT;
};

#endif