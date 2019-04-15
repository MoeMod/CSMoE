/*
FirePistol.hpp - part of CSMoE template weapon framework, to auto-gen Fire() function for pistols
Copyright (C) 2019 Moemod Yanase

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

template<class CFinal, class CBase = CBaseTemplateWeapon>
class TFirePistol : public CBase
{
public:
	// Default Settings
//	static constexpr auto &&AccuracyCalc = (x * x * x / 200.0) + 0.35;
//	static constexpr float AccuracyMin = 0.6f;
//	static constexpr float AccuracyMax = 0.92f;
//	static constexpr float RangeModifier = 0.79;
//	static constexpr auto BulletType = BULLET_PLAYER_45ACP;
//	static constexpr int Penetration = 1;
	static constexpr int Distance = 4096;
	static constexpr const auto &T = WeaponTemplate::Varibles::T;
	static constexpr const auto &A = WeaponTemplate::Varibles::A;

public:
	void Fire(float flSpread, float flCycleTime, BOOL fUseAutoAim)
	{
		CFinal &wpn = static_cast<CFinal &>(*this);

		flCycleTime -= 0.075;
		CBase::m_iShotsFired++;

		if (CBase::m_iShotsFired > 1)
			return;

		if (CBase::m_flLastFire)
		{
			CBase::m_flAccuracy -= wpn.AccuracyCalc(T = (gpGlobals->time - CBase::m_flLastFire));
			CheckAccuracyBoundaryMin(&wpn);
			CheckAccuracyBoundaryMax(&wpn);
		}

		CBase::m_flLastFire = gpGlobals->time;

		if (CBase::m_iClip <= 0)
		{
			if (CBase::m_fFireOnEmpty)
			{
				CBase::PlayEmptySound();
				CBase::m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
			}

			return;
		}

		CBase::m_iClip--;
		CBase::m_pPlayer->pev->effects |= EF_MUZZLEFLASH;
#ifndef CLIENT_DLL
		CBase::m_pPlayer->SetAnimation(PLAYER_ATTACK1);
#endif

		UTIL_MakeVectors(CBase::m_pPlayer->pev->v_angle + CBase::m_pPlayer->pev->punchangle);

		CBase::m_pPlayer->m_iWeaponVolume = BIG_EXPLOSION_VOLUME;
		CBase::m_pPlayer->m_iWeaponFlash = DIM_GUN_FLASH;

		const float flDistance = wpn.Distance;
		const int iPenetration = wpn.Penetration;
		const Bullet iBulletType = wpn.BulletType;
		const int iDamage = wpn.GetDamage();
		const float flRangeModifier = wpn.RangeModifier;
		const BOOL bPistol = wpn.ItemSlot == PISTOL_SLOT;
		Vector vecSrc = CBase::m_pPlayer->GetGunPosition();
		Vector vecDir = CBase::m_pPlayer->FireBullets3(vecSrc, gpGlobals->v_forward, flSpread, flDistance, iPenetration, iBulletType, iDamage, flRangeModifier, CBase::m_pPlayer->pev, bPistol, CBase::m_pPlayer->random_seed);

		wpn.FireEvent(vecDir);

		CBase::m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
		CBase::m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
		CBase::m_flNextPrimaryAttack = CBase::m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + flCycleTime;

#ifndef CLIENT_DLL
		if (!CBase::m_iClip && CBase::m_pPlayer->m_rgAmmo[CBase::m_iPrimaryAmmoType] <= 0)
			CBase::m_pPlayer->SetSuitUpdate("!HEV_AMO0", FALSE, 0);
#endif
		CBase::m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + flCycleTime + 1.9f;

		wpn.Recoil();
	}

public:
	void FireEvent(const Vector &vecDir)
	{
		int flags = 0;
		/*
#ifdef CLIENT_WEAPONS
		flags = FEV_NOTHOST;
#else
		flags = 0;
#endif
		 */

		CFinal &wpn = static_cast<CFinal &>(*this);
		PLAYBACK_EVENT_FULL(flags, CBase::m_pPlayer->edict(), wpn.m_usFire, 0, (float *)&g_vecZero, (float *)&g_vecZero, vecDir.x, vecDir.y, static_cast<int>(CBase::m_pPlayer->pev->punchangle.x * 100), static_cast<int>(CBase::m_pPlayer->pev->punchangle.y * 100), CBase::m_iClip != 0, FALSE);
	}

private:
	template<class ClassToCheck = CFinal>
	auto CheckAccuracyBoundaryMin(ClassToCheck *p) -> decltype(&ClassToCheck::AccuracyMin, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_flAccuracy < wpn.AccuracyMin)
			CBase::m_flAccuracy = wpn.AccuracyMin;
	}
	void CheckAccuracyBoundaryMin(...) {}

	template<class ClassToCheck = CFinal>
	auto CheckAccuracyBoundaryMax(ClassToCheck *p) -> decltype(&ClassToCheck::AccuracyMax, void())
	{
		CFinal &wpn = static_cast<CFinal &>(*this);
		if (CBase::m_flAccuracy > wpn.AccuracyMax)
			CBase::m_flAccuracy = wpn.AccuracyMax;
	}
	void CheckAccuracyBoundaryMax(...) {}
};
