/*
wpn_svdex.h - CSMoE Gameplay server : Zombie Hero
Copyright (C) 2019 TmNine!~

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef WPN_SVDEX_H
#define WPN_SVDEX_H
#ifdef _WIN32
#pragma once
#endif

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

//SB
#define SVDEX_MAX_SPEED		270
#define SVDEX_DAMAGE		32
#define SVDEX_RANGE_MODIFER	0.99
#define SVDEX_RELOAD_TIME	3.8

class CSVDEX : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	float GetMaxSpeed() override { return SVDEX_MAX_SPEED; }
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
	KnockbackData GetKnockBackData() override { return { 1500.0f, 1000.0f, 1500.0f, 860.0f, 0.8f }; }
	const char *GetCSModelName() override { return "models/w_svdex.mdl"; }

public:
	void SVDEXFire1(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void SVDEXFire2(duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;

private:
	unsigned short m_usFireSVDEX;
};

}

#endif