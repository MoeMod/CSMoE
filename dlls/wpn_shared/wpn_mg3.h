#ifndef WPN_MG3_H
#define WPN_MG3_H
#ifdef _WIN32
#pragma once
#endif

//mg3
#define MG3_MAX_SPEED			235
#define MG3_DAMAGE			35
#define MG3_RANGE_MODIFER		0.79
#define MG3_RELOAD_TIME		4.8

class CMG3 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return MG3_MAX_SPEED; }
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
	virtual KnockbackData GetKnockBackData() override { return { 350.0f, 250.0f, 300.0f, 100.0f, 0.6f }; }
		const char *GetCSModelName() override { return "models/w_mg3.mdl"; }

public:
	void MG3Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireMG3;
};

#endif