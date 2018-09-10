#ifndef WPN_UMP45_H
#define WPN_UMP45_H
#ifdef _WIN32
#pragma once
#endif

//ump45
#define UMP45_MAX_SPEED			250
#define UMP45_DAMAGE			30
#define UMP45_RANGE_MODIFER		0.82
#define UMP45_RELOAD_TIME		3.5

class CUMP45 : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return UMP45_MAX_SPEED; }
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
	virtual KnockbackData GetKnockBackData() override { return { 250.0f, 200.0f, 250.0f, 90.0f, 0.7f }; }

public:
	void UMP45Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

public:
	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireUMP45;
};

#endif