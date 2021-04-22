/*
z4b_m4a1razer.h - Port of Zombie IV Mod
Copyright (C) 2019 Moemod Haoyuan

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#pragma once

#ifdef CLIENT_DLL
namespace cl {
#else
namespace sv {
#endif

class CM4A1Razer : public CBasePlayerWeapon
{
public:
	void Spawn() override;
	void Precache() override;
	int GetItemInfo(ItemInfo *p) override;
	BOOL Deploy() override;
	void Holster(int skiplocal) override;
	int ExtractAmmo(CBasePlayerWeapon* pWeapon) override;
	float GetMaxSpeed() override { return 235; }
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
	KnockbackData GetKnockBackData() override { return { 700.0f, 450.0f, 600.0f, 450.0f, 0.4f }; }
	const char *GetCSModelName() override { return "models/z4b/w_m4a1razer.mdl"; }

public:
	void SVDEXFire1(float flSpread, duration_t flCycleTime, BOOL fUseAutoAim);
	void SVDEXFire2(duration_t flCycleTime, BOOL fUseAutoAim);

	int m_iShell;
	int iShellOn;
	int m_iDefaultAmmo2;

private:
	unsigned short m_usFireSVDEX;
};

}
