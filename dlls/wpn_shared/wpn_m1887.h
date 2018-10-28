#ifndef WPN_M1887_H
#define WPN_M1887_H
#ifdef _WIN32
#pragma once
#endif

//m1887
#define M1887_MAX_SPEED		240
#define M1887_CONE_VECTOR		Vector(0.0675, 0.0675, 0.0)	// special shotgun spreads

class CM1887 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return M1887_MAX_SPEED; }
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
	virtual KnockbackData GetKnockBackData() override { return { 1000.0f, 250.0f, 600.0f, 700.0f, 0.9f }; }
	float GetArmorRatioModifier() override { return 1.0; }
	const char *GetCSModelName() override { return "models/w_m1887.mdl"; }

public:
	int m_iShell;
	float m_flPumpTime;

private:
	unsigned short m_usFireM1887;
};

#endif