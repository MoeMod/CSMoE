#ifndef WPN_AK47L_H
#define WPN_AK47L_H
#ifdef _WIN32
#pragma once
#endif

#include "wpn_ak47.h"

class CAK47_Long : public CBasePlayerWeapon
{
public:
	virtual void Spawn();
	virtual void Precache();
	virtual int GetItemInfo(ItemInfo *p);
	virtual BOOL Deploy();
	virtual float GetMaxSpeed() { return AK47_MAX_SPEED; }
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

	virtual KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	virtual bool HasSecondaryAttack() override { return true; }
	virtual const char *GetCSModelName() override;

public:
	void AK47Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireAK47;
};

#endif